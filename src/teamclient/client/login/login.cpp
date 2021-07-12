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

#include "login.h"
#include "../rpclient/rpclient.h"
#include "../khepri/khepri.h"

#include <QtWidgets/QMessageBox>
#include<QSettings>


login::login(QWidget *parent /*= Q_NULLPTR*/) :rpc_client_(nullptr), main_window_(nullptr)
{
	ui_.setupUi(this);

	connect(ui_.connectBtn, SIGNAL(clicked()), this, SLOT(on_connectbtn()));
	load_connect_cfg();
}

void login::on_connectbtn()
{
	this->hide();

	auto host = ui_.hostEdit->text().trimmed().toStdString();
	auto port = ui_.portEdit->text().trimmed().toStdString();
	auto url = host + ":" + port;

	auto username = ui_.userEdit->text().trimmed().toStdString();
	auto password = ui_.passwordEdit->text().trimmed().toStdString();
	if (!rpc_client_)
		rpc_client_ = new rpclient(url);

	save_connect_cfg();

	if (rpc_client_->login(username, password))
	{
		main_window_ = new khepri(rpc_client_);
		main_window_->show();
	}
	else {
		QMessageBox::warning(this, "error", rpc_client_->get_last_error().c_str());
		QApplication* app;
		app->exit(0);
	}
}

#define LOAD_CFG_VARIANT(x) cfg.value(#x).toString()
#define SAVE_CFG_VARIANT(x) cfg.setValue(#x, x)

void login::save_connect_cfg()
{
	QSettings cfg("connect_cfg.ini", QSettings::IniFormat);
	host_ = ui_.hostEdit->text().trimmed();
	port_ = ui_.portEdit->text().trimmed();
	username_ = ui_.userEdit->text().trimmed();
	SAVE_CFG_VARIANT(host_);
	SAVE_CFG_VARIANT(port_);
	SAVE_CFG_VARIANT(username_);
	cfg.sync();
}

void login::load_connect_cfg()
{
	QSettings cfg("connect_cfg.ini", QSettings::IniFormat);
	host_ = LOAD_CFG_VARIANT(host_);
	port_ = LOAD_CFG_VARIANT(port_);
	username_ = LOAD_CFG_VARIANT(username_);
	ui_.hostEdit->setText(host_);
	ui_.portEdit->setText(port_);
	ui_.userEdit->setText(username_);
}

