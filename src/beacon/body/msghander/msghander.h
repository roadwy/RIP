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

#ifndef BODY_MSG_HANDER_H_
#define BODY_MSG_HANDER_H_
#include <vector>
#include <map>
#include <thread>
#include "queue.hpp"
#include "taskdata.pb.h"

using namespace PolyM;

class netio_service;

typedef std::function<bool(c2::TaskData& task, std::vector<char>& msg_rsp)> msg_handler_exec;

class msghandler
{
public:
	msghandler(netio_service& service);
	~msghandler();

	void put_msg(std::vector<char>&& msg);
	void start_handler();
	void stop_handler();

	bool handle_msg(const std::vector<char>& task_msg, std::vector<char>& task_rsp);

	void set_msg_handler(int msg_id, msg_handler_exec handler);
	msg_handler_exec get_msg_handler(int msg_id);

private:
	netio_service& netio_service_;
	Queue msg_queue_;
	std::thread* msg_handler_thread_;
	bool handler_working_;

	std::map<int, msg_handler_exec> msg_handlers_;
};

#endif //BODY_MSG_HANDER_H_
