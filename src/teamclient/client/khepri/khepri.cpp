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

#include "khepri.h"

#include <sstream>
#include <qdesktopservices.h>

#include "../rpclient/rpclient.h"
#include "../beacon/beacon_req.h"
#include "../beacon/beacon_msghandler.h"
#include "../beacon/configure.h"

#include "../about/about.h"

#include "../tabwidget/tabwidget.h"
#include "../tabwidget/eventlog.h"
#include "../tabwidget/process.h"
#include "../tabwidget/downloads.h"
#include "../tabwidget/filebrowser.h"
#include "../tabwidget/shell.h"
#include "../tabwidget/listeners.h"

#include "teamrpc.grpc.pb.h"
#include "client.pb.h"

extern const std::string MSGID_NAME[c2::MSGID_ARRAYSIZE];

bool beacon_sort_filter_proxy_model::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

enum beacon_tab_menu_index {
	ARCH = 0,
	BEACONID,
	FOREIGN_ADDR,
	FIRST_TM,
	UPDATE_TM,
	DETAIL,
	MENU_MAX_INDEX
};

const QString beacon_tab_label[MENU_MAX_INDEX] = { "Arch","Beacon Id","Foreign address","First Time","Update Time","Detail Info" };

enum beacon_arch_index {
	WIN = 0,
	LINUX,
	MACOS,
	ARCH_MAX_INDEX
};

const QString beacon_arch_label[ARCH_MAX_INDEX] = { "win", "linux", "macos" };

khepri::khepri(rpclient* rpc_client) :
	log_(nullptr), downloads_(nullptr), rpc_client_(rpc_client), latest_msg_label_(nullptr), listeners_(nullptr)
{
	ui_.setupUi(this);

	tabwidgets_ = new tabwidget();

	log_ = new eventlog_tab(this);
	on_view_eventlog(true);

	downloads_ = new downloads_tab(this);

	listeners_ = new listeners_tab(this, rpc_client_);

	req_mq_ = new PolyM::Queue();
	rsp_mq_ = new PolyM::Queue();

	beacon_cmd_ = new beacon_req(req_mq_, [&](std::string log) {
		emit statusmsg(QString::fromStdString(log));
	});

	beacon_msghandler_ = new beacon_msghandler([&](std::string log) {
		emit statusmsg(QString::fromStdString(log));
	});

	rpc_client_->set_fatal_callback([&](std::string msg) {
		emit fatalmsg(QString::fromStdString(msg));
	});

	auto cmd_channel = [&]() {
		rpc_client_->start_cmd_channel_loop(req_mq_, rsp_mq_);
	};

	auto cmd_thread = new std::thread(cmd_channel);
	cmd_thread->detach();
	start_msghandler();

	refresh_timer_ = new QTimer(this);
	connect(refresh_timer_, SIGNAL(timeout()), this, SLOT(on_refresh_beacon_list()));
	refresh_timer_->start(20 * 1000);

	init_target_view();

	//menuview
	connect(ui_.actionEventLog, SIGNAL(triggered(bool)), this, SLOT(on_view_eventlog(bool)));
	connect(ui_.actionDownload, SIGNAL(triggered(bool)), this, SLOT(on_view_download(bool)));
	connect(ui_.actionScreenShots, SIGNAL(triggered(bool)), this, SLOT(on_view_screenshots(bool)));
	connect(ui_.actionListener, SIGNAL(triggered(bool)), this, SLOT(on_view_listeners(bool)));
	connect(ui_.actionSetup, SIGNAL(triggered(bool)), this, SLOT(on_view_beacon_configure(bool)));


	connect(ui_.actionAbout, SIGNAL(triggered(bool)), this, SLOT(on_help_about(bool)));
	connect(ui_.actionSupport, SIGNAL(triggered(bool)), this, SLOT(on_help_support(bool)));

	connect(ui_.tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(on_close_tab(int)));

	connect(beacon_msghandler_, SIGNAL(beacon_data(const int&, const QString&, const QByteArray&)), this, SLOT(on_beacon_data(const int&, const QString&, const QByteArray&)));

	connect(this, SIGNAL(statusmsg(const QString&)), this, SLOT(on_statusmsg(const QString&)));
	connect(this, SIGNAL(fatalmsg(const QString&)), this, SLOT(on_fatalmsg(const QString&)));
}

