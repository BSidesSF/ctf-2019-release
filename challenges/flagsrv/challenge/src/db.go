package main

import (
	"database/sql"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
	_ "github.com/mattn/go-sqlite3"
	"log"
	"os"
)

type FlagData struct {
	Username string
	Flag     string
	Readonly bool
}

var globalDB *sql.DB

const sqliteTest = "/tmp/flagsrv.db3"
const mysqlEnv = "MYSQL_DB"

type LoginResult int

const (
	LoginFailed LoginResult = iota
	LoginSuccess
	LoginNeedsMFA
)

func getDB() (*sql.DB, error) {
	if globalDB != nil {
		return globalDB, nil
	}
	if _, err := os.Stat(sqliteTest); err == nil {
		db, err := sql.Open("sqlite3", sqliteTest)
		if err != nil {
			return nil, err
		}
		globalDB = db
	} else {
		db, err := sql.Open("mysql", os.Getenv(mysqlEnv))
		if err != nil {
			return nil, err
		}
		globalDB = db
	}
	if err := globalDB.Ping(); err != nil {
		return nil, err
	}
	return globalDB, nil
}

func mustGetDB() *sql.DB {
	db, err := getDB()
	if err != nil {
		log.Fatalf("Error connecting to database: %s", err)
	}
	return db
}

func verifyUsernamePassword(username, password string) LoginResult {
	db := mustGetDB()
	qry := `SELECT username,twofactor FROM users WHERE username = ? AND password = ?;`
	if rows, err := db.Query(qry, username, password); err != nil {
		log.Printf("Error verifying username/password: %s", err)
		return LoginFailed
	} else {
		defer rows.Close()
		if !rows.Next() {
			return LoginFailed
		}
		var ret_username string
		var twofactor int
		if err := rows.Scan(&ret_username, &twofactor); err != nil {
			log.Printf("Error scanning username/password: %s", err)
			return LoginFailed
		}
		if ret_username != username {
			log.Printf("Username mismatch!? %s vs %s", username, ret_username)
			return LoginFailed
		}
		if twofactor > 0 {
			return LoginNeedsMFA
		}
		return LoginSuccess
	}
}

func getFlagDataForUser(username string) *FlagData {
	db := mustGetDB()
	qry := `SELECT flag, readonly FROM users WHERE username = ?;`
	if rows, err := db.Query(qry, username); err != nil {
		log.Printf("Error querying for flag: %s", err)
		return nil
	} else {
		defer rows.Close()
		if !rows.Next() {
			return nil
		}
		var flag sql.NullString
		var readonly sql.NullInt64
		if err := rows.Scan(&flag, &readonly); err != nil {
			log.Printf("Error scanning flag/readonly: %s", err)
			return nil
		}
		fd := &FlagData{
			Flag:     flag.String,
			Username: username,
		}
		if readonly.Valid && readonly.Int64 == 1 {
			fd.Readonly = true
		}
		return fd
	}
}

func (fd *FlagData) Save() error {
	db := mustGetDB()
	if fd.Readonly {
		return fmt.Errorf("Attempt to save readonly flagdata!")
	}
	qry := `UPDATE users SET flag=? WHERE username=?;`
	if _, err := db.Exec(qry, fd.Flag, fd.Username); err != nil {
		log.Printf("Error updating flag: %s", err)
		return err
	}
	return nil
}

func dbRegister(username, password string) error {
	db := mustGetDB()
	qry := `INSERT INTO users(username, password, twofactor) VALUES(?, ?, 0);`
	if _, err := db.Exec(qry, username, password); err != nil {
		log.Printf("Error registering new user: %s", err)
		// TODO: return a more friendly error
		return err
	}
	return nil
}

func dbEnroll2FA(username string) error {
	db := mustGetDB()
	qry := `UPDATE users SET twofactor=1 WHERE username=?;`
	if _, err := db.Exec(qry, username); err != nil {
		log.Printf("Error enable 2FA: %s", err)
		return err
	}
	return nil
}
