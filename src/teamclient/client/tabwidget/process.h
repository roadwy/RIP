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

#ifndef CLIENT_PROCESS_H
#define CLIENT_PROCESS_H 

#include <QtWidgets>
#include "ui_process.h"

#include "../common/qt-wrapper.h"
#include "taskdata.pb.h"

PROXY_FILTER(processlist_sort_filter_proxy_model);

PROXY_FILTER(processtree_sort_filter_proxy_model);

class beacon_req;

class process_tab : public QWidget
{
	Q_OBJECT
public:
	process_tab(QWidget* parent, const QString& beacon_id, beacon_req* cmd);

	void on_processlist_data(const QByteArray& data);

	bool eventFilter(QObject *obj, QEvent *e);

private:
	void init_process_view();

	void on_process_tree_view(const c2::ProcessListData& data);

	void on_process_list_view(const c2::ProcessListData& data);

	void get_proc_childs(int pid, const c2::ProcessListData& data, c2::ProcessListData& child);

	private slots:
	void on_processlist_pressed(QModelIndex index);
	void on_processtree_pressed(QModelIndex index);

private:

	QMenu *process_menu_;

	QStandardItemModel *processlist_model_;
	processlist_sort_filter_proxy_model *proxy_processlist_;

	QStandardItemModel *processtree_model_;
	processtree_sort_filter_proxy_model* proxy_processtree_;

	beacon_req* cmd_;

	QString beacon_id_;

	Ui_ProcessForm ui_;
};

#endif //CLIENT_PROCESS_H
