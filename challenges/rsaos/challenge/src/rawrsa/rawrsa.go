package rawrsa

import (
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"hash"
	"io/ioutil"
	"log"
	"math/big"
	"os"
)

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

func (r *RSA) Sign(msg *big.Int) *big.Int {
	return new(big.Int).Exp(msg, r.d, r.n)
}

func (r *RSA) HashAndSign(msg []byte, hasher hash.Hash) *big.Int {
	hasher.Reset()
	hasher.Write(msg)
	z := new(big.Int).SetBytes(hasher.Sum(nil))
	return r.Sign(z)
}

func (r *RSA) Verify(msg, sig *big.Int) bool {
	return r.Sign(msg).Cmp(sig) == 0
}

func (r *RSA) HashAndVerify(msg []byte, hasher hash.Hash, sig *big.Int) bool {
	return r.HashAndSign(msg, hasher).Cmp(sig) == 0
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
