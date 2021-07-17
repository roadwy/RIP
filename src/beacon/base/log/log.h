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

#ifndef _LOG_H
#define _LOG_H

#include <stdarg.h>

#define ODS_LOG_MAXLENGTH 1024

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4996)
#endif

#ifdef _WIN32
#  include <Windows.h>
#else
#define GetLastError() errno
typedef int DWORD
#  endif

#if (defined OUTPUT_LOG) || (defined _DEBUG)
#	define LOG_ERROR ErrorODS
#	define LOG_INFO InfoODS
#	define LOG_DEBUG DebugODS

#	define LOG_ERROR_E ErrorODSE
#	define LOG_INFO_E InfoODSE
#	define LOG_DEBUG_E DebugODSE
#else 
#	define LOG_ERROR
#	define LOG_INFO
#	define LOG_DEBUG

#	define LOG_ERROR_E 
#	define LOG_INFO_E 
#	define LOG_DEBUG_E 
#endif


#ifdef _DEBUG
#define ODS_LEVEL ODSLEVEL_DEBUG
#else
#define ODS_LEVEL ODSLEVEL_INFO
#endif
;
enum LOG_LEVEL
{
	ODSLEVEL_DEBUG = 0,
	ODSLEVEL_INFO,
	ODSLEVEL_ERROR,
};

void debug_log(DWORD errno_code, const char* file, int line_no, LOG_LEVEL level, const char* content, ...);

#define DebugODS(fmt, ...) debug_log(0, __FILE__, __LINE__, ODSLEVEL_DEBUG, fmt, ##__VA_ARGS__)

#define InfoODS(fmt, ...) debug_log(0, __FILE__, __LINE__, ODSLEVEL_INFO, fmt, ##__VA_ARGS__)

#define ErrorODS(fmt, ...) debug_log(0, __FILE__, __LINE__, ODSLEVEL_ERROR, fmt, ##__VA_ARGS__)

#define DebugODSE(fmt, ...) debug_log(::GetLastError(), __FILE__, __LINE__, ODSLEVEL_DEBUG, fmt, ##__VA_ARGS__)

#define InfoODSE(fmt, ...) debug_log(::GetLastError(), __FILE__, __LINE__, ODSLEVEL_INFO, fmt, ##__VA_ARGS__)

#define ErrorODSE(fmt, ...) debug_log(::GetLastError(), __FILE__, __LINE__, ODSLEVEL_ERROR, fmt, ##__VA_ARGS__)

#endif//_LOG_H
