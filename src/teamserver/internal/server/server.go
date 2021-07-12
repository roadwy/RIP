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

package server

import (
	"encoding/binary"
	"fmt"
	"github.com/panjf2000/gnet"
	"github.com/panjf2000/gnet/pool/goroutine"
	"log"
	"strings"
	"teamserver/internal/handler"
	pb "teamserver/internal/proto/protobuf"
	"time"
)

type beaconServer struct {
	*gnet.EventServer
	addr       string
	multicore  bool
	async      bool
	codec      gnet.ICodec
	workerPool *goroutine.Pool
	handler    *handler.MsgHandler
	conntype   pb.CONN_TYPE
}

func (cs *beaconServer) OnInitComplete(srv gnet.Server) (action gnet.Action) {
	log.Printf("beacon server is listening on %s (multi-cores: %t, loops: %d)\n",
		srv.Addr.String(), srv.Multicore, srv.NumEventLoop)
	return
}

func (cs *beaconServer) React(frame []byte, c gnet.Conn) (out []byte, action gnet.Action) {

	data := append([]byte{}, frame...)
	_ = cs.workerPool.Submit(func() {
		rsp, err := cs.handler.HandleMsg(data, c, cs.conntype)
		if err != nil {
			fmt.Println(err)
			c.Close()
			return
		}
		if cs.conntype == pb.CONN_TYPE_CONNNAME_TCP {
			c.AsyncWrite(rsp)
		} else {
			c.SendTo(rsp)
		}
	})
	return
}

func (cs *beaconServer) OnClosed(c gnet.Conn, err error) (action gnet.Action) {
	cs.handler.HandleClose(c)
	return
}

func (cs *beaconServer) Run() (err error) {
	return gnet.Serve(cs, cs.addr, gnet.WithMulticore(cs.multicore), gnet.WithTCPKeepAlive(time.Minute*5), gnet.WithCodec(cs.codec))
}

func NewBeaconServer(addr string, multicore, async bool, codec gnet.ICodec, msghandler *handler.MsgHandler) (cs *beaconServer) {
	if codec == nil {
		encoderConfig := gnet.EncoderConfig{
			ByteOrder:                       binary.BigEndian,
			LengthFieldLength:               4,
			LengthAdjustment:                -13,
			LengthIncludesLengthFieldLength: false,
		}
		decoderConfig := gnet.DecoderConfig{
			ByteOrder:           binary.BigEndian,
			LengthFieldOffset:   0,
			LengthFieldLength:   4,
			LengthAdjustment:    13,
			InitialBytesToStrip: 0,
		}
		codec = gnet.NewLengthFieldBasedFrameCodec(encoderConfig, decoderConfig)
	}

	conntype := pb.CONN_TYPE_CONNNAME_TCP
	if strings.EqualFold("udp", addr[:3]) {
		conntype = pb.CONN_TYPE_CONNNAME_UDP
	}

	cs = &beaconServer{addr: addr, multicore: multicore, async: async,
		codec: codec, workerPool: goroutine.Default(), handler: msghandler, conntype: conntype}
	return cs
}
