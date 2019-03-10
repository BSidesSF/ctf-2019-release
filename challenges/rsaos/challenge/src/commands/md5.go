package commands

import (
	"crypto/md5"
	"fmt"
	"os"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/foldhash"
	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

type MD5Command struct{}

func (mc *MD5Command) GetName() string {
	return "md5"
}

func (mc *MD5Command) GetDescription() string {
	return "Get md5 of first argument"
}

func (mc *MD5Command) GetUsage() string {
	return `md5 <arg>

	MD5 of arg`
}

func (mc *MD5Command) IsPrivileged() bool {
	return false
}

func (mc *MD5Command) Dispatch(_ *sessions.Session, args []string) bool {
	if len(args) < 1 {
		Usage(mc)
		return false
	}
	h := md5.New()
	h.Write([]byte(args[0]))
	fmt.Fprintf(os.Stdout, "md5(%s): 0x%0x\n", args[0], h.Sum(nil))
	return true
}

type FoldHashCommand struct{}

func (mc *FoldHashCommand) GetName() string {
	return "foldhash"
}

func (mc *FoldHashCommand) GetDescription() string {
	return "Get foldhash of first argument"
}

func (mc *FoldHashCommand) GetUsage() string {
	return `foldhash <arg>

	FoldHash of arg`
}

func (mc *FoldHashCommand) IsPrivileged() bool {
	return false
}

func (mc *FoldHashCommand) Dispatch(_ *sessions.Session, args []string) bool {
	if len(args) < 1 {
		Usage(mc)
		return false
	}
	h := foldhash.New()
	h.Write([]byte(args[0]))
	fmt.Fprintf(os.Stdout, "foldhash(%s): 0x%0x\n", args[0], h.Sum(nil))
	return true
}

func init() {
	RegisterCommand(&MD5Command{})
	RegisterCommand(&FoldHashCommand{})
}
