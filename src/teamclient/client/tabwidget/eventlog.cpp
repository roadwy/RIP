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

#include "eventlog.h"

eventlog_tab::eventlog_tab(QWidget* parent)
{
	ui_.setupUi(this);
}

void eventlog_tab::on_event_log_out(std::string log)
{
	auto q_log = QString::fromStdString(log);
	q_log.replace("\n", "<br/>");
	q_log.replace("error", "<font color=red>error</font>");
	q_log.replace("err", "<font color=red>err</font>");
	q_log.replace("ERROR", "<font color=red>ERROR</font>");
	q_log.replace("ERR", "<font color=red>ERR</font>");
	q_log = QString("<font color=#E0E2E4>%1</font>").arg(q_log);
	ui_.eventlogWindow->append(q_log);
}
