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

#ifndef CLIENT_QT_WRAPPER_H
#define CLIENT_QT_WRAPPER_H

#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QIcon>
#include <QPainter>
#include <QProxyStyle>
#include <QSize>
#include <QTreeView>
#include <QModelIndex>
#include <QStandardItem>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QPoint>

#define PROXY_FILTER(classname) \
class classname : public QSortFilterProxyModel {\
	Q_OBJECT \
public: \
	classname(QWidget *parent) {};\
	~classname() {}; \
protected: \
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const; \
};

#define TR(str) QObject::tr(str)

void clear_item_model_data(QStandardItemModel* model, int pos = 0);

QString get_current_item_view_data(QAbstractItemView *view, int column);

int get_index_by_tabtext(QTabWidget* tabwidget, const QString tabtext);

void set_default_tree_view_style(QTreeView* view, QStandardItemModel* model, QSortFilterProxyModel *proxy,
	std::vector<std::pair<int, QString>>& layout);

QString get_str_tm(uint64_t uinx_tm);

#endif //CLIENT_QT_WRAPPER_H