bool khepri::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_.beaconView) menu = beacon_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}

	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyevt = dynamic_cast<QKeyEvent*>(e);
		if (keyevt->matches(QKeySequence::Refresh)) {
			on_refresh_beacon_list();
		}
	}

	return QWidget::eventFilter(obj, e);
}

void khepri::start_msghandler()
{
	auto msg_handler = [&]() {
		while (true)
		{
			auto msg = rsp_mq_->get();
			auto& data = dynamic_cast<PolyM::DataMsg<std::vector<char>>&>(*msg);

			beacon_msghandler_->on_recv_beacon_data(data.getPayload());

			on_refresh_beacon_list();
		}
	};

	msg_handler_thread_ = new std::thread(msg_handler);
	msg_handler_thread_->detach();
}


void khepri::on_refresh_beacon_list()
{
	c2::ServerCmdReq req;
	req.set_cmd_id(ct::GET_BEACONS_REQ);

	c2::ServerCmdRsp rsp;

	if (!rpc_client_->set_server_cmd(req, rsp))
		return;

	auto get_key_value = [](const std::string& detail, const std::string& key)->std::string {
		int pos1 = detail.find(key);
		if (pos1 < 0)
			return "";
		int pos2 = detail.substr(pos1).find(",");
		if (pos2 > 0 && pos1 >= 0)
			return detail.substr(pos1 + key.size(), pos2 - key.size());
		return "";
	};


	clear_item_model_data(beacon_model_, 0);
	ct::BeaconsRsp beacon_rsp;
	beacon_rsp.ParseFromArray(rsp.byte_value().data(), rsp.byte_value().size());
	auto size = beacon_rsp.beacon_size();
	for (auto i = 0; i < size; i++)
	{
		ct::BeaconInfo beacon = beacon_rsp.beacon(i);
		auto beaconid_item = new QStandardItem(QString::fromStdString(beacon.beacon_id()));
		auto ip_item = new QStandardItem(QString::fromStdString(beacon.ipaddr()));
		auto first_tm_item = new QStandardItem(QString::fromStdString(beacon.create_tm()));
		auto update_tm_item = new QStandardItem(QString::fromStdString(beacon.update_tm()));
		auto detail_item = new QStandardItem(QString::fromStdString(beacon.detail_info()));

		auto arch = QString::fromStdString(get_key_value(beacon.detail_info(), "arch:"));

		QStandardItem* arch_item = nullptr;
		if (arch.contains(beacon_arch_label[LINUX], Qt::CaseInsensitive))
			arch_item = new QStandardItem(QIcon(":/ico/ico/linux.png"), beacon_arch_label[LINUX]);
		else if (arch.contains(beacon_arch_label[MACOS], Qt::CaseInsensitive))
			arch_item = new QStandardItem(QIcon(":/ico/ico/macos.png"), beacon_arch_label[MACOS]);
		else
			arch_item = new QStandardItem(QIcon(":/ico/ico/windows.png"), beacon_arch_label[WIN]);

		auto count = beacon_model_->rowCount();

		beacon_model_->setItem(count, ARCH, arch_item);
		beacon_model_->setItem(count, BEACONID, beaconid_item);
		beacon_model_->setItem(count, FOREIGN_ADDR, ip_item);
		beacon_model_->setItem(count, FIRST_TM, first_tm_item);
		beacon_model_->setItem(count, UPDATE_TM, update_tm_item);
		beacon_model_->setItem(count, DETAIL, detail_item);
	}
}

