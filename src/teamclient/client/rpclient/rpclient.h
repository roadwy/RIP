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

#ifndef CLIENT_RPCLIENT_H
#define CLIENT_RPCLIENT_H

#include <memory>
#include <random>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include "../mq/queue.hpp"

#include "teamrpc.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace c2;

typedef std::function<void(std::string msg)> fatal_func;

class rpclient
{
public:
	rpclient(const std::string& url, const std::string& http_proxy);
	rpclient(std::shared_ptr<Channel> channel);

	bool login(const std::string& username, const std::string& password);

	bool set_server_cmd(int cmd_id, const std::vector<char>& data, c2::ServerCmdRsp& rsp);

	bool set_server_cmd(c2::ServerCmdReq& req, c2::ServerCmdRsp& rsp);

	void start_cmd_channel_loop(PolyM::Queue* req_mq, PolyM::Queue* rsp_mq);

	void set_fatal_callback(fatal_func func);

	std::string get_last_error();

private:
	std::string token_;
	std::string error_;

	std::unique_ptr<TeamRPCService::Stub> stub_;

	fatal_func fatal_;
};

#endif //CLIENT_RPCLIENT_H
