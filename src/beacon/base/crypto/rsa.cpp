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

#include "crypto.h"

int rsa_pub_encrypt(const char* N, const char* E, unsigned char* plain_data, int plain_data_size, std::vector<char>& cipher_data)
{
	CryptoPP::Integer big_n(N);
	CryptoPP::Integer big_e(E);
	CryptoPP::RSA::PublicKey pk;
	pk.Initialize(big_n, big_e);
	CryptoPP::RSAES_PKCS1v15_Encryptor encryptor(pk);
	CryptoPP::AutoSeededRandomPool rng;
	auto cipher_len = encryptor.CiphertextLength(plain_data_size);
	size_t maxLength = encryptor.FixedMaxPlaintextLength();
	cipher_data.resize(cipher_len);
	encryptor.Encrypt(rng, (const unsigned char *)plain_data, plain_data_size, (unsigned char *)cipher_data.data());
	return encryptor.FixedCiphertextLength();
}
