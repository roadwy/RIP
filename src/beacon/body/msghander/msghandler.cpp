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

#include "../netio/netio.h"
#include "msghander.h"
#include <type_traits>
#include "../base/log/log.h"

msghandler::msghandler(netio_service& service) :netio_service_(service)
{

}

msghandler::~msghandler()
{

}

void msghandler::put_msg(std::vector<char>&& msg)
{
	msg_queue_.put(DataMsg<std::vector<char>>(1, msg));
}

void msghandler::start_handler()
{
	handler_working_ = true;
	auto msg_handler = [&]() {
		while (handler_working_)
		{
			auto msg = msg_queue_.get();
			auto& data = dynamic_cast<DataMsg<std::vector<char>>&>(*msg);
			LOG_INFO("msg_size:%d\n", data.getPayload().size());

			std::vector<char> msg_rsp_data;
			handle_msg(data.getPayload(), msg_rsp_data);
			if (msg_rsp_data.size() != 0)
				netio_service_.send_data(msg_rsp_data, true, nullptr);

		}
		LOG_INFO("exit msghandler loop");
	};

	msg_handler_thread_ = new std::thread(msg_handler);

	msg_handler_thread_->detach();
}

void msghandler::stop_handler()
{
	if (!handler_working_)
	{
		handler_working_ = false;
		if (msg_handler_thread_->joinable())
			msg_handler_thread_->join();
	}
	LOG_INFO("stop msghandler");
}

void msghandler::set_msg_handler(int msg_id, msg_handler_exec handler)
{
	msg_handlers_[msg_id] = handler;
}

msg_handler_exec msghandler::get_msg_handler(int msg_id)
{
	auto handler = msg_handlers_.find(msg_id);
	if (handler != msg_handlers_.end())
		return handler->second;

	return nullptr;
}

bool msghandler::handle_msg(const std::vector<char>& task_msg, std::vector<char>& task_rsp)
{
	c2::TaskData task;
	if (!task.ParseFromArray(task_msg.data(), task_msg.size()))
	{
		LOG_ERROR("error  parse packet");
		return false;
	}

	auto msg_id = task.msg_id();
	auto handler = get_msg_handler(msg_id);
	if (!handler)
	{
		LOG_ERROR("no msg handler:msg_id:%d", msg_id);
		return false;
	}

	return handler(task, task_rsp);
}