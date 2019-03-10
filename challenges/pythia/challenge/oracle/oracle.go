package main

import (
        "crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"io/ioutil"
	"math/big"
	"os"
	"syscall"
	"regexp"
	"bufio"
	"path"
	"log"
)


var (
	msg_num = regexp.MustCompile(`^\d+$`)
)

// RSA code just straight up stolen from matir

type RSA struct {
	// public elements
	n *big.Int
	e int
	// private elements, only set for private keys
	p *big.Int
	q *big.Int
	d *big.Int
}

func NewRSAFromPEMFile(f string) *RSA {
	r := &RSA{}
	pk := loadPEMPrivateKeyFile(f)
	if pk == nil {
		return nil
	}
	r.extractPrivateComponents(pk)
	return r
}

func (r *RSA) Decrypt(msg *big.Int) *big.Int {
	return new(big.Int).Exp(msg, r.d, r.n)
}

// Just used for debugging
func (r *RSA) Encrypt(msg *big.Int) *big.Int {
	return new(big.Int).Exp(msg, big.NewInt(int64(r.e)), r.n)
}

func (r *RSA) PublicString() string {
	return fmt.Sprintf("N: 0x%0x\nE: 0x%0x\n", r.n, r.e)
}

func (r *RSA) PrivateString() string {
	return fmt.Sprintf("N: 0x%0x\nE: 0x%0x\nP: 0x%0x\nQ: 0x%0x\nD: 0x%0x\n",
		r.n, r.e, r.p, r.q, r.d)
}

func loadPEMPrivateKeyFile(f string) *rsa.PrivateKey {
	if fp, err := os.Open(f); err != nil {
		log.Printf("Error opening %s: %s", f, err)
		return nil
	} else {
		defer fp.Close()
		if buf, err := ioutil.ReadAll(fp); err != nil {
			log.Printf("Error reading %s: %s", f, err)
			return nil
		} else {
			return loadPEMPrivateKey(buf)
		}
	}
}

func loadPEMPrivateKey(buf []byte) *rsa.PrivateKey {
	block, err := pem.Decode(buf)
	if block == nil {
		log.Printf("Error decoding: %s", err)
		return nil
	}
	if block.Type != "RSA PRIVATE KEY" {
		log.Printf("Expected RSA PRIVATE KEY")
		return nil
	}
	if pk, err := x509.ParsePKCS1PrivateKey(block.Bytes); err != nil {
		log.Printf("Invalid data: %s", err)
		return nil
	} else {
		return pk
	}
}

func (r *RSA) extractPrivateComponents(pk *rsa.PrivateKey) {
	if len(pk.Primes) != 2 {
		panic("Expecting 2 primes.")
	}
	r.p = pk.Primes[0]
	r.q = pk.Primes[1]
	r.d = pk.D
	r.n = pk.PublicKey.N
	r.e = pk.PublicKey.E
}

func CheckPadding(m *big.Int) (string, bool) {

	// assume 4096 bit RSA
	// SHIT WILL BREAK IF YOU GIVE THIS ORACLE A SMALLER
	// OR BIGGER KEY!
	var bin [512]byte

	binslice := m.Bytes();
	copy(bin[(512 - len(binslice)):], binslice)

	//fmt.Printf("%x\n", bin[:])

	if bin[0] != 0 {
		return "First byte of padding is not 0x00", false
	}

	if bin[1] != 2 {
		return "Second byte of padding is not 0x02", false
	}

	randc := 0
	gotz := false
	for _, n := range bin[2:] {
		if n == 0 {
			gotz = true
			break
		} else {
			randc++
		}
	}

	if gotz == false {
		return "Failed to find another 0x00 byte to end padding", false
	}

	if randc < 8 {
		return "Did not get at least 8 random bytes in the padding string", false
	}

	return "", true
}


func main() {

	changeBinDir() // Counteract xinetd starting us in /
	limitTime(20) // Hopefully this is enough for ~4096 rsa calls?
	privkey := NewRSAFromPEMFile("secret_rsa.pem")

	//fmt.Print(privkey.PrivateString())

	msg_alice := "97194177896481512304064978309034321461025762756340017496111640975739830387385854882431480452436216956112809618668391794272302311497635805786268172820031327225122010041312335517351045828850200993395482567710274913092057731669380489916396283293318228422256704037627567943783641513830629774336056301920074020697179040836341125814095980221020668841947541974435280377948243726594711261745962802315343568928603740498708815830723776086056513943001041572944735489926813532376139870917206126334869880031295721104283287168686682125992584876950045554919986570707604807900459438257402999041174707594620245307807930156403957174889931962686759231931506277832088799251129710327680717001145498705864571383522600721659848737468866428568381038511710756191210141897519794881377993736985184496980654819932766773514928510404625439911634183914813923991259611957603388470573730102401827322156847433704797664184138440633130787499540974692066921362528006650235445635321441585218074614701821247272539415808199775303179663596055858617317087220166029719010368749476350997166651243451665453758986881333031169620832886562797896606513051066018706275321760339599845475544771768973992880336083282390617731529988148983719830408435519637355303934370943489179469129412"


	inp := bufio.NewScanner(os.Stdin)

	// Get this baby started
	fmt.Fprint(os.Stdout, banner)

	fmt.Fprint(os.Stdout, "== Server Parameters ==\n")
	fmt.Fprint(os.Stdout, fmt.Sprintf("n: %d\n", privkey.n))
	fmt.Fprint(os.Stdout, fmt.Sprintf("e: %d\n", privkey.e))
	fmt.Fprint(os.Stdout, "\n")
	fmt.Fprint(os.Stdout, "== Your Messages ==\n")
	fmt.Fprint(os.Stdout, fmt.Sprintf("You have a message from Alice: %s\n", msg_alice))
	fmt.Fprint(os.Stdout, "\n")

	for {
		fmt.Fprint(os.Stdout, "\nmsg> ")
		ok := inp.Scan()
		if !ok {
			break
		}

		msg := inp.Text()

		if !msg_num.Match([]byte(msg)) {
			fmt.Fprint(os.Stdout, "Message must be an integer")
			continue
		}

		c, _ := new(big.Int).SetString(msg, 10)

		if c.Cmp(privkey.n) > 0 {
			fmt.Fprint(os.Stdout, "Message must be smaller than the modulus")
			continue
		}

		m := privkey.Decrypt(c)

		errstr, err := CheckPadding(m)

		if err == false {
			fmt.Fprint(os.Stdout, fmt.Sprintf("Message decryption failed padding checks: %s\n", errstr))
		} else {
			fmt.Fprint(os.Stdout, "Message accepted.\n")
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



var banner = `
 (        )             )  (
 )\ )  ( /(   \   )  ( /(  )\ )    (
(()/(  )\())/ )  /(  )\())(()/(    )\
 /(_))((_)\  ( )(_))((_)\  /(_))((((_)(
(_)) __ ((_)(_(_())  _((_)(_))   )\ _ )\
| _ \\ \ / /|_   _| | || ||_ _|  (_)_\(_)
|  _/ \ V /   | |   | __ | | |    / _ \
|_|    |_|    |_|   |_||_||___|  /_/ \_\

    SECURE MESSAGE SUBMISSION SERVICE

`
