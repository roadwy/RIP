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

#ifndef CLIENT_LOGIN_H_
#define CLIENT_LOGIN_H_

#include <QtWidgets/QMainWindow>
#include "ui_login.h"

class rpclient;
class khepri;

class login : public QWidget {
	Q_OBJECT
public:
	login(QWidget *parent = Q_NULLPTR);

	private slots:
	void on_connectbtn();

private:
	void save_connect_cfg();
	void load_connect_cfg();

private:
	QString host_;
	QString port_;
	QString username_;
	QString http_proxy_;

	Ui_Connect ui_;
	rpclient* rpc_client_;
	khepri* main_window_;
};

#endif //CLIENT_LOGIN_H_
