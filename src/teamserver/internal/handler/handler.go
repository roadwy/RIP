/*
 * Copyright (c) 2021.  https://github.com/geemion
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package handler

import (
	"errors"
	"github.com/golang/protobuf/proto"
	"github.com/panjf2000/gnet"
	"log"
	"math/rand"
	"sync"
	bn "teamserver/internal/beacon"
	"teamserver/internal/conf"
	"teamserver/internal/proto/encode"
	pb "teamserver/internal/proto/protobuf"
	"teamserver/internal/store"
	"teamserver/pkg/crypto"
	"teamserver/pkg/mq"
	"time"
)

type MsgHandler struct {
	Sessions sync.Map
	cmdqueue *mq.Client
}

func NewMsgHandler(mqclient *mq.Client) *MsgHandler {
	handler := &MsgHandler{cmdqueue: mqclient}
	go handler.pushTask()
	return handler
}

func (hm *MsgHandler) HandleMsg(msg []byte, c gnet.Conn, conntype pb.CONN_TYPE) (msgrsp []byte, err error) {

	dp := encode.NewDataPack()
	netio, err := dp.Unpack(msg)
	if err != nil {
		return nil, err
	}

	if !netio.IsEncrypted() {
		return hm.auth(netio, c, conntype)
	} else {
		return hm.msgdispatch(netio, conntype)
	}
}

func (hm *MsgHandler) HandleClose(c gnet.Conn) {
	sessionid := c.Context().(uint64)
	hm.Sessions.Delete(sessionid)
}

func (hm *MsgHandler) auth(netio encode.INetioData, c gnet.Conn, conntype pb.CONN_TYPE) (msgrsp []byte, err error) {
	task := &pb.TaskData{}
	err = proto.Unmarshal(netio.GetData(), task)
	if err != nil {
		return
	}
	switch pb.MSGID(task.MsgId) {
	case pb.MSGID_PUBKEY_REQ:
		msgrsp, err = hm.onReqPubKey(task, c, conntype)
		return
	case pb.MSGID_AUTH_REQ:
		msgrsp, err = hm.onReqAuth(netio.GetSessionId(), task, c, conntype)
		return
	default:
		return msgrsp, errors.New("no msgid")
	}
}

func (hm *MsgHandler) msgdispatch(netio encode.INetioData, conntype pb.CONN_TYPE) (msgrsp []byte, err error) {
	sessionid := netio.GetSessionId()
	beacon, ok := hm.Sessions.Load(sessionid)
	if !ok {
		return msgrsp, errors.New("no session id")
	}
	sessionkey := beacon.(*bn.Beacon).SessionKey
	taskdata, err := crypto.Xchacha20(sessionkey, netio.GetData())
	if err != nil {
		return msgrsp, errors.New("error session key")
	}

	task := &pb.TaskData{}
	err = proto.Unmarshal(taskdata, task)
	if err != nil {
		return
	}
	var taskrspdata []byte
	switch pb.MSGID(task.MsgId) {
	case pb.MSGID_HOST_INFO_RSP:
		taskrspdata, err = hm.OnRspData(task)
		break
	case pb.MSGID_HEAT_BEAT_REQ:
		{
			taskrspdata, err = hm.OnQuerytask(task)

			ipaddr := beacon.(*bn.Beacon).Conn.RemoteAddr()
			hm.onUpdateBeacon(task, ipaddr.String())
			break
		}
	default:
		taskrspdata, err = hm.OnRspData(task)
		break
	}

	taskdataenc, err := crypto.Xchacha20(sessionkey, taskrspdata)
	if err != nil {
		return
	}

	dp := encode.NewDataPack()
	msg := encode.NewNetioData(true, sessionid, taskdataenc)
	msgrsp, _ = dp.Pack(msg, conntype)
	return
}

func (hm *MsgHandler) onReqPubKey(taskreq *pb.TaskData, c gnet.Conn, conntype pb.CONN_TYPE) (rsp []byte, err error) {

	authkey := &pb.AuthRsaKey{Pe: conf.GlobalConf.RsaEncode.E, Pn: conf.GlobalConf.RsaEncode.N}
	authdata, _ := proto.Marshal(authkey)

	taskrsp := &pb.TaskData{MsgId: int32(pb.MSGID_PUBKEY_RSP), BeaconId: taskreq.BeaconId, TaskId: 123445, ByteValue: authdata}
	taskdata, _ := proto.Marshal(taskrsp)

	nano := time.Now().UnixNano()
	rand.Seed(nano)
	sessionid := rand.Uint64()

	c.SetContext(sessionid)

	dp := encode.NewDataPack()
	msg := encode.NewNetioData(false, sessionid, taskdata)
	rsp, _ = dp.Pack(msg, conntype)

	return rsp, nil
}

func (hm *MsgHandler) onReqAuth(sessionid uint64, taskreq *pb.TaskData, c gnet.Conn, conntype pb.CONN_TYPE) (rsp []byte, err error) {
	enc_session_key := taskreq.GetByteValue()
	key, err := conf.GlobalConf.RsaEncode.PrivateDecode(enc_session_key)
	if err != nil {
		return
	}
	beacon := &bn.Beacon{BeaconId: taskreq.BeaconId, SessionKey: key, ConnType: conntype, Conn: c}
	hm.Sessions.Store(sessionid, beacon)

	taskrsp := &pb.TaskData{MsgId: int32(pb.MSGID_AUTH_RSP), BeaconId: taskreq.BeaconId, TaskId: 123445, ByteValue: nil}
	taskdata, _ := proto.Marshal(taskrsp)

	dp := encode.NewDataPack()
	msg := encode.NewNetioData(false, sessionid, taskdata)
	rsp, _ = dp.Pack(msg, conntype)
	return
}

//todo: save rsp to db
func (hm *MsgHandler) OnRspData(taskrsp *pb.TaskData) (rsp []byte, err error) {
	teamclientrsp := pb.CommandRsp{
		TaskId:    taskrsp.TaskId,
		BeaconId:  taskrsp.BeaconId,
		MsgId:     taskrsp.MsgId,
		ByteValue: taskrsp.ByteValue,
	}
	hm.cmdqueue.Publish(conf.CmdRspTopic, teamclientrsp)
	return
}

//todo:tcp push cmd
func (hm *MsgHandler) OnQuerytask(taskrsp *pb.TaskData) (rsp []byte, err error) {
	taskdata, err := store.GetTask(taskrsp.BeaconId)
	if err != nil {
		return
	}
	rsp, err = proto.Marshal(&taskdata)
	return
}

func (hm *MsgHandler) onUpdateBeacon(taskrsp *pb.TaskData, ipaddr string) {
	store.UpdateBeacon(taskrsp.BeaconId, ipaddr, taskrsp.ByteValue)
}

func (hm *MsgHandler) pushTask() {

	taskch, err := hm.cmdqueue.Subscribe(conf.CmdReqTopic)
	if err != nil {
		log.Print(err)
		return
	}

	defer hm.cmdqueue.Unsubscribe(conf.CmdReqTopic, taskch)

	for {
		req, ok := (hm.cmdqueue.GetPayLoad(taskch)).(*pb.CommandReq)
		if !ok {
			continue
		}

		var beacon *bn.Beacon = nil
		var sessionid uint64
		hm.Sessions.Range(func(key, value interface{}) bool {
			temp := value.(*bn.Beacon)
			if temp.BeaconId == req.GetBeaconId() {
				sessionid = key.(uint64)
				beacon = temp
				ok = true
				return true
			}
			return false
		})

		if beacon != nil {
			task, err := store.GetTask(req.BeaconId)
			if err != nil {
				return
			}
			data, err := proto.Marshal(&task)
			taskdataenc, err := crypto.Xchacha20(beacon.SessionKey, data)
			if err != nil {
				return
			}

			dp := encode.NewDataPack()
			msg := encode.NewNetioData(true, sessionid, taskdataenc)
			msgrsp, _ := dp.Pack(msg, beacon.ConnType)

			if beacon.ConnType == pb.CONN_TYPE_CONNNAME_TCP {
				beacon.Conn.AsyncWrite(msgrsp)
			} else {
				beacon.Conn.SendTo(msgrsp)
			}
		}
	}
}
