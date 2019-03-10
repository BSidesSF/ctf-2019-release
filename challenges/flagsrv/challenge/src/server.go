package main

import (
	"crypto/hmac"
	"crypto/sha256"
	"fmt"
	"html/template"
	"log"
	"net/http"
	"regexp"
	"sort"
	"strings"
)

const POW_BITS = 12
const POW_FIELD = "__pow__"

var indexTemplate *template.Template
var enrollTemplate *template.Template
var flagTemplate *template.Template
var cookieName = "1337_AUTH"

type LoginPageData struct {
	LoggedIn bool
	Register bool
	Error    string
	MFA      bool
	Username string
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
		errs := LoginPageData{Error: msg}
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
	if !verifyProofOfWork(w, r, POW_FIELD, []string{"username", "password"}) {
		return
	}
	valid := verifyUsernamePassword(username, password)
	if valid == LoginFailed {
		renderLoginError("Username/password mismatch.")
		return
	}

	// Set cookie
	session := &Session{
		Username:    username,
		LoginPassed: true,
	}
	var dest string
	if valid == LoginNeedsMFA {
		session.MFAState = Needed
		dest = "/2fa"
	} else {
		session.MFAState = NotNeeded
		dest = "/flag"
	}
	saveSessionCookie(w, session)
	http.Redirect(w, r, dest, http.StatusFound)
}

func saveSessionCookie(w http.ResponseWriter, s *Session) {
	cookie := &http.Cookie{
		Name:     cookieName,
		Value:    s.ToString(),
		HttpOnly: true,
	}
	http.SetCookie(w, cookie)
}

func getSession(w http.ResponseWriter, r *http.Request) *Session {
	failed := func() *Session {
		log.Printf("No session for user when required.")
		http.Error(w, "Forbidden", http.StatusForbidden)
		return nil
	}
	if val, err := r.Cookie(cookieName); err != nil {
		if err == http.ErrNoCookie {
			return failed()
		}
		log.Printf("Unexpected error getting cookie: %s", err)
		return failed()
	} else {
		s, err := SessionFromCookie(val.Value)
		if err != nil {
			log.Printf("Error getting session: %s", err)
			return failed()
		}
		return s
	}
}

func serveRegister(w http.ResponseWriter, r *http.Request) {
	if _, err := r.Cookie(cookieName); err != http.ErrNoCookie {
		http.Error(w, "Forbidden", http.StatusForbidden)
		return
	}
	renderRegister := func(estr string) {
		tmpData := LoginPageData{Register: true}
		if estr != "" {
			tmpData.Error = estr
		}
		if err := indexTemplate.ExecuteTemplate(w, "base", tmpData); err != nil {
			log.Printf("Error rendering register: %s", err)
			http.Error(w, "Server Error", http.StatusInternalServerError)
		}
	}
	if r.Method != "POST" {
		renderRegister("")
		return
	}
	username := r.FormValue("username")
	password := r.FormValue("password")
	if username == "" || password == "" || username == "flag" {
		renderRegister("Must specify a username/password")
		return
	}
	if m, err := regexp.MatchString(`^[a-zA-Z0-9]+$`, username); err != nil {
		log.Printf("Error regexp: %s", err)
		http.Error(w, "Server Error", http.StatusInternalServerError)
		return
	} else {
		if !m {
			renderRegister("Username must be alphanumeric.")
			return
		}
	}
	if !verifyProofOfWork(w, r, POW_FIELD, []string{"username", "password"}) {
		return
	}
	if err := dbRegister(username, password); err != nil {
		renderRegister(fmt.Sprintf("Error registering: %s", err))
		return
	}
	session := &Session{
		Username:    username,
		LoginPassed: true,
		MFAState:    NotNeeded,
	}
	saveSessionCookie(w, session)
	http.Redirect(w, r, "/flag", http.StatusFound)
}

func serve2FA(w http.ResponseWriter, r *http.Request) {
	session := getSession(w, r)
	if session == nil {
		return
	}
	continueAndRedir := func() {
		saveSessionCookie(w, session)
		http.Redirect(w, r, "/flag", http.StatusFound)
	}
	if session.AuthnDone() {
		continueAndRedir()
		return
	}
	if !session.LoginPassed {
		http.Error(w, "Forbidden", http.StatusForbidden)
		return
	}

	renderMFA := func(estr string) {
		tmpData := LoginPageData{MFA: true, Username: session.Username}
		if estr != "" {
			tmpData.Error = estr
		}
		if err := indexTemplate.ExecuteTemplate(w, "base", tmpData); err != nil {
			log.Printf("Error rendering 2FA: %s", err)
			http.Error(w, "Server Error", http.StatusInternalServerError)
		}
	}

	if r.Method != "POST" {
		renderMFA("")
		return
	}

	username := r.FormValue("username")
	if session.Username != username {
		log.Printf("Expected username %s, got %s", session.Username, username)
		session.Username = username
	}
	passcode := r.FormValue("passcode")
	if passcode == "" {
		renderMFA("Passcode is required.")
		return
	}
	if !verifyProofOfWork(w, r, POW_FIELD, []string{"username", "passcode"}) {
		return
	}
	if !validateTOTP(username, passcode) {
		renderMFA("Invalid OTP.")
		return
	}
	session.MFAState = Fulfilled
	continueAndRedir()
}

