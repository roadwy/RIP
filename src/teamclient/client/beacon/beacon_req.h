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

#ifndef CLIENT_BEACON_CMD_H_
#define CLIENT_BEACON_CMD_H_

#include <vector>
#include <functional>
#include <QByteArray>

#include "../mq/queue.hpp"

typedef std::function<void(std::string log)> log_func;

class beacon_req
{
public:
	beacon_req(PolyM::Queue* mq, log_func func);

	void send_get_host_info(const std::string& beacon_id);

	void send_get_process_list(const std::string& beacon_id);

	void send_kill_process(const std::string& beacon_id, int pid);

	void send_get_disk_list(const std::string& beacon_id);

	void send_get_file_list(const std::string& beacon_id, const std::string& dir);

	void send_upload_file(const std::string& beacon_id, const std::string& upload_dir, const std::string& file_name, const QByteArray& file_data);

	void send_delete_file(const std::string& beacon_id, const std::string& file_path);

	void send_download_file(const std::string& beacon_id, const std::string& file_path);

	void send_exec_file(const std::string& beacon_id, const std::string& file_path);

	void send_shell_cmd(const std::string& beacon_id, const std::string& path, const std::string& cmd);

	void send_sync_files();

private:
	void put_strparam_req(const std::string& beacon_id, int msg_id, const std::string str_param);

	void put_cmd_req(const std::string& beacon_id, int msg_id, const void* byte_value = nullptr, size_t byte_value_size = 0);

private:
	PolyM::Queue* req_mq_;
	log_func log_out_;
};

#endif // CLIENT_BEACON_CMD_H_
