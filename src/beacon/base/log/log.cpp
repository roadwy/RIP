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

#include "log.h"
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <string>
#include <string.h>


#ifdef _WIN32
#else
typedef char TCHAR;
#define _TRUNCATE ((size_t)-1)
#define GetCurrentThreadId pthread_self
#  endif


void debug_log(DWORD errno_code, const char* file, int line_no, LOG_LEVEL level, const char* content, ...)
{
	if (level < ODS_LEVEL)
		return;

	char log_content[ODS_LOG_MAXLENGTH + 1] = { 0 };

	DWORD thread_id = ::GetCurrentThreadId();
	std::string code_file = file;
	std::string::size_type pos = code_file.find_last_of('\\');
	if (pos != std::string::npos && pos + 1 < code_file.size()) code_file = code_file.substr(pos + 1);

	std::string level_str;
	switch (level)
	{
	case ODSLEVEL_DEBUG:
		level_str = "[D]";
		break;
	case ODSLEVEL_INFO:
		level_str = "[+]";
		break;
	case ODSLEVEL_ERROR:
		level_str = "[-]";
		break;
	default:
		level_str = "[?]";
		break;
	}
#if _DEBUG
	int written = sprintf(log_content, "%s [%s:%d] %u ", level_str.c_str(), code_file.c_str(), line_no, thread_id);
#else
	int written = sprintf(log_content, "%s ", level_str.c_str());
#endif

	va_list ap;
	va_start(ap, content);
	vsnprintf(log_content + written, ODS_LOG_MAXLENGTH - written, content, ap);
	va_end(ap);

	if (errno_code != 0)
	{
		char last_error[16] = { 0 };
		sprintf(last_error, "E:%d", errno_code);

		size_t len = strlen(log_content);
		if (len + strlen(last_error) < ODS_LOG_MAXLENGTH)
		{
			strcat(log_content, last_error);
		}
	}
	std::string log = log_content;
	std::transform(log.begin(), log.end(), log.begin(), toupper);

	printf("%s\n", log.c_str());

#ifdef _WIN32
	OutputDebugStringA(log_content);
#endif
}