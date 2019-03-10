package main

import (
	"crypto/hmac"
	"crypto/sha256"
	"encoding/base32"
	"encoding/base64"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"github.com/pquerna/otp/totp"
	"log"
	"strings"
	"time"
)

type TwoFactorState int

var hmacKey = []byte("too many secrets")

const (
	Needed TwoFactorState = iota
	Fulfilled
	NotNeeded
)

type Session struct {
	Username    string
	LoginPassed bool
	MFAState    TwoFactorState
}

func (s *Session) AuthnDone() bool {
	if !s.LoginPassed {
		return false
	}
	return s.MFAState == Fulfilled || s.MFAState == NotNeeded
}

func (s *Session) ToString() string {
	buf, err := json.Marshal(s)
	if err != nil {
		log.Printf("Error marshaling json: %s", err)
		return ""
	}
	mac := hmac.New(sha256.New, hmacKey)
	mac.Write(buf)
	hexMac := hex.EncodeToString(mac.Sum(nil))
	msg := hexMac + ":" + string(buf)
	return base64.URLEncoding.EncodeToString([]byte(msg))
}

func SessionFromCookie(c string) (*Session, error) {
	buf, err := base64.URLEncoding.DecodeString(c)
	if err != nil {
		log.Printf("Error unmarshaling b64: %s", err)
		return nil, fmt.Errorf("Error decoding!")
	}
	pieces := strings.SplitN(string(buf), ":", 2)
	if len(pieces) != 2 {
		log.Printf("Error splitting cookie.")
		return nil, fmt.Errorf("Error decoding!")
	}
	sig, err := hex.DecodeString(pieces[0])
	if err != nil {
		log.Printf("Error decoding hex: %s", err)
		return nil, fmt.Errorf("Error decoding!")
	}
	mac := hmac.New(sha256.New, hmacKey)
	data := []byte(pieces[1])
	mac.Write(data)
	expected := mac.Sum(nil)
	if !hmac.Equal(expected, sig) {
		log.Printf("Signature mismatch: %x vs %x", expected, sig)
		return nil, fmt.Errorf("Error decoding!")
	}
	newSession := &Session{}
	if err := json.Unmarshal(data, newSession); err != nil {
		log.Printf("Error decoding JSON: %s", err)
		return nil, fmt.Errorf("Error decoding!")
	}
	return newSession, nil
}

func validateTOTP(username, passcode string) bool {
	mac := hmac.New(sha256.New, []byte(username))
	mac.Write([]byte(username))
	b32Mac := base32.StdEncoding.WithPadding(base32.NoPadding).EncodeToString(mac.Sum(nil))
	if !totp.Validate(passcode, b32Mac) {
		gen, err := totp.GenerateCode(b32Mac, time.Now().UTC())
		if err != nil {
			log.Printf("Error generating OTP: %s", err)
		} else {
			log.Printf("Expected OTP %s, received %s.", gen, passcode)
		}
		return false
	}
	return true
}
