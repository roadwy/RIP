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

#ifndef CLIENT_LISTENERS_H
#define CLIENT_LISTENERS_H
#include <QtWidgets>
#include "ui_listeners.h"
#include "../common/qt-wrapper.h"
#include "client.pb.h"

PROXY_FILTER(listener_sort_filter_proxy_model);

class rpclient;
class listeners_tab : public QWidget
{
	Q_OBJECT
public:
	listeners_tab(QWidget* parent, rpclient* rpc);

	bool eventFilter(QObject *obj, QEvent *e);

	bool get_listener(ct::ServerInfo& listener);

private:
	void init_listeners_view();

	void stop_listener();

	void get_listener();

	void on_listener_data(const std::string& servers_data);

	void on_error(const QString& title, const std::string& error_data);

	private slots:
	void start_listener();

private:
	QMenu listener_menu_;

	QStandardItemModel *listener_model_;
	listener_sort_filter_proxy_model *proxy_listener_list_;

	rpclient* rpc_;

	Ui_ListenersForm ui_;
};


#endif //CLIENT_LISTENERS_H
