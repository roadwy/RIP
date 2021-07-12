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

RSA* createRSA(unsigned char* key, bool pubkey)
{
	RSA* rsa = nullptr;
	BIO* keybio = nullptr;
	keybio = BIO_new_mem_buf(key, strlen((const char*)key));
	if (keybio == NULL) {
		return nullptr;
	}

	if (pubkey)
		rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	else
		rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

	BIO_free(keybio);
	return rsa;
}

RSA* new_pub_rsa(const char* N, const char* E)
{
	BIGNUM *bne = BN_new();
	if (!bne)
		return nullptr;

	BIGNUM *bnn = BN_new();
	if (!bnn) {
		BN_free(bne);
		return nullptr;
	}

	BIGNUM * bnd = BN_new();
	if (!bnd) {
		BN_free(bnd);
		return nullptr;
	}

	RSA* rsa = RSA_new();
	if (!rsa) {
		BN_free(bne);
		BN_free(bnn);
		BN_free(bnd);
		return nullptr;
	}
	BN_hex2bn(&bne, E);
	BN_hex2bn(&bnn, N);
	RSA_set0_key(rsa, bnn, bne, bnd);
	return rsa;
}

int rsa_pub_encrypt(RSA* rsa_key, unsigned char* data, int data_size, std::vector<char>& enc_data)
{
	int  enc_size = 0;
	do
	{
		int rsa_len = RSA_size(rsa_key);
		enc_data.resize(rsa_len);
		enc_size = RSA_public_encrypt(data_size, data, (unsigned char*)enc_data.data(), rsa_key, RSA_PKCS1_PADDING);
	} while (false);

	RSA_set0_key(rsa_key, 0, 0, 0);
	RSA_free(rsa_key);
	if (!enc_size)
		return 0;

	return enc_size;
}