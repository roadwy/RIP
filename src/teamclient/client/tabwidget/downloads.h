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

#ifndef CLIENT_DOWNLOADS_H
#define CLIENT_DOWNLOADS_H

#include <QtWidgets>
#include "ui_downloads.h"

#include "../common/qt-wrapper.h"

PROXY_FILTER(downloads_sort_filter_proxy_model);

class downloads_tab : public QWidget
{
	Q_OBJECT
public:
	downloads_tab(QWidget* parent);

	void on_downloads_data(const QString& beacon_id, const QByteArray& data);

	bool eventFilter(QObject *obj, QEvent *e);

private:
	void init_downloads_view();

	void save_download_file(const QString& beaconid, const QString& filename, const std::string& file_data);

	void on_reveal_file(const QString& beaconid, const QString& filename);

private:
	Ui_DownloadsForm ui_;

	QMenu download_menu_;

	QStandardItemModel *downloads_model_;
	downloads_sort_filter_proxy_model *proxy_downloads_;
};

#endif // CLIENT_DOWNLOADS_H
