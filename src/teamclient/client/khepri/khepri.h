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

#ifndef CLIENT_MAIN_WINDOW_
#define CLIENT_MAIN_WINDOW_

#include<QMainWindow>
#include <thread>

#include "../mq/queue.hpp"
#include "../common/qt-wrapper.h"

#include "ui_khepri.h"

class rpclient;
class beacon_req;
class beacon_msghandler;
class tabwidget;
class eventlog_tab;
class downloads_tab;
class listeners_tab;

PROXY_FILTER(beacon_sort_filter_proxy_model);
class khepri : public QMainWindow
{
	Q_OBJECT
public:
	khepri(rpclient* rpc_client);

public:
	bool eventFilter(QObject *obj, QEvent *e);

	void start_msghandler();
private:
	void init_target_view();

	void add_tab(QWidget* widget, QString tab_text);

	void on_latest_msg(const QString& msg);

	void on_process_data(const QString& beacon_id, const QByteArray& data);
	void on_shell_data(const QString& beacon_id, const QByteArray& data);
	void on_file_data(const int msg_id, const QString& beacon_id, const QByteArray& data);
	void on_error_rsp_data(const QString& beacon_id, const QByteArray& data);

	private slots:
	void on_view_eventlog(bool checked);
	void on_view_download(bool checked);
	void on_view_screenshots(bool checked);
	void on_view_listeners(bool checked);
	void on_view_beacon_configure(bool checked);

	void on_help_about(bool checked);
	void on_help_support(bool checked);
	void on_close_tab(int index);

	void on_beacon_data(const int& msg_id, const QString& beacon_id, const QByteArray& data);
	void on_statusmsg(const QString& msg);
	void on_fatalmsg(const QString& msg);

	void on_refresh_beacon_list();

	void delete_beacon(const QString& beacon_id);

signals:
	void statusmsg(const QString& msg);
	void fatalmsg(const QString& msg);

private:
	Ui_MainWindow ui_;

	QLabel* latest_msg_label_;

	QTimer* refresh_timer_;

	eventlog_tab* log_;
	downloads_tab* downloads_;
	listeners_tab* listeners_;

	beacon_req* beacon_cmd_;
	beacon_msghandler* beacon_msghandler_;

	rpclient* rpc_client_;
	PolyM::Queue* req_mq_;
	PolyM::Queue* rsp_mq_;

	QMenu *beacon_menu_;

	QStandardItemModel *beacon_model_;
	beacon_sort_filter_proxy_model *proxy_beacon_;

	std::thread* msg_handler_thread_;

	tabwidget* tabwidgets_;
};

#endif //CLIENT_MAIN_WINDOW_
