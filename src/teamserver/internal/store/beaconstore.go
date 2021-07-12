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

package store

import (
	"errors"
	"github.com/golang/protobuf/proto"
	pb "teamserver/internal/proto/protobuf"
	"time"
)

type BeaconStore struct {
	CreatedAt time.Time
	UpdatedAt time.Time
	BeaconId  string `gorm:"primary_key"`
	IpAddr    string
	Detail    []byte
}

func UpdateBeacon(beaconid string, ipaddr string, detail []byte) (err error) {
	beacon := BeaconStore{
		BeaconId: beaconid,
		IpAddr:   ipaddr,
		Detail:   detail,
	}

	db := Instance()
	db.AutoMigrate(&BeaconStore{})

	//create
	if err = db.Create(&beacon).Error; err == nil {
		return
	}

	beacon_old := BeaconStore{}

	if db.First(&beacon_old, "beacon_id = ?", beaconid).RecordNotFound() {
		return errors.New("beacon id not found")
	}

	return db.Model(&beacon_old).Update(BeaconStore{
		IpAddr: ipaddr,
		Detail: detail,
	}).Error
}

func GetBeacons() (beacons_data []byte, err error) {
	var beacon []BeaconStore

	db := Instance()

	query := db.Find(&beacon)
	if query.Error != nil {
		err = query.Error
		return
	}

	rsp := &pb.BeaconsRsp{}
	for _, v := range beacon {
		value := &pb.MapValueData{}
		proto.Unmarshal(v.Detail, value)
		var detail string
		for k, v := range value.DictValue {
			detail = detail + k + ":" + v + ", "
		}

		b := &pb.BeaconInfo{
			CreateTm:   v.CreatedAt.String(),
			UpdateTm:   v.UpdatedAt.String(),
			Ipaddr:     v.IpAddr,
			BeaconId:   v.BeaconId,
			DetailInfo: detail,
		}

		rsp.Beacon = append(rsp.Beacon, b)
	}

	beacons_data, err = proto.Marshal(rsp)
	return
}
