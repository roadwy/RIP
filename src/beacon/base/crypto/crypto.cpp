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
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/chacha.h>

void generate_random_block(uint8_t* block, size_t block_size)
{
	CryptoPP::AutoSeededRandomPool prng;
	prng.GenerateBlock(block, block_size);
}

void xchacha20(const uint8_t key[xchacha20_key_len], const uint8_t iv[xchacha20_iv_len], uint8_t* bytes, size_t n_bytes, uint64_t counter /*= 0*/)
{
	//cryptopp xchacha20 initial block counter of 1(https://cryptopp.com/wiki/XChaCha20)

	CryptoPP::XChaCha20::Encryption enc;
	enc.SetKeyWithIV(key, xchacha20_key_len, iv, xchacha20_iv_len);
	enc.ProcessData(bytes, bytes, n_bytes);
}

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

std::string md5_string(const std::string& msg)
{
	std::string digest;
	CryptoPP::Weak1::MD5 md5;
	CryptoPP::HashFilter hashfilter(md5);
	hashfilter.Attach(new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false));
	hashfilter.Put(reinterpret_cast<const unsigned char*>(msg.c_str()), msg.length());
	hashfilter.MessageEnd();
	return std::move(digest);
}
