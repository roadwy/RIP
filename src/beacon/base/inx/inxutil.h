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

#ifndef INX_UTIL_H
#define INX_UTIL_H
#include <string>

namespace inx {
	bool get_mac(std::string& mac);

	bool execute_cmd(const std::string& cmd_line, const std::string& current_dir, std::string& out_put);
}

#endif //INX_UTIL_H
