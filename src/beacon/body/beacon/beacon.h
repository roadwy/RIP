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

#ifndef BODY_BEACON_H_
#define BODY_BEACON_H_

#include <string>

namespace beacon
{
	std::string get_group();
	std::string get_beacon_id();
	std::string get_teamserver_addr();
	int get_teamserver_port();
	int get_conn_type();

	int get_heat_beat_time();

	int get_reconnect_time();
};

#endif //BODY_BEACON_H_
