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

#include "inxutil.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

bool inx::get_mac(std::string &mac) {
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024];
	bool success = false;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1)
		return false;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
		return false;

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) 
	{
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) 
		{
			if (!(ifr.ifr_flags & IFF_LOOPBACK)) 
			{ 
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) 
				{
					success = true;
					break;
				}
			}
		}
	}

	if (success) 
	{
		char temp[32] = {0};
		sprintf(temp, "%02X-%02X-%02X-%02X-%02X-%02X",
		        uint8_t(ifr.ifr_addr.sa_data[0]),
                uint8_t(ifr.ifr_addr.sa_data[1]),
                uint8_t(ifr.ifr_addr.sa_data[2]),
                uint8_t(ifr.ifr_addr.sa_data[3]),
                uint8_t(ifr.ifr_addr.sa_data[4]),
                uint8_t(ifr.ifr_addr.sa_data[5]));
		mac = temp;
		return true;
	}
	return false;
}

bool inx::execute_cmd(const std::string &cmd_line, const std::string &current_dir, std::string &out_put) {
	const int buf_size = 1024;
	char temp[buf_size] = { 0 };
	char old_path[512] = { 0 };
	getcwd(old_path, sizeof(old_path));

	chdir(current_dir.c_str());

	FILE *cmd_file = nullptr;
	if ((cmd_file = popen(cmd_line.c_str(), "r")) == nullptr) {
		chdir(old_path);
		return false;
	}

	chdir(old_path);
	out_put.resize(2 * buf_size);
	while (fgets(temp, sizeof(temp), cmd_file) != nullptr) {
		if (out_put.size() - strlen(out_put.data()) < strlen(temp))
			out_put.resize(out_put.size() + buf_size);
		strcat((char*)out_put.data(), temp);
	}
	pclose(cmd_file);
	return true;
}
