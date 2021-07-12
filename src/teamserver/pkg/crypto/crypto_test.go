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

package crypto

import (
	"bytes"
	"teamserver/pkg/crypto/chacha20"
	"teamserver/pkg/crypto/rsa"
	"testing"
)

var privateKey = []byte(`
-----BEGIN RSA PRIVATE KEY-----
MIICXgIBAAKBgQDIOkm7kTU6m8ALDqPE2nn049vOHSKkekpdzwcOmfYiAwg+sbli2ToVTlUwM51SDNdOfbvogbH4KeDqwYnb0bjMhk4wxfZPD1wHXotRc/ERN2iaZ6Km0ImJjlW41zDFicNoj9qre3NpFkAAjXyJEDEdQrVCGYRYvcAYwUtpNVhavwIDAQABAoGBAMWSnobyte9rGIjQnVD1tDmtTYuIvFJISXFfg7souPK+wzf57tBXQTUc4np5s9buzNWqw+ydbZtO151N9FZwD0Qij7YtfvoxNrEzMRd1SzaFMxKOZqNSV8uW6v7o8ISS7E7YIrnGvJIXTRAOiehu3Zy3maQc6wVLJVDDf0kqHPRhAkEA6VShkeUXxJ9t08O+YEX+CV3KHXBfmXBIMKaKBAShnVUyx2+TW0C0ZyGTJMwjPhpVIhWDlNwQ1mVTsMjc+CBWlQJBANuuU2ABCLyyG423PQmOHSuAm+80kUEoPkBpWDOHa+kR7yR1mdaKne4Yzv/7aTC9qmKQ+LdtSassGu7YWoM3OwMCQAm+mBTQvYJfqiWK6jt5ENfxS8yY8dUlpE4r1l2+l8VLVpiPp1bLR/16oHuL7vjb/qwyu9EOs8FQcANVEC1opFUCQQDTajZk+znMV2A7B3CfZHxgJFptX9q2qSMX3An9NUO9vvu1y9OsbCTHQmrcYbj/Jlj2mOwzouK18DFPUTnyc9G/AkEAwyVQYY6tQNUJ6srRA1MrS8K0iwS7PRyt/u+ziBS8Va/rzRJe9s30pXsiRubUhCCKUvuCWWwOrxhPQB+VGx7EXQ==
-----END RSA PRIVATE KEY-----
`)

var publicKey = []byte(`
-----BEGIN PUBLIC KEY-----
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDIOkm7kTU6m8ALDqPE2nn049vOHSKkekpdzwcOmfYiAwg+sbli2ToVTlUwM51SDNdOfbvogbH4KeDqwYnb0bjMhk4wxfZPD1wHXotRc/ERN2iaZ6Km0ImJjlW41zDFicNoj9qre3NpFkAAjXyJEDEdQrVCGYRYvcAYwUtpNVhavwIDAQAB
-----END PUBLIC KEY-----
`)

var E = "10001"
var N = "c83a49bb91353a9bc00b0ea3c4da79f4e3dbce1d22a47a4a5dcf070e99f62203083eb1b962d93a154e5530339d520cd74e7dbbe881b1f829e0eac189dbd1b8cc864e30c5f64f0f5c075e8b5173f11137689a67a2a6d089898e55b8d730c589c3688fdaab7b73691640008d7c8910311d42b542198458bdc018c14b6935585abf"

func TestNewRsaEncode(t *testing.T) {
	rsa, err := rsa.NewRsaEncode(publicKey, privateKey)
	if err != nil {
		t.Error(err)
	}

	if (E != rsa.E) || (N != rsa.N) {
		t.Error("N E parse error")
	}

	src := "abc"

	dst, err := rsa.PubEncode([]byte(src))
	if err != nil {
		t.Error(err)
	}

	dec, err := rsa.PrivateDecode(dst)
	if err != nil {
		t.Error(err)
	}

	if !bytes.Equal(dec, []byte(src)) {
		t.Error("Rsa error")
	}
}

func TestChaCha20(t *testing.T) {
	var key []byte = []byte{0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00}
	var nonce []byte = []byte{0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08}

	enc, err := chacha20.New(key, nonce)
	if err != nil {
		t.Error(err)
	}
	var dst []byte = make([]byte, 3)
	src := "abc"
	enc.XORKeyStream(dst, []byte(src))

	dec, err := chacha20.New(key, nonce)
	if err != nil {
		t.Error(err)
	}

	dec.XORKeyStream(dst, dst)
	if !bytes.Equal(dst, []byte(src)) {
		t.Error("chacha20 error")
	}
}
