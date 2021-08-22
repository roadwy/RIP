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
	pb "teamserver/internal/proto/protobuf"
	"time"
)

type TaskStatus int32

const (
	Status_Create   TaskStatus = 0
	Status_Dispatch TaskStatus = 1
	Status_Done     TaskStatus = 2
)

type TaskStore struct {
	CreatedAt time.Time
	UpdatedAt time.Time
	TaskID    uint64 `gorm:"AUTO_INCREMENT;primary_key"`
	MsgId     int32
	BeaconId  string
	ReqParam  []byte
	RspParam  []byte
	Status    TaskStatus
}

func AddTask(msgid int32, beaconid string, reqparam []byte) (err error) {
	task := TaskStore{
		MsgId:    msgid,
		BeaconId: beaconid,
		ReqParam: reqparam,
		Status:   Status_Create,
	}
	db := Instance()

	db.AutoMigrate(&TaskStore{})

	if err := db.Create(&task).Error; err != nil {
		return err
	}
	return
}

func GetTask(beaconid string) (data pb.TaskData, err error) {
	task := TaskStore{}
	db := Instance()

	if db.Where("beacon_id = ? and status = ?", beaconid, Status_Create).First(&task).RecordNotFound() {
		err = errors.New("no task")
		return
	}

	data.MsgId = task.MsgId
	data.BeaconId = task.BeaconId
	data.ByteValue = task.ReqParam
	data.TaskId = task.TaskID

	db.Model(&task).Update(TaskStore{
		Status: Status_Dispatch,
	})
	return
}

func UpdateTask(taskid uint64, rspparam []byte) (err error) {
	task := TaskStore{}
	db := Instance()

	if db.Where("task_id = ? and status = ?", taskid, Status_Dispatch).First(&task).RecordNotFound() {
		return
	}

	return db.Model(&task).Update(TaskStore{
		RspParam: rspparam,
		Status:   Status_Done,
	}).Error
}

func GetTaskRspData(msgid int32)(rspDatas []TaskStore, err error){

	db := Instance()
	query := db.Where("msg_id = ? and status = ?", msgid, Status_Done).Find(&rspDatas)
	if query.Error != nil {
		err = query.Error
		return
	}
	return
}