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

#ifndef BASE_WIN_UTIL_H
#define BASE_WIN_UTIL_H
#include <windows.h>
#include <Tlhelp32.h>
#include <string>
#include <functional>

namespace win {

	LPCSTR get_process_user(DWORD pid);

	typedef std::function<bool(PROCESSENTRY32W &entry)> process_callback;

	bool enum_process(process_callback process_cb);

	std::string str_w2a(unsigned int code, const std::wstring& wstr);

	std::string str_c2c(unsigned int code_dst, unsigned int code_src, const std::string& str);

	std::string str_a2utf8(const std::string& str);

	std::string str_utf82a(const std::string& str);

	int get_process_sessionid(DWORD pid);

	bool os_is64();

	bool is_wow64(HANDLE phd);

	bool is_wow64(DWORD pid);

	bool is_x64(DWORD pid);

	ULONGLONG tm_to_unix_tm(ULONGLONG win_tm);

	bool execute_cmd(const std::string& cmd_line, const std::string& current_dir, std::string& out_put, DWORD time_out);

	bool  get_mac(std::string& mac);

}

#endif //BASE_WIN_UTIL_H
