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

#include "downloads.h"
#include "taskdata.pb.h"
#include "../rpclient/rpclient.h"
#include "client.pb.h"

enum download_tab_menu_index {
	BEACON_ID = 0,
	DOWNLOAD_FILENAME,
	DOWNLOAD_PATH,
	DOWNLOAD_SIZE,
	DOWNLOAD_DATE,
	MAX_INDEX
};

const QString downloads_label[MAX_INDEX] = { "beaconid", "name", "path", "size", "date" };

bool downloads_sort_filter_proxy_model::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

downloads_tab::downloads_tab(QWidget* parent, rpclient* rpc):rpc_(rpc)
{
	ui_.setupUi(this);
	init_downloads_view();

	connect(ui_.SyncButton, SIGNAL(clicked()), this, SLOT(on_sync_files()));
}

void downloads_tab::on_downloads_data(const QString& beacon_id, const QByteArray& data)
{
	c2::DownLoadFile downloadfile;
	if (!downloadfile.ParseFromArray(data.data(), data.size()))
		return;

	QFileInfo qfile(QString::fromStdString(downloadfile.file_path()));

	auto count = downloads_model_->rowCount();
	auto beacon_item = new QStandardItem(beacon_id);
	auto name_item = new QStandardItem(qfile.fileName());
	auto path_item = new QStandardItem(QString::fromStdString(downloadfile.file_path()));
	auto size_item = new QStandardItem(QString::number(downloadfile.file_data().size() / 1024) + "kb");
	auto date = new QStandardItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

	downloads_model_->setItem(count, BEACON_ID, beacon_item);
	downloads_model_->setItem(count, DOWNLOAD_FILENAME, name_item);
	downloads_model_->setItem(count, DOWNLOAD_PATH, path_item);
	downloads_model_->setItem(count, DOWNLOAD_SIZE, size_item);
	downloads_model_->setItem(count, DOWNLOAD_DATE, date);

	save_download_file(beacon_id, qfile.fileName(), downloadfile.file_data());
}

bool downloads_tab::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_.DownloadtreeView) menu = &download_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}

	return QWidget::eventFilter(obj, e);
}

void downloads_tab::init_downloads_view()
{

	QTreeView *view = ui_.DownloadtreeView;
	downloads_model_ = new QStandardItemModel;
	proxy_downloads_ = new downloads_sort_filter_proxy_model(view);

	std::vector<std::pair<int, QString>> layout = {
		{ 300, downloads_label[BEACON_ID] },
		{ 300, downloads_label[DOWNLOAD_FILENAME] },
		{ 800, downloads_label[DOWNLOAD_PATH] },
		{ 120, downloads_label[DOWNLOAD_SIZE] },
		{ 200, downloads_label[DOWNLOAD_DATE] }
	};

	set_default_tree_view_style(view, downloads_model_, proxy_downloads_, layout);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);

	download_menu_.addAction(tr("Reveal in File Explorer"), this, [&] {
		auto beaconid = get_current_item_view_data(ui_.DownloadtreeView, BEACON_ID);
		auto file_name = get_current_item_view_data(ui_.DownloadtreeView, DOWNLOAD_FILENAME);
		on_reveal_file(beaconid, file_name);
	});
}


//todo: File with the same name
void downloads_tab::save_download_file(const QString& beaconid, const QString& filename, const std::string& file_data)
{
	QString dir = QCoreApplication::applicationDirPath() + "/" + beaconid;
	QDir qdir;
	qdir.mkdir(dir);

	QString path = dir + "/" + filename;
	QFile qfile(path);
	if (qfile.open(QIODevice::WriteOnly)) {
		qfile.write(file_data.data(), file_data.size());
		qfile.close();
	}
}

void downloads_tab::on_reveal_file(const QString& beaconid, const QString& filename)
{

	QString path = QCoreApplication::applicationDirPath() + "/" + beaconid;
	QUrl url = QUrl::fromLocalFile(path);
	QDesktopServices::openUrl(url);
}

void downloads_tab::on_sync_files()
{
	c2::ServerCmdReq req;
	req.set_cmd_id(ct::SYNC_DOWNLOAD_FILES);

	c2::ServerCmdRsp rsp;

	rpc_->set_server_cmd(req, rsp);

	if (rsp.cmd_id() == ct::ERROR_MSG) {
		return;
	}
}

