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

#ifndef _CLIENT_BEACON_MSGHANDLER_H
#define _CLIENT_BEACON_MSGHANDLER_H

#include <QObject>
#include <vector>
#include <functional>

#include "../mq/queue.hpp"

typedef std::function<void(std::string log)> log_func;

class beacon_msghandler : public QObject
{
	Q_OBJECT
public:
	beacon_msghandler(log_func func);

	void on_recv_beacon_data(const std::vector<char>& data);

signals:
	void beacon_data(const int&, const QString&, const QByteArray&);

private:
	log_func log_out_;
};


#endif // _CLIENT_BEACON_MSGHANDLER_H
