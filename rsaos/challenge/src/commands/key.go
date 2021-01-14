package commands

import (
	"fmt"
	"os"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

type PublicKeyCommand struct{}

func (ec *PublicKeyCommand) GetName() string {
	return "get-publickey"
}

func (ec *PublicKeyCommand) GetDescription() string {
	return "Retrieve the Public Key"
}

func (ec *PublicKeyCommand) GetUsage() string {
	return `get-publickey

	Get the Public Key for signed requests.`
}

func (ec *PublicKeyCommand) IsPrivileged() bool {
	return false
}

func (ec *PublicKeyCommand) Dispatch(sess *sessions.Session, _ []string) bool {
	fmt.Fprintf(os.Stdout, "Public key parameters:\n%s", sess.RSA.PublicString())
	return true
}

type PrivateKeyCommand struct{}

func (ec *PrivateKeyCommand) GetName() string {
	return "get-privatekey"
}

func (ec *PrivateKeyCommand) GetDescription() string {
	return "Retrieve the Private Key"
}

func (ec *PrivateKeyCommand) GetUsage() string {
	return `get-privatekey

	Get the Private Key for signed requests.`
}

func (ec *PrivateKeyCommand) IsPrivileged() bool {
	return true
}

func (ec *PrivateKeyCommand) Dispatch(sess *sessions.Session, _ []string) bool {
	fmt.Fprintf(os.Stdout, "Private key parameters:\n%s", sess.RSA.PrivateString())
	return true
}

func init() {
	RegisterCommand(&PublicKeyCommand{})
	RegisterCommand(&PrivateKeyCommand{})
}
