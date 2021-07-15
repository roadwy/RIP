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

#ifndef BODY_CRYPTO_H_
#define BODY_CRYPTO_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/chacha.h>


#define chacha20_nonce_len	 8
#define chacha20_key_len     32

void chacha20_crypt(const uint8_t key[chacha20_key_len], const uint8_t nonce[chacha20_nonce_len], uint8_t* bytes, size_t n_bytes, uint64_t counter = 0);

int rsa_pub_encrypt(const char* N, const char* E, unsigned char* data, int data_size, std::vector<char>& enc_data);

#endif


