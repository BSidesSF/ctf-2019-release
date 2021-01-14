package commands

import (
	"fmt"
	"hash/crc32"
	"math/big"
	"os"
	"strings"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/foldhash"
	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

type Command interface {
	GetName() string
	GetDescription() string
	GetUsage() string
	IsPrivileged() bool
	Dispatch(*sessions.Session, []string) bool
}

func RegisterCommand(c Command) {
	name := strings.ToLower(c.GetName())
	if _, ok := commandRegistry[name]; ok {
		panic(fmt.Sprintf("Duplicate command entry: %s", name))
	}
	commandRegistry[name] = c
}

func ParseAndExecute(sess *sessions.Session, cmdStr string) bool {
	// Currently no quoting is supported
	cmdStr = strings.TrimSuffix(cmdStr, "\n")
	comsplit := strings.Split(cmdStr, "#")
	words := strings.Split(comsplit[0], " ")
	name, args := words[0], words[1:]
	lname := strings.ToLower(name)
	if cmd, ok := commandRegistry[lname]; ok {
		// validate here
		if sess.Privileged() || cmd.IsPrivileged() {
			if !validatePrivileged(sess, cmdStr) {
				fmt.Fprintf(os.Stderr, "Privileged command validation failed!\n")
				return false
			}
		} else {
			if !validate(sess, cmdStr) {
				fmt.Fprintf(os.Stderr, "Command validation failed!\n")
				return false
			}
		}
		return cmd.Dispatch(sess, args)
	}
	// Command not found error
	fmt.Fprintf(os.Stderr, "Command not found: %s\n", name)
	return false
}

func Usage(c Command) {
	fmt.Fprintln(os.Stdout, c.GetUsage())
}

func validatePrivileged(sess *sessions.Session, cmd string) bool {
	if sess.RSA == nil {
		return false
	}
	fmt.Fprintf(os.Stdout, "RSA(FoldHash) sig: ")
	if sigStr, ok := sess.Readline(); ok {
		if sig, ok := new(big.Int).SetString(sigStr, 0); ok {
			return sess.RSA.HashAndVerify([]byte(cmd), foldhash.New(), sig)
		}
	}
	return false
}

func validate(sess *sessions.Session, cmd string) bool {
	if sess.RSA == nil {
		return false
	}
	h := crc32.NewIEEE()
	h.Write([]byte(cmd))
	crc := new(big.Int).SetBytes(h.Sum(nil))
	sig := sess.RSA.Sign(crc)
	if sess.DebugEnabled() {
		fmt.Fprintf(os.Stderr, "DBG: CRC-32(0x%08x) SIG(0x%x)\n", crc, sig)
	}
	return true
}

var commandRegistry = make(map[string]Command)
