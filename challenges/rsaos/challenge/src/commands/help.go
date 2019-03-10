package commands

import (
	"fmt"
	"os"
	"sort"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

type HelpCommand struct{}

func (hc *HelpCommand) GetName() string {
	return "help"
}

func (hc *HelpCommand) GetDescription() string {
	return "Get information about all commands."
}

func (hc *HelpCommand) IsPrivileged() bool {
	return false
}

func (hc *HelpCommand) GetUsage() string {
	return `help [command]

	Get help, optionally about command.`
}

func (hc *HelpCommand) Dispatch(_ *sessions.Session, args []string) bool {
	if len(args) == 1 {
		if cmd, ok := commandRegistry[args[0]]; ok {
			Usage(cmd)
			return true
		}
	}

	prived := make([]string, 0)
	unprived := make([]string, 0)
	for k, v := range commandRegistry {
		if v.IsPrivileged() {
			prived = append(prived, k)
		} else {
			unprived = append(unprived, k)
		}
	}

	sort.Strings(prived)
	sort.Strings(unprived)

	fmt.Fprint(os.Stdout, "Unprivileged commands:\n")

	for _, c := range unprived {
		printCommandInfo(c)
	}

	fmt.Fprint(os.Stdout, "\nPrivileged commands:\n")

	for _, c := range prived {
		printCommandInfo(c)
	}

	return true
}

func printCommandInfo(name string) {
	fmt.Fprintf(os.Stdout, "    %-16s    %s\n", name, commandRegistry[name].GetDescription())
}

type SecurityCommand struct{}

func (hc *SecurityCommand) GetName() string {
	return "security"
}

func (hc *SecurityCommand) GetDescription() string {
	return "Get information about security."
}

func (hc *SecurityCommand) IsPrivileged() bool {
	return true
}

func (hc *SecurityCommand) GetUsage() string {
	return `security

	Authentication and authorization for operations is provided by RSA signing.
	Unpriviliged commands are automatically signed by the client using the IEEE
	CRC-32 of the full command line.
	
	Privileged commands must be manually signed by RSA over the FoldHash of the
	full command line.  FoldHash is a hash that was developed in response to the
	Shattered attack on SHA-1.  FoldHash provides 80 bits of security by use of
	an exclusive-or operation on the output of the standard SHA-1 hash split in
	two.
	
	All RSA signing operations are unpadded signatures of the hash value in
	big endian order.  This avoids attacks where only bad sources of entropy are
	available.`
}

func (hc *SecurityCommand) Dispatch(_ *sessions.Session, _ []string) bool {
	Usage(hc)
	return true
}

func init() {
	RegisterCommand(&HelpCommand{})
	RegisterCommand(&SecurityCommand{})
}
