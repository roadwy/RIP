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

#include "winutil.h"
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

LPCSTR win::get_process_user(DWORD pid)
{
	static char username[256] = { 0 };
	memset(username, 0, sizeof(username));
	strcpy(username, "");

	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (process == NULL)
		return username;

	HANDLE token = NULL;
	DWORD size = 0;

	char domain[256] = { 0 };
	DWORD domain_size = 256;
	DWORD name_size = 256;

	SID_NAME_USE    SNU;
	PTOKEN_USER token_user = NULL;
	__try
	{
		if (!OpenProcessToken(process, TOKEN_QUERY, &token))
		{
			__leave;
		}

		if (!GetTokenInformation(token, TokenUser, token_user, size, &size))
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				__leave;
			}
		}

		token_user = NULL;
		token_user = (PTOKEN_USER)malloc(size);
		if (token_user == NULL)
		{
			__leave;
		}

		if (!GetTokenInformation(token, TokenUser, token_user, size, &size))
		{
			__leave;
		}

		if (LookupAccountSidA(NULL, token_user->User.Sid, username, &name_size, domain, &domain_size, &SNU) != 0)
		{
			return username;
		}
	}
	__finally
	{
		if (token_user != NULL)
			free(token_user);
	}
	return username;
}

bool win::enum_process(process_callback process_cb)
{
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE) {
		return false;
	}
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(entry);
	if (!Process32FirstW(snap, &entry)) {
		CloseHandle(snap);
		return false;
	}
	do {
		if (!process_cb(entry))
			break;
	} while (Process32NextW(snap, &entry));
	CloseHandle(snap);
	return true;
}

