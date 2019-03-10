package main

import (
	"html/template"
	"log"
	"net/http"
	"regexp"
)

var indexTemplate *template.Template
var sequelTemplate *template.Template
var cookieName = "1337_AUTH"

type errorStruct struct {
	Error string
}

func serveStaticFile(fname string) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, fname)
	}
}

func serveIndex(w http.ResponseWriter, r *http.Request) {
	if r.URL.Path != "/" {
		http.NotFoundHandler().ServeHTTP(w, r)
		return
	}
	if err := indexTemplate.ExecuteTemplate(w, "base", nil); err != nil {
		log.Printf("Error rendering index: %s", err)
		http.Error(w, "Server Error", http.StatusInternalServerError)
	}
}

func serveLogin(w http.ResponseWriter, r *http.Request) {
	username := r.FormValue("username")
	password := r.FormValue("password")
	renderLoginError := func(msg string) {
		errs := errorStruct{msg}
		if err := indexTemplate.ExecuteTemplate(w, "base", errs); err != nil {
			log.Printf("Error rendering index: %s", err)
			http.Error(w, "Server Error", http.StatusInternalServerError)
		}
	}

	// Validate username
	if m, err := regexp.MatchString(`^[a-zA-Z0-9]+$`, username); err != nil {
		log.Printf("Error regexp: %s", err)
		http.Error(w, "Server Error", http.StatusInternalServerError)
		return
	} else {
		if !m {
			renderLoginError("Username must be alphanumeric.")
			return
		}
	}
	validUser, validPass := VerifyUsernamePassword(username, password)
	if !validUser {
		renderLoginError("Unknown user.")
		return
	}
	if !validPass {
		renderLoginError("Username/password mismatch.")
		return
	}

	// Set cookie
	uinfo := &Userinfo{
		Username: username,
		Password: password,
	}
	cookie := &http.Cookie{
		Name:     cookieName,
		Value:    uinfo.MakeCookie(),
		HttpOnly: true,
	}
	http.SetCookie(w, cookie)
	http.Redirect(w, r, "/sequels", http.StatusFound)
}

func serveSequels(w http.ResponseWriter, r *http.Request) {
	// Verify cookie
	var username string
	if cookie, err := r.Cookie(cookieName); err != nil {
		if err == http.ErrNoCookie {
			http.Redirect(w, r, "/", http.StatusFound)
			return
		}
		log.Printf("Error getting cookie: %s", err)
		http.Error(w, "Server Error", http.StatusInternalServerError)
		return
	} else {
		uinfo := UserinfoFromCookie(cookie.Value)
		if uinfo == nil {
			http.Error(w, "Server Error", http.StatusInternalServerError)
			return
		}
		validUser, validPass := VerifyUserinfo(uinfo)
		if !validUser {
			http.Error(w, "Invalid user.", http.StatusForbidden)
			return
		}
		if !validPass {
			http.Error(w, "Invalid user/pass.", http.StatusForbidden)
			return
		}
		username = uinfo.Username
	}

	// Retrieve the data
	reviews, err := GetReviewsForUser(username)
	if err != nil {
		http.Error(w, "Server Error", http.StatusInternalServerError)
		return
	}

	args := struct {
		Reviews  []Review
		Username string
	}{
		Reviews:  reviews,
		Username: username,
	}

	// Serve the page
	if err := sequelTemplate.ExecuteTemplate(w, "base", args); err != nil {
		log.Printf("Error rendering template: %s", err)
		http.Error(w, "Server Error", http.StatusInternalServerError)
		return
	}
}

func loadTemplates() {
	indexTemplate = template.Must(template.ParseFiles(
		"templates/base.html", "templates/index.html"))
	sequelTemplate = template.Must(template.ParseFiles(
		"templates/base.html", "templates/sequel.html"))
}

func main() {
	// Load templates
	loadTemplates()

	// Setup handlers
	fs := http.FileServer(http.Dir("./static/"))
	http.Handle("/static/", http.StripPrefix("/static/", fs))

	http.HandleFunc("/", serveIndex)
	http.HandleFunc("/login", serveLogin)
	http.HandleFunc("/sequels", serveSequels)

	// Prepare to serve
	PrepareDB()
	port := ":8081"
	log.Printf("Starting serving on %s", port)
	log.Fatal(http.ListenAndServe(port, nil))
}