void khepri::init_target_view()
{
	QTreeView *view = ui_.beaconView;
	beacon_model_ = new QStandardItemModel;
	proxy_beacon_ = new beacon_sort_filter_proxy_model(view);

	std::vector<std::pair<int, QString>> layout = {
		{ 50,   beacon_tab_label[ARCH]},
		{ 180, beacon_tab_label[BEACONID] },
		{ 120, beacon_tab_label[FOREIGN_ADDR] },
		{ 200, beacon_tab_label[FIRST_TM] },
		{ 200, beacon_tab_label[UPDATE_TM] },
		{ 300, beacon_tab_label[DETAIL] },
	};

	set_default_tree_view_style(view, beacon_model_, proxy_beacon_, layout);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);

	beacon_menu_ = new QMenu();
	beacon_menu_->addAction(tr("Refresh Beacon List"), this, [&] {
		on_refresh_beacon_list();
	}, QKeySequence::Refresh);

	beacon_menu_->addAction(tr("Refresh Beacon"), this, [&] {
		auto beacon_id = get_current_item_view_data(ui_.beaconView, BEACONID).toStdString();
		beacon_cmd_->send_get_host_info(beacon_id);
	});

	beacon_menu_->addAction(tr("Process List"), this, [&] {
		auto beacon_id = get_current_item_view_data(ui_.beaconView, BEACONID);
		auto tabtext = "Process " + beacon_id;
		auto widget = tabwidgets_->get_tabwidget(tabtext);
		if (beacon_id.isEmpty())
			return;

		auto arch = get_current_item_view_data(ui_.beaconView, ARCH);
		if (!arch.contains(beacon_arch_label[WIN]))
		{
			QMessageBox::warning(this, "warning", "This feature is not implemented now on " + arch);
			return;
		}

		if (!widget)
		{
			widget = new process_tab(this, beacon_id, beacon_cmd_);
			tabwidgets_->add_tabwidget(tabtext, widget);
		}
		add_tab(widget, tabtext);
	});

	beacon_menu_->addAction(tr("File List"), this, [&] {
		auto beacon_id = get_current_item_view_data(ui_.beaconView, BEACONID);
		auto tabtext = "File " + beacon_id;
		if (beacon_id.isEmpty())
			return;

		auto widget = tabwidgets_->get_tabwidget(tabtext);
		if (!widget)
		{
			widget = new file_tab(this, beacon_id, beacon_cmd_);
			tabwidgets_->add_tabwidget(tabtext, widget);
		}
		add_tab(widget, tabtext);
	});

	beacon_menu_->addAction(tr("Shell"), this, [&] {
		auto beacon_id = get_current_item_view_data(ui_.beaconView, BEACONID);
		auto arch = get_current_item_view_data(ui_.beaconView, ARCH);
		auto tabtext = "Shell " + beacon_id;
		if (beacon_id.isEmpty())
			return;

		auto widget = tabwidgets_->get_tabwidget(tabtext);
		if (!widget)
		{
			widget = new shell_tab(this, beacon_id, arch, beacon_cmd_);
			tabwidgets_->add_tabwidget(tabtext, widget);
		}
		add_tab(widget, tabtext);
	});

	on_refresh_beacon_list();
}


void khepri::add_tab(QWidget* widget, QString tab_text)
{
	auto index = get_index_by_tabtext(ui_.tabWidget, tab_text);
	if (index < 0)
		index = ui_.tabWidget->addTab(widget, tab_text);
	else
	{
		ui_.tabWidget->removeTab(index);
		ui_.tabWidget->insertTab(index, widget, tab_text);
	}
	ui_.tabWidget->setCurrentIndex(index);
}

void khepri::on_statusmsg(const QString& msg)
{
	on_latest_msg(msg);
	log_->on_event_log_out(msg.toStdString());
}

void khepri::on_fatalmsg(const QString& msg)
{
	QMessageBox::critical(NULL, "critical error", msg, QMessageBox::Yes, QMessageBox::Yes);
	QApplication* app;
	app->exit(0);
}

void khepri::on_close_tab(int index)
{
	ui_.tabWidget->removeTab(index);
}

