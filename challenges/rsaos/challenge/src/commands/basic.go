package commands

import (
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

type ExitCommand struct{}

func (ec *ExitCommand) GetName() string {
	return "exit"
}

func (ec *ExitCommand) GetDescription() string {
	return "Exit the OS."
}

func (ec *ExitCommand) GetUsage() string {
	return `exit

	Exit RSA-OS`
}

func (ec *ExitCommand) IsPrivileged() bool {
	return false
}

func (ec *ExitCommand) Dispatch(_ *sessions.Session, _ []string) bool {
	os.Exit(0)
	return true
}

type EchoCommand struct{}

func (ec *EchoCommand) GetName() string {
	return "echo"
}

func (ec *EchoCommand) GetDescription() string {
	return "Echo the arguments."
}

func (ec *EchoCommand) GetUsage() string {
	return `echo [args] ...

	Echo arguments`
}

func (ec *EchoCommand) IsPrivileged() bool {
	return false
}

func (ec *EchoCommand) Dispatch(_ *sessions.Session, args []string) bool {
	fmt.Fprintf(os.Stdout, "%s\n", strings.Join(args, " "))
	return true
}

type DateCommand struct{}

func (dc *DateCommand) GetName() string {
	return "date"
}

func (dc *DateCommand) GetDescription() string {
	return "Get the date"
}

func (dc *DateCommand) GetUsage() string {
	return `date

	Get the date.`
}

func (dc *DateCommand) IsPrivileged() bool {
	return false
}

func (dc *DateCommand) Dispatch(_ *sessions.Session, _ []string) bool {
	fmt.Fprintf(os.Stdout, "%s\n",
		time.Now().Format("Mon Jan 2 15:04:05 -0700 MST 2006"))
	return true
}

func init() {
	RegisterCommand(&ExitCommand{})
	RegisterCommand(&EchoCommand{})
	RegisterCommand(&DateCommand{})
}
