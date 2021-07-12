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

#ifndef _BODY_PROTOCOL_HEADER_
#define _BODY_PROTOCOL_HEADER_

#define CONNECT_FLAG 0xDEADBEEF
#define NETIO_FLAG 0x10010

#pragma pack(1)
struct netio_header {
	int size;
	bool encrypted;
	uint64_t session_id;
	int    reserved1;
	netio_header(int s, bool e, uint64_t id) :encrypted(e), size(s), session_id(id) {};
	//4 + 1 + 8 + 4
};
#pragma pack()

#endif //_BODY_PROTOCOL_HEADER_