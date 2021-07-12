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

#ifndef CLIENT_FILE_BROWSER_H
#define CLIENT_FILE_BROWSER_H

#include <QtWidgets>
#include "ui_filebrowser.h"
#include "../common/qt-wrapper.h"
#include "taskdata.pb.h"

PROXY_FILTER(filelist_sort_filter_proxy_model);

PROXY_FILTER(filetree_sort_filter_proxy_model);

class beacon_req;

class file_tab : public QWidget
{
	Q_OBJECT
public:
	file_tab(QWidget* parent, const QString& beacon_id, beacon_req* cmd);

	void on_disk_data(const QByteArray& data);
	void on_filelist_data(const QByteArray& data);

	bool eventFilter(QObject *obj, QEvent *e);

private:
	void init_file_view();

	void on_filelist_data(c2::FileListData& data);
	void on_filetree_data(c2::FileListData& data);

	void set_current_path(const QString& path);

	void on_upload_file(const QString& upload_dir);

	void on_delete_file(const QString& path);

	void on_exec_file(const QString& path);

	void on_download_file(const QString& path);

	QStringList get_path_ino_name(const QString& dir_path);

	QStandardItem* get_parent_item(const QString& parent_dir);

	private slots:
	void on_filetree_doubleclicked(QModelIndex index);
	void on_uplevel_dir();

private:
	QMenu file_menu_;

	QStandardItemModel *filelist_model_;
	filelist_sort_filter_proxy_model *proxy_filelist_;

	QStandardItemModel *filetree_model_;
	filetree_sort_filter_proxy_model *proxy_filetree_;

	beacon_req* cmd_;

	QString beacon_id_;

	Ui_File ui_;

};

#endif //CLIENT_FILE_BROWSER_H
