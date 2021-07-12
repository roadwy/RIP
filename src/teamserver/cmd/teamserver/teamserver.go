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
	"log"
	"sync"
	"teamserver/internal/conf"
	"teamserver/internal/rpc"
	"teamserver/pkg/mq"
)

func main() {
	serverConf := &conf.ServerConf{}
	err := serverConf.GetUserConf()
	if err != nil {
		log.Fatal(err)
	}

	var wg sync.WaitGroup
	wg.Add(1)

	//msgqueue
	mqclient := mq.NewClient()
	mqclient.SetConditions(1)

	//param
	maxRecvSize, maxSendSize := 200*1024*1024, 200*1024*1024

	go func() {
		err := rpc.NewTeamRpcService(conf.GlobalConf.BindHost, mqclient, maxSendSize, maxRecvSize)
		log.Fatal(err)
	}()

	wg.Wait()
}
