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

package rsa

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"errors"
	"fmt"
)

type RsaEncode struct {
	PubKey     *rsa.PublicKey
	PrivateKey *rsa.PrivateKey
	N          string
	E          string
}

func NewRsaEncode(pubKey, privateKey []byte) (encode *RsaEncode, err error) {
	pubBlock, _ := pem.Decode(pubKey)
	if pubBlock == nil {
		err = errors.New("publick error")
		return
	}

	privateBlock, _ := pem.Decode(privateKey)
	if privateBlock == nil {
		err = errors.New("private error")
		return
	}

	pubInterface, err := x509.ParsePKIXPublicKey(pubBlock.Bytes)
	if err != nil {
		return
	}

	privateInterface, err := x509.ParsePKCS1PrivateKey(privateBlock.Bytes)
	if err != nil {
		return
	}

	pubRsaKey := pubInterface.(*rsa.PublicKey)
	N := fmt.Sprintf("0x%x", pubRsaKey.N)
	E := fmt.Sprintf("0x%x", pubRsaKey.E)

	encode = &RsaEncode{
		PubKey:     pubRsaKey,
		PrivateKey: privateInterface,
		N:          N,
		E:          E,
	}
	return encode, nil
}

func (r *RsaEncode) PubEncode(data []byte) ([]byte, error) {
	return rsa.EncryptPKCS1v15(rand.Reader, r.PubKey, data)
}

func (r *RsaEncode) PrivateDecode(ciphertext []byte) ([]byte, error) {
	return rsa.DecryptPKCS1v15(rand.Reader, r.PrivateKey, ciphertext)
}
