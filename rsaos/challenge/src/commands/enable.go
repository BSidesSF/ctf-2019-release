package commands

import (
	"strings"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

type EnableCommand struct{}

func (ec *EnableCommand) GetName() string {
	return "enable"
}

func (ec *EnableCommand) GetDescription() string {
	return "Enable all commands as privileged."
}

func (ec *EnableCommand) GetUsage() string {
	return `enable <enable|disable>

	Enable/disable all commands as privileged.
	Once enabled, all commands will require privileged authentication.`
}

func (ec *EnableCommand) IsPrivileged() bool {
	return false
}

func (ec *EnableCommand) Dispatch(sess *sessions.Session, args []string) bool {
	if len(args) == 1 {
		opt := strings.ToLower(args[0])
		if opt == "enable" {
			sess.SetPrivileged(true)
			return true
		} else if opt == "disable" {
			sess.SetPrivileged(false)
			return true
		}
	}
	Usage(ec)
	return false
}

func init() {
	RegisterCommand(&EnableCommand{})
}
