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

#include "process.h"
#include "../beacon/beacon_req.h"

enum process_tab_menu_index
{
	NAME = 0,
	PID,
	PPID,
	ARCH,
	SESSION,
	USER,
	MAX_INDEX
};

const QString processlist_label[MAX_INDEX] = { "NAME","PID", "PPID","ARCH", "SESSION", "USER" };

bool processlist_sort_filter_proxy_model::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

bool processtree_sort_filter_proxy_model::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

process_tab::process_tab(QWidget* parent, const QString& beacon_id, beacon_req* cmd) :
	beacon_id_(beacon_id), cmd_(cmd)
{
	ui_.setupUi(this);

	init_process_view();

	connect(ui_.ProcessListView, SIGNAL(pressed(QModelIndex)), this, SLOT(on_processlist_pressed(QModelIndex)));
	connect(ui_.ProcessTreeView, SIGNAL(pressed(QModelIndex)), this, SLOT(on_processtree_pressed(QModelIndex)));

	cmd_->send_get_process_list(beacon_id_.toStdString());
}

void process_tab::on_processlist_data(const QByteArray& data)
{
	c2::ProcessListData list_data;
	if (!list_data.ParseFromArray(data.data(), data.size()))
		return;

	on_process_tree_view(list_data);
	on_process_list_view(list_data);
}

bool process_tab::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_.ProcessListView) menu = process_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}

	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyevt = dynamic_cast<QKeyEvent*>(e);
		if (keyevt->matches(QKeySequence::Refresh)) {
			cmd_->send_get_process_list(beacon_id_.toStdString());
		}
	}

	return QWidget::eventFilter(obj, e);
}

void process_tab::init_process_view()
{
	QTreeView *list_view = ui_.ProcessListView;
	processlist_model_ = new QStandardItemModel;
	proxy_processlist_ = new processlist_sort_filter_proxy_model(list_view);

	std::vector<std::pair<int, QString>> list_layout = {
		{ 250, processlist_label[NAME] },
		{ 120, processlist_label[PID] },
		{ 120, processlist_label[PPID] },
		{ 200, processlist_label[ARCH] },
		{ 120, processlist_label[SESSION]},
		{ 200, processlist_label[USER] }
	};

	set_default_tree_view_style(list_view, processlist_model_, proxy_processlist_, list_layout);
	list_view->viewport()->installEventFilter(this);
	list_view->installEventFilter(this);

	process_menu_ = new QMenu();
	process_menu_->addAction(tr("Refresh"), this, [&] {
		cmd_->send_get_process_list(beacon_id_.toStdString());
	});

	process_menu_->addAction(tr("Kill"), this, [&] {
		auto pid = get_current_item_view_data(ui_.ProcessListView, PID);
		cmd_->send_kill_process(beacon_id_.toStdString(), pid.toInt());
	});

	QTreeView *tree_view = ui_.ProcessTreeView;
	processtree_model_ = new QStandardItemModel;
	proxy_processtree_ = new processtree_sort_filter_proxy_model(tree_view);

	std::vector<std::pair<int, QString>> tree_layout = {
		{ 500, processlist_label[NAME] },
		{ 120, processlist_label[PID] }
	};

	set_default_tree_view_style(tree_view, processtree_model_, proxy_processtree_, tree_layout);
	tree_view->viewport()->installEventFilter(this);
	tree_view->installEventFilter(this);
}

void process_tab::on_process_tree_view(const c2::ProcessListData& data)
{
	std::set<int> render_set;

	auto beacon_pid = data.beacon_pid();

	std::function<void(QStandardItem* parent, QStandardItem* pid_item, c2::ProcessItem item, int seq)> render_item =
		[&](QStandardItem* parent, QStandardItem* name_item, c2::ProcessItem item, int seq) {

		render_set.insert(item.pid());
		QStandardItem *pid_item = new QStandardItem(QString::number(item.pid()));

		if (item.pid() == beacon_pid)
		{
			pid_item->setBackground(QColor(253, 251, 172));
			name_item->setBackground(QColor(253, 251, 172));
		}

		if (parent == nullptr)
		{
			processtree_model_->setItem(seq, NAME, name_item);
			processtree_model_->setItem(seq, PID, pid_item);
		}
		else {
			parent->appendRow(name_item);
			parent->setChild(seq, PID, pid_item);
		}
	};


	std::function<void(QStandardItem *parent, c2::ProcessItem item, int seq)> render_process_tree =
		[&](QStandardItem *parent, c2::ProcessItem item, int seq) {

		QStandardItem *name_item = new QStandardItem(QString::fromStdString(item.name()));
		render_item(parent, name_item, item, seq);
		c2::ProcessListData child;
		get_proc_childs(item.pid(), data, child);
		for (auto i = 0; i < child.item_size(); i++)
		{
			auto child_item = child.item(i);
			render_process_tree(name_item, child_item, i);
		}
	};

	clear_item_model_data(processtree_model_, 0);

	for (auto i = 0; i < data.item_size(); i++)
	{
		auto item = data.item(i);
		auto pid_item = render_set.find(item.pid());
		if (pid_item == render_set.end()) {
			render_process_tree(nullptr, data.item(i), processtree_model_->rowCount());
		}
	}
	ui_.ProcessTreeView->expandAll();
}

