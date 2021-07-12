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

#include "qt-wrapper.h"

void clear_item_model_data(QStandardItemModel* model, int pos)
{
	model->removeRows(pos, model->rowCount());
}

QString get_current_item_view_data(QAbstractItemView *view, int column)
{
	auto idx = view->currentIndex();
	return idx.sibling(idx.row(), column).data().toString();
}

int get_index_by_tabtext(QTabWidget* tabwidget, const QString tabtext)
{
	auto count = tabwidget->count();
	for (int i = 0; i < count; i++)
	{
		QString temp = tabwidget->tabText(i);
		if (temp == tabtext)
			return i;
	}
	return -1;
}

void set_default_tree_view_style(QTreeView* view, QStandardItemModel* model,
	QSortFilterProxyModel *proxy, std::vector<std::pair<int, QString>>& layout)
{
	proxy->setSourceModel(model);
	proxy->setDynamicSortFilter(true);
	proxy->setFilterKeyColumn(1);
	view->setModel(proxy);
	view->selectionModel()->setModel(proxy);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->header()->setStretchLastSection(true);
	view->setSortingEnabled(true);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	view->setFocusPolicy(Qt::NoFocus);
	view->setSelectionBehavior(QTreeView::SelectRows);
	QStringList name_list;
	for (int i = 0; i < layout.size(); i++) {
		name_list << layout[i].second;
	}
	model->setHorizontalHeaderLabels(name_list);
	for (int i = 0; i < layout.size(); i++) {
		if (layout[i].first)
			view->setColumnWidth(i, layout[i].first);
	}
}

QString get_str_tm(uint64_t uinx_tm)
{
	QDateTime timestamp;
	timestamp.setTime_t(uinx_tm);
	return timestamp.toString("yyyy-MM-dd hh:mm:ss");
}

