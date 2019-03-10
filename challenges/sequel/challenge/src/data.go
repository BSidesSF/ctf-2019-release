package main

import (
	"encoding/base64"
	"encoding/json"
	"log"
)

const flag = "CTF{%did_you_LIKE_it%}"

type Review struct {
	Id     int
	Name   string
	Rating int
	Note   *Note
}

type Note struct {
	Review   int
	Username string
	Note     string
}

type Userinfo struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

func (r *Review) RatingSlice() []int {
	return make([]int, r.Rating, r.Rating)
}

func (u *Userinfo) MakeCookie() string {
	jdata, err := json.Marshal(u)
	if err != nil {
		return ""
	}
	return base64.URLEncoding.EncodeToString(jdata)
}

func UserinfoFromCookie(c string) *Userinfo {
	jdata, err := base64.URLEncoding.DecodeString(c)
	if err != nil {
		log.Printf("Error decoding cookie: %s", err)
		return nil
	}
	ui := &Userinfo{}
	if err := json.Unmarshal(jdata, ui); err != nil {
		log.Printf("Error unmarshaling cookie: %s", err)
		return nil
	}
	return ui
}

var preloadUsers = []Userinfo{
	{
		Username: "guest",
		Password: "guest",
	},
	{
		Username: "sequeladmin",
		Password: "f5ec3af19f0d3679e7d5a148f4ac323d",
	},
}

var preloadReviews = []Review{
	{
		Id:     1,
		Name:   "Hackers",
		Rating: 5,
	},
	{
		Id:     2,
		Name:   "War Games",
		Rating: 5,
	},
	{
		Id:     3,
		Name:   "Swordfish",
		Rating: 3,
	},
	{
		Id:     4,
		Name:   "Sneakers",
		Rating: 4,
	},
	{
		Id:     5,
		Name:   "The Net",
		Rating: 1,
	},
}

var preloadNotes = []Note{
	{
		Review:   1,
		Username: "sequeladmin",
		Note:     "My favorite hacking movie ever!  In fact, I like it so much that I'll give you a flag for liking it too: " + flag,
	},
	{
		Review:   1,
		Username: "guest",
		Note:     "Pwning Agent Richard Gill may have been the first Hacking CTF included in a movie.  Maybe the admin likes it too?",
	},
	{
		Review:   5,
		Username: "guest",
		Note:     "If you liked this movie, you should be ashamed of yourself.",
	},
}
