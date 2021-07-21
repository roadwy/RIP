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

#include "rpclient.h"

using namespace google::protobuf::util;

rpclient::rpclient(std::shared_ptr<Channel> channel) :stub_(TeamRPCService::NewStub(channel)), fatal_(nullptr)
{

}

rpclient::rpclient(const std::string& url, const std::string& http_proxy) :fatal_(nullptr)
{
	grpc::ChannelArguments channel_args;
	if (http_proxy.empty())
	{
		channel_args.SetInt(GRPC_ARG_ENABLE_HTTP_PROXY, false);
	}
	channel_args.SetString(GRPC_ARG_HTTP_PROXY, http_proxy);
	auto channel = grpc::CreateCustomChannel(url, grpc::InsecureChannelCredentials(), channel_args);
	stub_ = TeamRPCService::NewStub(channel);
}

bool rpclient::login(const std::string& username, const std::string& password)
{
	ClientContext context;
	LoginUserReq req;

	req.set_username(username);
	req.set_passwdhash(password);

	LoginUserRsp rsp;

	auto status = stub_->Login(&context, req, &rsp);
	if (status.error_code() != error::OK)
	{
		error_ = status.error_message();
		return false;
	}

	if (rsp.token().size() == 0)
	{
		error_ = rsp.error();
		return false;
	}

	token_ = rsp.token();
	return true;
}

bool rpclient::set_server_cmd(c2::ServerCmdReq& req, c2::ServerCmdRsp& rsp)
{
	ClientContext context;

	req.set_token(token_);

	auto status = stub_->ServerCmd(&context, req, &rsp);
	if (status.error_code() != error::OK)
	{
		error_ = status.error_message();
		if (fatal_)
			fatal_("teamserver: " + status.error_message());
		return false;
	}

	return true;
}

bool rpclient::set_server_cmd(int cmd_id, const std::vector<char>& data, c2::ServerCmdRsp& rsp)
{
	c2::ServerCmdReq req;
	req.set_cmd_id(cmd_id);
	req.set_byte_value(data.data(), data.size());

	return set_server_cmd(req, rsp);
}

void rpclient::start_cmd_channel_loop(PolyM::Queue* req_mq, PolyM::Queue* rsp_mq)
{
	ClientContext context;
	std::shared_ptr<ClientReaderWriter<c2::CommandReq, c2::CommandRsp> > stream(stub_->CommandChannel(&context));

	std::thread writer([&]() {
		while (true)
		{
			auto msg = req_mq->get();
			auto& data = dynamic_cast<PolyM::DataMsg<std::vector<char>>&>(*msg);
			c2::CommandReq req;
			req.ParseFromArray(data.getPayload().data(), data.getPayload().size());
			req.set_token(token_);
			if (!stream->Write(req)) {
				break;
			}
		}
		stream->WritesDone();
	});

	c2::CommandRsp rsp;
	while (stream->Read(&rsp)) {
		std::cout << "Got message " << rsp.beacon_id() << ":" << rsp.msg_id() << std::endl;
		auto size = rsp.ByteSizeLong();
		std::vector<char> data(size);
		rsp.SerializePartialToArray(data.data(), size);
		rsp_mq->put(PolyM::DataMsg<std::vector<char>>(1, data));
	}
	writer.join();

	grpc::Status status = stream->Finish();
	if (!status.ok() && fatal_)
		fatal_("teamserver: " + status.error_message());
}

void rpclient::set_fatal_callback(fatal_func func)
{
	fatal_ = func;
}

std::string rpclient::get_last_error()
{
	return error_;
}

