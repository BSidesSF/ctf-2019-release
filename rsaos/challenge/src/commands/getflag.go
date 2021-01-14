package commands

import (
	"fmt"
	"io/ioutil"
	"os"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

type GetFlagCommand struct {
	flag string
}

func (ec *GetFlagCommand) GetName() string {
	return "get-flag"
}

func (ec *GetFlagCommand) GetDescription() string {
	return "Get the flag."
}

func (ec *GetFlagCommand) GetUsage() string {
	return `getflag

	Get the flag.`
}

func (ec *GetFlagCommand) IsPrivileged() bool {
	return true
}

func (ec *GetFlagCommand) Dispatch(_ *sessions.Session, _ []string) bool {
	fmt.Fprintf(os.Stdout, "The flag is: %s\n", ec.flag)
	return true
}

func init() {
	gfc := &GetFlagCommand{}
	if buf, err := ioutil.ReadFile("./flag.txt"); err == nil {
		gfc.flag = string(buf)
		RegisterCommand(gfc)
		return
	}
	if buf, err := ioutil.ReadFile("/home/rsaos/flag.txt"); err == nil {
		gfc.flag = string(buf)
		RegisterCommand(gfc)
		return
	}
}
