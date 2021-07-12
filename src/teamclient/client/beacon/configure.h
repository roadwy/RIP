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

#ifndef CLIENT_CONFIGGURE_
#define CLIENT_CONFIGGURE_

#include <QtWidgets>
#include "ui_configure.h"
#include "client.pb.h"

class beacon_configure : public QWidget
{
	Q_OBJECT
public:
	beacon_configure(QWidget* parent);
	void refresh_listener_combo(const ct::ServerInfo& listener);

private:
	bool get_beacon_connect_param(int& netio_type, std::string& teamserver, int& teamserver_port);
	char* get_flag(const void* beacon_buf, int beacon_size, const void* flag, int flag_size);

	private slots :
	void generate_beacon();

private:
	Ui_ConfigureForm ui_;
};

#endif // CLIENT_CONFIGGURE_
