package main

import (
	"fmt"
	"os"
	"path"
	"syscall"

	"github.com/BSidesSF/ctf-2019/challenges/rsaos/commands"
	"github.com/BSidesSF/ctf-2019/challenges/rsaos/sessions"
)

var banner = `
      ___           ___           ___           ___           ___     
     /\  \         /\  \         /\  \         /\  \         /\  \    
    /::\  \       /::\  \       /::\  \       /::\  \       /::\  \   
   /:/\:\  \     /:/\ \  \     /:/\:\  \     /:/\:\  \     /:/\ \  \  
  /::\~\:\  \   _\:\~\ \  \   /::\~\:\  \   /:/  \:\  \   _\:\~\ \  \ 
 /:/\:\ \:\__\ /\ \:\ \ \__\ /:/\:\ \:\__\ /:/__/ \:\__\ /\ \:\ \ \__\
 \/_|::\/:/  / \:\ \:\ \/__/ \/__\:\/:/  / \:\  \ /:/  / \:\ \:\ \/__/
    |:|::/  /   \:\ \:\__\        \::/  /   \:\  /:/  /   \:\ \:\__\  
    |:|\/__/     \:\/:/  /        /:/  /     \:\/:/  /     \:\/:/  /  
    |:|  |        \::/  /        /:/  /       \::/  /       \::/  /   
     \|__|         \/__/         \/__/         \/__/         \/__/    

`

var rsaKeyFile = "./rsaos.key"

func main() {
	startup()
	sess := sessions.NewSession()
	sess.SetKeyFile(rsaKeyFile)

	for {
		fmt.Fprint(os.Stdout, "\n> ")
		cmd, ok := sess.Readline()
		if !ok {
			break
		}
		commands.ParseAndExecute(sess, cmd)
	}
}

func startup() {
	fmt.Fprint(os.Stdout, banner)
	changeBinDir()
	limitTime(5)
}

// Change to working directory
func changeBinDir() {
	// read /proc/self/exe
	if dest, err := os.Readlink("/proc/self/exe"); err != nil {
		fmt.Fprintf(os.Stderr, "Error reading link: %s\n", err)
		return
	} else {
		dest = path.Dir(dest)
		if err := os.Chdir(dest); err != nil {
			fmt.Fprintf(os.Stderr, "Error changing directory: %s\n", err)
		}
	}
}

// Limit CPU time to certain number of seconds
func limitTime(secs int) {
	lims := &syscall.Rlimit{
		Cur: uint64(secs),
		Max: uint64(secs),
	}
	if err := syscall.Setrlimit(syscall.RLIMIT_CPU, lims); err != nil {
		if inner_err := syscall.Getrlimit(syscall.RLIMIT_CPU, lims); inner_err != nil {
			fmt.Fprintf(os.Stderr, "Error getting limits: %s\n", inner_err)
		} else {
			if lims.Cur > 0 {
				// A limit was set elsewhere, we'll live with it
				return
			}
		}
		fmt.Fprintf(os.Stderr, "Error setting limits: %s", err)
		os.Exit(-1)
	}
}
