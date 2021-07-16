#ifndef BODY_NETIO_H_
#define BODY_NETIO_H_
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

#include "yasio/yasio.hpp"
#include "msghander/msghander.h"
#include "../base/crypto/crypto.h"

using namespace yasio;
using namespace yasio::inet;

class netio_service {

	friend class msghandler;

public:
	netio_service(std::string ip, u_short port, int netio_kind);

	~netio_service();

	void start(const highp_time_t& ht_interval);

private:

	int get_yasio_conn_kind();

	void on_connected(event_ptr& event);
	void on_packet(event_ptr& event);
	void on_connect_lost(event_ptr& event);

	void on_auth(const uint64_t& session_id, const std::vector<char>& packet);

	int send_data(std::vector<char> buffer, bool enc_data = true, completion_cb_t comp_cb = nullptr);

	int send_data(std::vector<char> buffer, bool enc_data, int msg_id, uint64_t task_id = 0, completion_cb_t comp_cb = nullptr);

	void init_session_key();

	void start_heartbeat_timer();

private:
	int netio_kind_;

	uint64_t session_id_;

	io_service service_;

	msghandler handler_;
	transport_handle_t transport_;

	deadline_timer heartbeat_timer_;
	highp_time_t ht_interval_;                 //seconds

	highp_time_t last_send_time_;

	uint8_t session_key_[xchacha20_key_len + xchacha20_iv_len];
};

#endif //BODY_NETIO_H_


