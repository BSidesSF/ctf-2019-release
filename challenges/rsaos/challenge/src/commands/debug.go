package commands

import (
	"strings"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

type DebugCommand struct{}

func (dc *DebugCommand) GetName() string {
	return "debug"
}

func (dc *DebugCommand) GetDescription() string {
	return "Debug the OS."
}

func (dc *DebugCommand) GetUsage() string {
	return `debug <enable|disable>

	Enable/disable debugging.`
}

func (dc *DebugCommand) IsPrivileged() bool {
	return false
}

func (dc *DebugCommand) Dispatch(sess *sessions.Session, args []string) bool {
	if len(args) == 1 {
		opt := strings.ToLower(args[0])
		if opt == "enable" {
			sess.SetDebug(true)
			return true
		} else if opt == "disable" {
			sess.SetDebug(false)
			return true
		}
	}
	Usage(dc)
	return false
}

func init() {
	RegisterCommand(&DebugCommand{})
}
