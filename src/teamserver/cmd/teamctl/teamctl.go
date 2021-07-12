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

package main

import (
	"context"
	"fmt"
	"google.golang.org/grpc"
	"io"
	"log"
	pb "teamserver/internal/proto/protobuf"
	"time"
)

func main() {
	conn, err := grpc.Dial("localhost:50051", grpc.WithInsecure())
	if err != nil {
		log.Fatalf("grpc.Dial err: %v", err)
	}

	defer conn.Close()

	client := pb.NewTeamRPCServiceClient(conn)

	err = SetServerCmd(client, &pb.ServerCmdReq{
		Token:     "",
		CmdId:     int32(pb.CMDID_GET_BEACONS_REQ),
		ByteValue: nil,
	})

	err = printChannleCmd(client, &pb.CommandReq{
		BeaconId:  "111111111",
		MsgId:     int32(pb.MSGID_HOST_INFO_REQ),
		ByteValue: nil,
	})

}

func SetServerCmd(client pb.TeamRPCServiceClient, r *pb.ServerCmdReq) (err error) {

	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(time.Duration(15*time.Second)))
	defer cancel()

	rsp, err := client.ServerCmd(ctx, r)
	if err != nil {
		fmt.Println(err)
		return err
	}
	fmt.Println(rsp)
	return
}

func printChannleCmd(client pb.TeamRPCServiceClient, r *pb.CommandReq) (err error) {
	channel, err := client.CommandChannel(context.Background())
	if err != nil {
		fmt.Println(err)
		return err
	}

	for {
		err = channel.Send(r)
		if err != nil {
			fmt.Println(err)
			return err
		}

		resp, err := channel.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}
		fmt.Print(resp)
		time.Sleep(5 * time.Second)
	}

	channel.CloseSend()

	return nil

}
