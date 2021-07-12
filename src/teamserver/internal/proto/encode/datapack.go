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

package encode

import (
	"bytes"
	"encoding/binary"
	pb "teamserver/internal/proto/protobuf"
)

type IDataPack interface {
	GetHeadLen() uint32
	Pack(msg NetioData) ([]byte, error)
	Unpack([]byte) (NetioData, error)
}

type DataPack struct{}

func NewDataPack() *DataPack {
	return &DataPack{}
}

func (dp *DataPack) GetHeadLen() uint32 {
	return DefaultHeadLength
}

func (dp *DataPack) Pack(msg INetioData, conntype pb.CONN_TYPE) ([]byte, error) {

	dataBuff := bytes.NewBuffer([]byte{})

	if conntype == pb.CONN_TYPE_CONNNAME_UDP {
		if err := binary.Write(dataBuff, binary.BigEndian, msg.GetDataLen()); err != nil {
			return nil, err
		}
	}

	if err := binary.Write(dataBuff, binary.LittleEndian, msg.IsEncrypted()); err != nil {
		return nil, err
	}

	if err := binary.Write(dataBuff, binary.LittleEndian, msg.GetSessionId()); err != nil {
		return nil, err
	}

	var reserved int32 = 0
	if err := binary.Write(dataBuff, binary.LittleEndian, reserved); err != nil {
		return nil, err
	}

	if err := binary.Write(dataBuff, binary.LittleEndian, msg.GetData()); err != nil {
		return nil, err
	}

	return dataBuff.Bytes(), nil
}

func (dp *DataPack) Unpack(binaryData []byte) (INetioData, error) {

	dataBuff := bytes.NewReader(binaryData)
	msg := &NetioData{}

	if err := binary.Read(dataBuff, binary.BigEndian, &msg.Size); err != nil {
		return nil, err
	}

	if err := binary.Read(dataBuff, binary.LittleEndian, &msg.Encrypted); err != nil {
		return nil, err
	}

	if err := binary.Read(dataBuff, binary.LittleEndian, &msg.SessionId); err != nil {
		return nil, err
	}

	if err := binary.Read(dataBuff, binary.LittleEndian, &msg.Reserved1); err != nil {
		return nil, err
	}

	msg.Data = make([]byte, msg.Size)
	if err := binary.Read(dataBuff, binary.LittleEndian, &msg.Data); err != nil {
		return nil, err
	}

	return msg, nil
}
