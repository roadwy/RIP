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

#include "listeners.h"
#include "../rpclient/rpclient.h"

enum listener_tab_menu_index
{
	NAME = 0,
	ADDR
};

bool listener_sort_filter_proxy_model::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}


listeners_tab::listeners_tab(QWidget* parent, rpclient* rpc) :rpc_(rpc)
{
	ui_.setupUi(this);
	init_listeners_view();

	connect(ui_.startButton, SIGNAL(clicked()), this, SLOT(start_listener()));

	get_listener();
}

bool listeners_tab::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_.ListenertreeView) menu = &listener_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}

	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyevt = dynamic_cast<QKeyEvent*>(e);
		if (keyevt->matches(QKeySequence::Refresh)) {
			get_listener();
		}
	}

	return QWidget::eventFilter(obj, e);
}

void listeners_tab::init_listeners_view()
{
	QTreeView *list_view = ui_.ListenertreeView;
	listener_model_ = new QStandardItemModel;
	proxy_listener_list_ = new listener_sort_filter_proxy_model(list_view);

	std::vector<std::pair<int, QString>> list_layout = {
		{ 400, "name" },
		{ 120, "address" },
	};

	set_default_tree_view_style(list_view, listener_model_, proxy_listener_list_, list_layout);
	list_view->viewport()->installEventFilter(this);
	list_view->installEventFilter(this);

	listener_menu_.addAction(tr("Stop"), this, [&] {
		stop_listener();
	});

	listener_menu_.addAction(tr("Refresh"), this, [&] {
		get_listener();
	});
}

void listeners_tab::start_listener()
{
	QString name = ui_.NamelineEdit->text();
	QString host = ui_.hostlineEdit->text();
	QString port = ui_.portlineEdit->text();
	if (name.isEmpty() || host.isEmpty()
		|| !port.contains(QRegExp("^\\d+$")))
	{
		QMessageBox::warning(this, "error", "Name or Host can't be empty, port must be number");
		return;
	}

	QString protocol = ui_.protocolcomboBox->currentText();
	QString addr = protocol + "://" + host + ":" + port;

	ct::ServerItem item;
	item.set_name(name.toStdString());
	item.set_addr(addr.toStdString());
	std::vector<char> data(item.ByteSizeLong());
	if (!item.SerializePartialToArray(data.data(), data.size()))
		return;

	c2::ServerCmdRsp rsp;
	if (!rpc_->set_server_cmd(ct::START_BEACON_SERVER, data, rsp))
		return;

	if (rsp.cmd_id() == ct::ERROR_MSG) {
		on_error("Start Listener", rsp.byte_value());
		return;
	}

	if (rsp.cmd_id() == ct::GET_BEACON_SERVER)
	{
		on_listener_data(rsp.byte_value());
	}
}

void listeners_tab::stop_listener()
{
	auto name = get_current_item_view_data(ui_.ListenertreeView, NAME);
	auto addr = get_current_item_view_data(ui_.ListenertreeView, ADDR);

	if (name.isEmpty())
	{
		QMessageBox::warning(this, "error", "You must choose one Listener");
		return;
	}

	ct::ServerItem item;
	item.set_name(name.toStdString());
	item.set_addr(addr.toStdString());
	std::vector<char> data(item.ByteSizeLong());
	if (!item.SerializePartialToArray(data.data(), data.size()))
		return;

	c2::ServerCmdRsp rsp;
	if (!rpc_->set_server_cmd(ct::STOP_BEACON_SERVER, data, rsp))
		return;

	if (rsp.cmd_id() == ct::ERROR_MSG) {
		on_error("Stop Listener", rsp.byte_value());
		return;
	}

	if (rsp.cmd_id() == ct::GET_BEACON_SERVER)
	{
		on_listener_data(rsp.byte_value());
	}
}

void listeners_tab::get_listener()
{
	c2::ServerCmdReq req;
	req.set_cmd_id(ct::GET_BEACON_SERVER);

	c2::ServerCmdRsp rsp;

	rpc_->set_server_cmd(req, rsp);

	if (rsp.cmd_id() == ct::ERROR_MSG) {
		on_error("Refresh Listener", rsp.byte_value());
		return;
	}

	if (rsp.cmd_id() == ct::GET_BEACON_SERVER)
	{
		on_listener_data(rsp.byte_value());
	}
}

bool listeners_tab::get_listener(ct::ServerInfo& listener)
{
	c2::ServerCmdReq req;
	req.set_cmd_id(ct::GET_BEACON_SERVER);

	c2::ServerCmdRsp rsp;

	rpc_->set_server_cmd(req, rsp);

	if (rsp.cmd_id() == ct::ERROR_MSG) {
		on_error("Refresh Listener", rsp.byte_value());
		return false;
	}

	if (!listener.ParseFromArray(rsp.byte_value().data(), rsp.byte_value().size()))
		return false;

	return true;
}

void listeners_tab::on_listener_data(const std::string& servers_data)
{
	ct::ServerInfo server_info;
	if (!server_info.ParseFromArray(servers_data.data(), servers_data.size()))
		return;

	clear_item_model_data(listener_model_, 0);

	auto size = server_info.server_size();
	for (auto i = 0; i < size; i++)
	{
		auto server = server_info.server(i);

		auto count = listener_model_->rowCount();
		auto name_item = new QStandardItem(QString::fromStdString(server.name()));
		auto addr_item = new QStandardItem(QString::fromStdString(server.addr()));

		listener_model_->setItem(count, NAME, name_item);
		listener_model_->setItem(count, ADDR, addr_item);
	}
}

void listeners_tab::on_error(const QString& title, const std::string& error_data)
{
	ct::ErrorMsg error_msg;
	if (!error_msg.ParseFromArray(error_data.data(), error_data.size()))
		return;

	QMessageBox::warning(this, title, QString::fromStdString(error_msg.error()));
}

