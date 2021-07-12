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
	"github.com/jinzhu/gorm"
	_ "github.com/jinzhu/gorm/dialects/sqlite"
	"log"
	"sync"
	"teamserver/internal/conf"
)

var db *gorm.DB
var once sync.Once

func initDB() {
	dbname := conf.GlobalConf.Dbname
	var err error
	db, err = gorm.Open("sqlite3", dbname)
	if err != nil {
		log.Fatalln(err)
	}
}

func Instance() *gorm.DB {
	once.Do(initDB)
	return db
}
