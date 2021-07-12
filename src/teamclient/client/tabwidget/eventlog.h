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

#ifndef CLIENT_EVENTLOG_H_
#define CLIENT_EVENTLOG_H_
#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include "ui_eventlog.h"

class eventlog_tab : public QWidget
{
	Q_OBJECT
public:
	eventlog_tab(QWidget* parent);

	public slots:
	void on_event_log_out(std::string log);

private:
	Ui_EventLog ui_;
};

#endif //CLIENT_EVENTLOG_H_
