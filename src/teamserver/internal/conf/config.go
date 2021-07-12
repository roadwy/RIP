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

package conf

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"errors"
	"flag"
	"io/ioutil"
	"os"
	innerRsa "teamserver/pkg/crypto/rsa"
)

const (
	CmdReqTopic = "teamclient_req"
	CmdRspTopic = "beacon_rsp"
)

type ServerConf struct {
	Dbname           string //default sqlite3 db file
	BindHost         string //teamserver bind addr
	RsaEncode        *innerRsa.RsaEncode
	TeamclientSecret string //teamclient  connect password
	PublicKeyFile    string //beacon rsa public key file
	PrivateKeyFile   string //beacon rsa private key file
}

var GlobalConf *ServerConf

func (s *ServerConf) GetUserConf() (err error) {
	flag.StringVar(&s.Dbname, "d", "khepri.db", "default sqlite3 db file, default:khepri.db")
	flag.StringVar(&s.BindHost, "l", "0.0.0.0:50051", "teamserver listen at addr, default:0.0.0:50051")
	flag.StringVar(&s.TeamclientSecret, "p", "", "teamclient connect password")
	flag.StringVar(&s.PublicKeyFile, "-pubkey", "publickey.pem", "beacon rsa public key file, default:publickey.pem")
	flag.StringVar(&s.PrivateKeyFile, "-privatekey", "privatekey.pem", "beacon rsa private key file, default:privatekey.pem")

	h := false
	flag.BoolVar(&h, "h", false, "help usage")
	flag.Parse()
	if h || len(s.TeamclientSecret) == 0 {
		flag.Usage()
		return errors.New("param error")
	}

	err = s.getRsa()
	if err != nil {
		return err
	}
	GlobalConf = s
	return nil
}

func (s *ServerConf) genRsaKey(bits int, publicKeyFile string, privateKeyFile string) (err error) {
	privateKey, err := rsa.GenerateKey(rand.Reader, bits)
	if err != nil {
		return
	}
	derStream := x509.MarshalPKCS1PrivateKey(privateKey)
	block := &pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: derStream,
	}
	file, err := os.Create(privateKeyFile)
	if err != nil {
		return
	}
	err = pem.Encode(file, block)
	if err != nil {
		return
	}

	publicKey := &privateKey.PublicKey
	derPkix, err := x509.MarshalPKIXPublicKey(publicKey)
	if err != nil {
		return
	}
	block = &pem.Block{
		Type:  "PUBLIC KEY",
		Bytes: derPkix,
	}
	file, err = os.Create(publicKeyFile)
	if err != nil {
		return
	}
	err = pem.Encode(file, block)
	if err != nil {
		return
	}
	return
}

func (s *ServerConf) getRsa() (err error) {
	publicKey, _ := ioutil.ReadFile(s.PublicKeyFile)
	privateKey, _ := ioutil.ReadFile(s.PrivateKeyFile)
	if len(publicKey) == 0 || len(privateKey) == 0 {
		err = s.genRsaKey(1024, s.PublicKeyFile, s.PrivateKeyFile)
		if err != nil {
			return err
		}
		publicKey, _ = ioutil.ReadFile(s.PublicKeyFile)
		privateKey, _ = ioutil.ReadFile(s.PrivateKeyFile)
	}

	s.RsaEncode, err = innerRsa.NewRsaEncode(publicKey, privateKey)
	if err != nil {
		return err
	}

	return nil
}
