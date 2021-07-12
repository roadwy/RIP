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

#include "tabwidget.h"

void tabwidget::add_tabwidget(const QString& label, QWidget* widget)
{
	QMutexLocker lock(&mutex_);

	tabwidgets_.insert(label, widget);
}

QWidget* tabwidget::get_tabwidget(const QString& label)
{
	QMutexLocker locker(&mutex_);

	auto widget = tabwidgets_.find(label);
	if (widget != tabwidgets_.end())
		return widget.value();
	else
		return nullptr;
}