void process_tab::on_process_list_view(const c2::ProcessListData& data)
{
	auto beacon_pid = data.beacon_pid();
	auto size = data.item_size();
	clear_item_model_data(processlist_model_, 0);
	for (auto i = 0; i < size; i++)
	{
		c2::ProcessItem process = data.item(i);
		auto pid = new QStandardItem(QString::number(process.pid()));
		auto ppid = new QStandardItem(QString::number(process.ppid()));
		auto name = new QStandardItem(QString::fromStdString(process.name()));
		auto arch = new QStandardItem(process.is64() ? "x64" : "x86");
		auto session = new QStandardItem(QString::number(process.session()));
		auto user = new QStandardItem(QString::fromStdString(process.user()));

		auto count = processlist_model_->rowCount();
		if (process.pid() == beacon_pid)
		{
			pid->setBackground(QColor(253, 251, 172));
			ppid->setBackground(QColor(253, 251, 172));
			name->setBackground(QColor(253, 251, 172));
			arch->setBackground(QColor(253, 251, 172));
			session->setBackground(QColor(253, 251, 172));
			user->setBackground(QColor(253, 251, 172));
		}

		processlist_model_->setItem(count, PID, pid);
		processlist_model_->setItem(count, PPID, ppid);
		processlist_model_->setItem(count, NAME, name);
		processlist_model_->setItem(count, ARCH, arch);
		processlist_model_->setItem(count, SESSION, session);
		processlist_model_->setItem(count, USER, user);
	}
}

void process_tab::get_proc_childs(int pid, const c2::ProcessListData& data, c2::ProcessListData& child)
{
	if (pid == 0)
		return;

	auto size = data.item_size();
	for (auto i = 0; i < size; i++)
	{
		c2::ProcessItem process = data.item(i);
		if (process.ppid() == pid)
		{
			auto temp = child.add_item();
			temp->set_pid(process.pid());
			temp->set_name(process.name());
		}
	}
}

void process_tab::on_processlist_pressed(QModelIndex index)
{
	QAbstractItemModel* m = (QAbstractItemModel*)index.model();
	auto pid = m->index(index.row(), PID).data().toString();

	auto view = ui_.ProcessTreeView;
	std::function<bool(QModelIndex idx)> locate_process_tree = [&](QModelIndex idx)->bool {
		int rows = processtree_model_->rowCount(idx);
		for (int i = 0; i < rows; i++) {
			QString pid_temp;
			QModelIndex child_name;
			QStandardItem *item;
			if (idx == view->rootIndex()) {
				child_name = processtree_model_->index(i, NAME);
				item = processtree_model_->itemFromIndex(child_name);
				pid_temp = processtree_model_->index(i, PID).data(Qt::DisplayRole).toString();
			}
			else {
				item = processtree_model_->itemFromIndex(idx);
				child_name = item->child(i, NAME)->index();
				pid_temp = item->child(i, PID)->data(Qt::DisplayRole).toString();
			}

			if (pid_temp == pid) {
				auto idx = proxy_processtree_->mapFromSource(child_name);
				view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
				view->scrollTo(idx);
				return true;
			}
			if (processtree_model_->itemFromIndex(child_name)->hasChildren()) {
				if (locate_process_tree(child_name)) {
					return true;
				}
			}
		}
		return false;
	};

	locate_process_tree(view->rootIndex());
}

void process_tab::on_processtree_pressed(QModelIndex index)
{
	auto pid = index.sibling(index.row(), PID).data().toString();
	auto view = ui_.ProcessListView;

	std::function<bool(QModelIndex idx)> locate_process_list = [&](QModelIndex idx)->bool {
		int rows = processlist_model_->rowCount(idx);
		for (int i = 0; i < rows; i++) {
			QString pid_temp;
			QModelIndex child_name;
			QStandardItem *item;
			if (idx == view->rootIndex()) {
				child_name = processlist_model_->index(i, PID);
				item = processlist_model_->itemFromIndex(child_name);
				pid_temp = processlist_model_->index(i, PID).data(Qt::DisplayRole).toString();
			}
			if (pid_temp == pid) {
				auto idx = proxy_processlist_->mapFromSource(child_name);
				view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
				view->scrollTo(idx);
				return true;
			}
		}
		return false;
	};

	locate_process_list(view->rootIndex());
}