std::string win::str_w2a(unsigned int code, const std::wstring& wstr)
{
	try {
		if (wstr.empty()) return "";
		int templen = WideCharToMultiByte(code, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
		if (!templen) return "";
		char* tempstr = new(std::nothrow) char[templen];
		if (!tempstr) return "";
		memset(tempstr, 0, templen);
		WideCharToMultiByte(code, 0, wstr.c_str(), -1, tempstr, templen, NULL, NULL);
		std::string str(tempstr);
		delete[] tempstr;
		return str;
	}
	catch (...) {
		return "";
	}
}

std::string win::str_c2c(unsigned int code_dst, unsigned int code_src, const std::string& str)
{
	try {
		if (str.empty()) return "";
		int templen = MultiByteToWideChar(code_src, 0, str.c_str(), -1, NULL, 0);
		if (!templen) return "";
		wchar_t* tempstr = new(std::nothrow) wchar_t[templen * 2];
		if (!tempstr) return "";
		memset(tempstr, 0, templen * 2);
		MultiByteToWideChar(code_src, 0, str.c_str(), -1, tempstr, templen);
		templen = WideCharToMultiByte(code_dst, 0, tempstr, -1, NULL, 0, NULL, NULL);
		if (!templen) {
			delete[] tempstr;
			return "";
		}
		char* tempstr2 = new(std::nothrow) char[templen];
		if (!tempstr2) {
			delete[] tempstr;
			return "";
		}
		memset(tempstr2, 0, templen);
		WideCharToMultiByte(code_dst, 0, tempstr, -1, tempstr2, templen, NULL, NULL);
		std::string result(tempstr2);
		delete[] tempstr;
		delete[] tempstr2;
		return std::move(result);
	}
	catch (...) {
		return "";
	}
}

std::string win::str_a2utf8(const std::string& str)
{
	return str_c2c(CP_UTF8, CP_ACP, str);
}

std::string win::str_utf82a(const std::string& str)
{
	return str_c2c(CP_ACP, CP_UTF8, str);
}

int win::get_process_sessionid(DWORD pid) {
	DWORD sessionid = 0;
	ProcessIdToSessionId(pid, &sessionid);
	return sessionid;
}

bool win::os_is64()
{
	bool result = false;
	SYSTEM_INFO si;
	RtlZeroMemory(&si, sizeof(SYSTEM_INFO));
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		result = true;
	return result;
}

bool win::is_wow64(HANDLE phd)
{
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS pIsWow64Process = NULL;
	BOOL is_wow64 = FALSE;
	pIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (NULL != pIsWow64Process) {
		if (!pIsWow64Process(phd, &is_wow64)) {
			// handle error
		}
	}
	return is_wow64 == TRUE;
}

bool win::is_wow64(DWORD pid)
{
	bool result = false;
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (process) {
		result = is_wow64(process);
		CloseHandle(process);
	}
	return result;
}

bool win::is_x64(DWORD pid) {
	return os_is64() && !is_wow64(pid);
}

ULONGLONG win::tm_to_unix_tm(ULONGLONG win_tm) {

	const ULONGLONG WINDOWS_TICK = 10000000;
	const ULONGLONG SEC_TO_UNIX_EPOCH = 11644473600LL;
	return (win_tm / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);
}

bool win::execute_cmd(const std::string& cmd_line, const std::string& current_dir, std::string& out_put, DWORD time_out)
{
	const int block_size = 512;
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(sa);
	HANDLE read_hd = NULL;
	HANDLE write_hd = NULL;
	BOOL ret = CreatePipe(&read_hd, &write_hd, &sa, 64 * 0x1000);
	if (!ret) {
		return false;
	}

	STARTUPINFOA si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	GetStartupInfoA(&si);
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = write_hd;
	std::string cmd = "cmd.exe /c " + cmd_line;
	ret = CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, 0, NULL, current_dir.c_str(), &si, &pi);
	CloseHandle(write_hd);
	if (!ret) {
		CloseHandle(read_hd);
		return false;
	}
	if (WaitForSingleObject(pi.hProcess, time_out) == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		CloseHandle(read_hd);
		return false;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	bool result = false;
	char* buffer = new(std::nothrow) char[block_size];
	if (buffer == nullptr) {
		CloseHandle(read_hd);
		return false;
	}
	while (1) {
		DWORD read_len = 0;
		if (ReadFile(read_hd, buffer, block_size, &read_len, NULL)) {
			out_put.append(buffer, read_len);
			if (block_size > read_len) {
				result = true;
				break;
			}
		}
		else {
			if (GetLastError() == ERROR_BROKEN_PIPE) {
				result = true;
				break;
			}
		}
	}
	CloseHandle(read_hd);
	delete[] buffer;
	return result;
}


bool win::get_mac(std::string& mac)
{
	bool ret = false;

	ULONG out_len = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO adapter_info = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	if (adapter_info == NULL)
		return false;

	if (GetAdaptersInfo(adapter_info, &out_len) == ERROR_BUFFER_OVERFLOW)
	{
		free(adapter_info);
		adapter_info = (IP_ADAPTER_INFO *)malloc(out_len);
		if (adapter_info == NULL)
			return false;
	}

	if (GetAdaptersInfo(adapter_info, &out_len) == NO_ERROR)
	{
		for (PIP_ADAPTER_INFO adapter_ptr = adapter_info; adapter_ptr != NULL; adapter_ptr = adapter_ptr->Next)
		{
			if (adapter_ptr->Type != MIB_IF_TYPE_ETHERNET)
				continue;

			if (adapter_ptr->AddressLength != 6)
				continue;
			char str[32];
			sprintf(str, "%02X-%02X-%02X-%02X-%02X-%02X",
                    uint8_t(adapter_ptr->Address[0]),
                    uint8_t(adapter_ptr->Address[1]),
                    uint8_t(adapter_ptr->Address[2]),
                    uint8_t(adapter_ptr->Address[3]),
                    uint8_t(adapter_ptr->Address[4]),
                    uint8_t(adapter_ptr->Address[5]));
			mac = str;
			ret = true;
			break;
		}
	}

	free(adapter_info);
	return ret;
}