func serveSetup(w http.ResponseWriter, r *http.Request) {
	session := getSession(w, r)
	if session == nil {
		return
	}
	if !session.AuthnDone() {
		http.Redirect(w, r, "/", http.StatusFound)
		return
	}

	var message string
	if r.Method == "POST" {
		message = "2FA Enrolled"
		if err := dbEnroll2FA(session.Username); err != nil {
			log.Printf("Error enrolling: %s", err)
			message = "2FA Enrollment Failed!"
		} else {
			session.MFAState = Fulfilled
			saveSessionCookie(w, session)
		}
	}
	tmpData := struct {
		LoggedIn bool
		Username string
		Enrolled bool
		Message  string
	}{
		LoggedIn: true,
		Username: session.Username,
		Enrolled: session.MFAState == Fulfilled,
		Message:  message,
	}
	if err := enrollTemplate.ExecuteTemplate(w, "base", tmpData); err != nil {
		log.Printf("Error rendering enrollment: %s", err)
		http.Error(w, "Server Error", http.StatusInternalServerError)
	}
}

func serveFlag(w http.ResponseWriter, r *http.Request) {
	session := getSession(w, r)
	if session == nil {
		return
	}
	if !session.AuthnDone() {
		http.Redirect(w, r, "/", http.StatusFound)
		return
	}

	fd := getFlagDataForUser(session.Username)
	if fd == nil {
		log.Printf("Error getting flag data.")
		http.Error(w, "Not Found", http.StatusNotFound)
		return
	}

	if r.Method == "POST" {
		if fd.Readonly {
			log.Printf("Update attempt for readonly!")
			http.Error(w, "Bad Request", http.StatusBadRequest)
			return
		}
		fd.Flag = r.FormValue("flag")
		fd.Save()
	}

	tmpData := struct {
		LoggedIn bool
		Username string
		Flag     string
		Readonly bool
	}{
		LoggedIn: true,
		Username: session.Username,
		Flag:     fd.Flag,
		Readonly: fd.Readonly,
	}

	if err := flagTemplate.ExecuteTemplate(w, "base", tmpData); err != nil {
		log.Printf("Error rendering flag page: %s", err)
		http.Error(w, "Server Error", http.StatusInternalServerError)
	}
}

func serveLogout(w http.ResponseWriter, r *http.Request) {
	cookie := &http.Cookie{
		Name:     cookieName,
		Value:    "",
		HttpOnly: true,
	}
	http.SetCookie(w, cookie)
	http.Redirect(w, r, "/", http.StatusFound)
}

func verifyProofOfWork(w http.ResponseWriter, r *http.Request, powField string, coveredFields []string) bool {
	sort.Strings(coveredFields)
	fieldValues := make([]string, 0, len(coveredFields))
	for _, k := range coveredFields {
		fieldValues = append(fieldValues, r.FormValue(k))
	}
	provenString := strings.Join(fieldValues, ";")
	mac := hmac.New(sha256.New, []byte(r.FormValue(powField)))
	mac.Write([]byte(provenString))
	sum := mac.Sum(nil)
	if countLeadingZeroBits(sum) < POW_BITS {
		log.Printf("Proof of work failed!")
		http.Error(w, "Forbidden", http.StatusForbidden)
		return false
	}
	return true
}

func countLeadingZeroBits(v []byte) int {
	count := 0
	for _, b := range v {
		if b == byte(0) {
			count += 8
			continue
		}
		for i := uint32(7); i >= 0; i-- {
			if (b & byte(1<<i)) != 0 {
				break
			}
			count++
		}
		break
	}
	return count
}

func loadTemplates() {
	indexTemplate = template.Must(template.ParseFiles(
		"templates/base.html", "templates/index.html"))
	enrollTemplate = template.Must(template.ParseFiles(
		"templates/base.html", "templates/enroll.html"))
	flagTemplate = template.Must(template.ParseFiles(
		"templates/base.html", "templates/flag.html"))
}

func main() {
	// Load templates
	loadTemplates()

	// Setup handlers
	fs := http.FileServer(http.Dir("./static/"))
	http.Handle("/static/", http.StripPrefix("/static/", fs))

	http.HandleFunc("/", serveIndex)
	http.HandleFunc("/login", serveLogin)
	http.HandleFunc("/logout", serveLogout)
	http.HandleFunc("/register", serveRegister)
	http.HandleFunc("/2fa", serve2FA)
	http.HandleFunc("/setup2fa", serveSetup)
	http.HandleFunc("/flag", serveFlag)

	// Prepare to serve
	port := ":8081"
	log.Printf("Starting serving on %s", port)
	log.Fatal(http.ListenAndServe(port, nil))
}
