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
#include <QString>
#include "beacon_msghandler.h"
#include "teamrpc.pb.h"
#include "taskdata.pb.h"

#include "../tabwidget/tabwidget.h"
#include "../tabwidget/process.h"



beacon_msghandler::beacon_msghandler(log_func func) :log_out_(func)
{

}

void beacon_msghandler::on_recv_beacon_data(const std::vector<char>& data)
{
	c2::CommandRsp rsp;
	if (!rsp.ParseFromArray(data.data(), data.size()))
	{
		log_out_("on_recv_beacon_data error parse data");
		return;
	}

	emit beacon_data(rsp.msg_id(), QString::fromStdString(rsp.beacon_id()), QByteArray::fromStdString(rsp.byte_value()));
}