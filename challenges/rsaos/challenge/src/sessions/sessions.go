package sessions

import (
	"bufio"
	"os"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/rawrsa"
)

type Session struct {
	debug      bool
	privileged bool
	RSA        *rawrsa.RSA
	inp        *bufio.Scanner
}

func NewSession() *Session {
	return &Session{
		debug:      false,
		privileged: false,
		inp:        bufio.NewScanner(os.Stdin),
	}
}

func (s *Session) SetDebug(enabled bool) {
	s.debug = enabled
}

func (s *Session) DebugEnabled() bool {
	return s.debug
}

func (s *Session) SetPrivileged(privileged bool) {
	if s.privileged {
		return
	}
	s.privileged = privileged
}

func (s *Session) Privileged() bool {
	return s.privileged
}

func (s *Session) Readline() (string, bool) {
	ok := s.inp.Scan()
	if !ok {
		return "", ok
	}
	return s.inp.Text(), true
}

func (s *Session) SetKeyFile(fname string) bool {
	s.RSA = rawrsa.NewRSAFromPEMFile(fname)
	return s.RSA != nil
}
