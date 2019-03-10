package main

import (
	"database/sql"
	"fmt"
	_ "github.com/mattn/go-sqlite3"
	"log"
)

var dbName = "/tmp/sequel.db3"
var globalConn *sql.DB

func Connect() (*sql.DB, error) {
	if globalConn != nil {
		return globalConn, nil
	}
	globalConn, err := sql.Open("sqlite3", dbName)
	if err != nil {
		return nil, err
	}
	err = globalConn.Ping()
	if err != nil {
		return nil, err
	}
	return globalConn, nil
}

func MustConnect() *sql.DB {
	db, err := Connect()
	if err != nil {
		panic(err)
	}
	return db
}

func PrepareDB() {
	db := MustConnect()
	prepareTables(db)
	prepareUsers(db)
	prepareReviews(db)
	prepareNotes(db)
}

func VerifyUsernamePassword(username string, password string) (validUser bool, validPass bool) {
	validUser = false
	validPass = false
	db := MustConnect()
	stmt := fmt.Sprintf(`SELECT password FROM userinfo WHERE username = "%s";`, username)
	log.Printf("Executing query: %s", stmt)
	if rows, err := db.Query(stmt); err != nil {
		log.Printf("Error with verify query: %s", err)
		return
	} else {
		defer rows.Close()
		if !rows.Next() {
			return
		}
		var pass string
		if err := rows.Scan(&pass); err != nil {
			log.Printf("Error scanning pass: %s", err)
			return
		}
		validUser = true
		validPass = (pass == password)
	}
	return
}

func VerifyUserinfo(uinfo *Userinfo) (bool, bool) {
	if uinfo == nil {
		return false, false
	}
	return VerifyUsernamePassword(uinfo.Username, uinfo.Password)
}

func GetReviewsForUser(username string) ([]Review, error) {
	db := MustConnect()
	res := make([]Review, 0)
	stmt := `SELECT reviews.Id, reviews.Name, reviews.Rating, notes.Note
		FROM reviews LEFT JOIN (SELECT * FROM notes WHERE notes.username = ?)
		AS notes ON reviews.Id = notes.Review;`
	if rows, err := db.Query(stmt, username); err != nil {
		log.Printf("Error querying for reviews: %s", err)
		return nil, err
	} else {
		for rows.Next() {
			var id int
			var name string
			var rating int
			var nullableNote sql.NullString
			if err := rows.Scan(&id, &name, &rating, &nullableNote); err != nil {
				log.Printf("Error scanning columns from database: %s", err)
				continue
			}
			var note string
			if nullableNote.Valid {
				note = nullableNote.String
			}
			res = append(res, Review{
				Id:     id,
				Name:   name,
				Rating: rating,
				Note: &Note{
					Review:   id,
					Username: username,
					Note:     note,
				},
			})
		}
	}
	return res, nil
}

func prepareTables(db *sql.DB) {
	stmt := `
		CREATE TABLE userinfo (
			username text not null primary key,
			password text not null);
		CREATE TABLE reviews (
			id int not null primary key,
			name text not null,
			rating int not null);
		CREATE TABLE notes (
			review int not null,
			username text not null,
			note text not null,
			PRIMARY KEY(review, username),
			FOREIGN KEY(review) REFERENCES reviews(id),
			FOREIGN KEY(username) REFERENCES userinfo(username));`
	_, err := db.Exec(stmt)
	if err != nil {
		log.Fatalf("Error creating tables: %s", err)
	}
}

func prepareUsers(db *sql.DB) {
	stmt, err := db.Prepare(`INSERT INTO userinfo(username, password) VALUES(?, ?);`)
	if err != nil {
		log.Fatalf("Error preparing: %s", err)
	}
	defer stmt.Close()
	for _, v := range preloadUsers {
		if _, err := stmt.Exec(v.Username, v.Password); err != nil {
			log.Fatal(err)
		}
	}
}

func prepareReviews(db *sql.DB) {
	stmt, err := db.Prepare(`INSERT INTO reviews(id, name, rating) VALUES(?, ?, ?);`)
	if err != nil {
		log.Fatalf("Error preparing: %s", err)
	}
	defer stmt.Close()
	for _, v := range preloadReviews {
		if _, err := stmt.Exec(v.Id, v.Name, v.Rating); err != nil {
			log.Fatal(err)
		}
	}
}

func prepareNotes(db *sql.DB) {
	stmt, err := db.Prepare(`INSERT INTO notes(review, username, note) VALUES(?, ?, ?);`)
	if err != nil {
		log.Fatalf("Error preparing: %s", err)
	}
	defer stmt.Close()
	for _, v := range preloadNotes {
		if _, err := stmt.Exec(v.Review, v.Username, v.Note); err != nil {
			log.Fatal(err)
		}
	}
}
