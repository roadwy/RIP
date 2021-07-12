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

#include "shell.h"
#include "../beacon/beacon_req.h"

#include "taskdata.pb.h"

shell_tab::shell_tab(QWidget* parent, const QString& beacon_id, const QString& arch, beacon_req* cmd) :
	beacon_id_(beacon_id), cmd_(cmd)
{
	ui_.setupUi(this);

	connect(ui_.cmdEdit, SIGNAL(returnPressed()), this, SLOT(on_shell_input()));

	if (!arch.contains("win"))
		ui_.pathEdit->setText("/");
}

void shell_tab::on_shell_data(const QByteArray& data)
{
	c2::StrParam cmd_data;

	if (!cmd_data.ParseFromArray(data.data(), data.size()))
		return;

	on_shell_out(QString::fromStdString(cmd_data.param()));
}

bool shell_tab::eventFilter(QObject *obj, QEvent *e)
{
	return QWidget::eventFilter(obj, e);
}

void shell_tab::on_shell_out(const QString& data)
{
	QString q_log(data);
	q_log.replace("\n", "<br/>");
	q_log = QString("<font color=#E0E2E4>%1</font>").arg(q_log);
	ui_.shellwindow->append(q_log);
}

void shell_tab::on_change_dir(const QString& cmd)
{
	auto change_pattern = QRegExp("c{1}d{1}\\s", Qt::CaseInsensitive);
	int pos = cmd.indexOf(change_pattern);
	if (pos >= 0)
	{
		auto mathc_len = change_pattern.matchedLength();
		auto dir = cmd.mid(mathc_len, cmd.size() - 1);
		ui_.pathEdit->setText(dir);
	}
}

void shell_tab::on_shell_input()
{
	QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
	QString cmd = sender->text();

	on_change_dir(cmd);

	QString path = ui_.pathEdit->text();
	QString out_line = path + ">" + cmd;

	out_line = QString("<font color=#8CCF34>%1</font>").arg(out_line);
	ui_.shellwindow->append(out_line);

	auto scroll = ui_.shellwindow->verticalScrollBar();
	scroll->setSliderPosition(scroll->maximum());
	sender->clear();

	if (cmd.size() == 0)
		return;

	cmd_->send_shell_cmd(beacon_id_.toStdString(), path.toStdString(), cmd.toStdString());
}

