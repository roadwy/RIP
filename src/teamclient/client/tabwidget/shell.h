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

#ifndef CLIENT_SHELL_H
#define CLIENT_SHELL_H

#include <QtWidgets>
#include "ui_shell.h"

class beacon_req;

class shell_tab : public QWidget
{
	Q_OBJECT
public:
	shell_tab(QWidget* parent, const QString& beacon_id, const QString& arch, beacon_req* cmd);

	void on_shell_data(const QByteArray& data);

	bool eventFilter(QObject *obj, QEvent *e);

private:
	void on_shell_out(const QString& data);

	void on_change_dir(const QString& cmd);

	private slots:
	void on_shell_input();

private:
	beacon_req* cmd_;

	QString beacon_id_;

	Ui_CmdForm ui_;
};



#endif // CLIENT_SHELL_H