void khepri::on_beacon_data(const int& msg_id, const QString& beacon_id, const QByteArray& data)
{
	std::ostringstream log_stream;
	log_stream << "[+] " << "recv beacon rsp:" << beacon_id.toStdString() << " " << MSGID_NAME[msg_id] << "(" << msg_id << ")" << ",datasize:" << data.size() << std::endl;

	on_statusmsg(QString::fromStdString(log_stream.str()));

	switch (msg_id)
	{
	case c2::PROCESS_INFO:
		on_process_data(beacon_id, data);
		break;
	case c2::DISK_LIST:
	case c2::FILE_LIST:
		on_file_data(msg_id, beacon_id, data);
		break;
	case c2::CMD_COMMAND:
		on_shell_data(beacon_id, data);
		break;
	case c2::ERROR_RSP:
		on_error_rsp_data(beacon_id, data);
		break;
	case c2::DOWNLOAD_FILE:
	{
		if (downloads_)
			downloads_->on_downloads_data(beacon_id, data);
		break;
	}
	default:
		break;
	}
}

void khepri::on_process_data(const QString& beacon_id, const QByteArray& data)
{
	auto tabtext = "Process " + beacon_id;
	auto process_widget = (process_tab*)(tabwidgets_->get_tabwidget(tabtext));
	if (!process_widget)
		return;

	process_widget->on_processlist_data(data);
}

void khepri::on_shell_data(const QString& beacon_id, const QByteArray& data)
{
	auto tabtext = "Shell " + beacon_id;
	auto shell_widget = (shell_tab*)(tabwidgets_->get_tabwidget(tabtext));
	if (!shell_widget)
		return;

	shell_widget->on_shell_data(data);
}

void khepri::on_file_data(const int msg_id, const QString& beacon_id, const QByteArray& data)
{
	auto tabtext = "File " + beacon_id;
	auto file_widget = (file_tab*)(tabwidgets_->get_tabwidget(tabtext));
	if (!file_widget)
		return;

	if (msg_id == c2::DISK_LIST)
		file_widget->on_disk_data(data);
	else if (msg_id == c2::FILE_LIST)
		file_widget->on_filelist_data(data);
}

void khepri::on_error_rsp_data(const QString& beacon_id, const QByteArray& data)
{
	c2::ErrorRsp error_rsp;
	if (!error_rsp.ParseFromArray(data.data(), data.size()))
		return;

	std::ostringstream log_stream;
	log_stream << "[-] " << "recv error rsp:" << beacon_id.toStdString() << " " << MSGID_NAME[error_rsp.msg_id()] << "(" << error_rsp.msg_id() << ")" << "," << error_rsp.error() << std::endl;

	on_statusmsg(QString::fromStdString(log_stream.str()));
}

void khepri::on_latest_msg(const QString& msg)
{
	if (!latest_msg_label_)
	{
		latest_msg_label_ = new QLabel();
		statusBar()->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
		statusBar()->setSizeGripEnabled(true);
		statusBar()->addPermanentWidget(latest_msg_label_);
	}

	QString log(msg);
	log.replace("\n", "<br/>");
	log.replace("error", "<font color=red>error</font>");
	log.replace("err", "<font color=red>err</font>");
	log.replace("ERROR", "<font color=red>ERROR</font>");
	log.replace("ERR", "<font color=red>ERR</font>");
	latest_msg_label_->setText(log);
}

void khepri::on_view_eventlog(bool checked)
{
	add_tab(log_, "Event Log");
}

void khepri::on_view_download(bool checked)
{
	if (!downloads_)
		downloads_ = new downloads_tab(this);

	add_tab(downloads_, "Downloads");
}

void khepri::on_view_screenshots(bool checked)
{
	QMessageBox::warning(this, "warning", "This feature is not implemented now");
}

void khepri::on_view_listeners(bool checked)
{
	if (!listeners_)
		listeners_ = new listeners_tab(this, rpc_client_);

	add_tab(listeners_, "Listeners");
}

void khepri::on_view_beacon_configure(bool checked)
{
	ct::ServerInfo info;
	if (!listeners_->get_listener(info))
		return;

	auto configure = new beacon_configure(this);
	configure->setObjectName("Beacon Configure");
	configure->refresh_listener_combo(info);
	configure->raise();
	configure->show();
}

void khepri::on_help_about(bool checked)
{
	auto about_widget = new about(this);
	about_widget->raise();
	about_widget->show();
}

void khepri::on_help_support(bool checked)
{
	QString github = "https://github.com/geemion/khepri";
	QDesktopServices::openUrl(QUrl(github.toLatin1()));
}
