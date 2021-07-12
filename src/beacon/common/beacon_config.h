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

#ifndef BEACON_CONFIG_H
#define BEACON_CONFIG_H

#define CONFIG_FLAG 0xdeadbeef

#pragma pack(push,1)
typedef struct  _CONNECT_INFO
{
	unsigned int flag;

	char group[10];

	int netio_type;
	char teamserver[64];
	int teamserver_port;

	//todo: proxy
	int proxy_type;
	char proxy_addr_url[64];
	char proxy_username[32];
	char proxy_password[32];

	//todo:config
	int heat_beat_interval;                    //seconds										   
	int	try_connect_interval;                  //connect lost, reconnect time interval seconds

}CONNECT_INFO, *PCONFIG_INFO;
#pragma pack(pop)

#endif // BEACON_CONFIG_H