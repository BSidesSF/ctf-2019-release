package main

import (
	"fmt"
	"hash/crc32"
	"math/big"
)

var target *big.Int
var targetStr = "0x21f86103118fadeb7441"
var alphabet = []byte(" !@#$%^&*()_+,./<>?;':[]{}\\|\"-=")
var prefix = []byte("echo ")

type Candidate struct {
	Word      []byte
	Hash      *big.Int
	SmallHash uint32
}

func init() {
	target_, err := new(big.Int).SetString(targetStr, 0)
	if !err {
		panic(fmt.Sprintf("Can't parse: %s", targetStr))
	}
	target = target_
	for i := 0; i < 26; i++ {
		alphabet = append(alphabet, 'a'+byte(i), 'A'+byte(i))
	}
	for i := 0; i < 10; i++ {
		alphabet = append(alphabet, '0'+byte(i))
	}
}

func IsFactor(a, b *big.Int) bool {
	return new(big.Int).GCD(nil, nil, a, b).Cmp(a) == 0
}

func IsUseful(a *big.Int) bool {
	return IsFactor(a, target)
}

func nextChar(b byte) byte {
	for i, c := range alphabet {
		if c == b {
			if i >= len(alphabet)-1 {
				return alphabet[0]
			}
			return alphabet[i+1]
		}
	}
	return alphabet[0]
}

func Generate(stopChan <-chan bool) <-chan []byte {
	out := make(chan []byte)
	lprefix := prefix[:]
	go func() {
		defer close(out)
		for {
			for _, c := range alphabet {
				word := make([]byte, len(lprefix)+1)
				copy(word, lprefix)
				word[len(lprefix)] = c
				select {
				case <-stopChan:
					return
				case out <- word:
					continue
				}
			}
			lastc := lprefix[len(lprefix)-1]
			if lastc == alphabet[len(alphabet)-1] || len(lprefix) == len(prefix) {
				lprefix = append(lprefix, alphabet[0])
			} else {
				lprefix[len(lprefix)-1] = nextChar(lastc)
			}
		}
	}()
	return out
}

func FilterCandidates(wordChan <-chan []byte) <-chan *Candidate {
	out := make(chan *Candidate)
	go func() {
		defer close(out)
		h := crc32.NewIEEE()
		for w := range wordChan {
			h.Reset()
			h.Write(w)
			v := h.Sum32()
			//fmt.Printf("%s: 0x%08x\n", string(w), v)
			n := big.NewInt(int64(v))
			if IsUseful(n) {
				out <- &Candidate{
					Word:      w,
					Hash:      n,
					SmallHash: v,
				}
			}
		}
	}()
	return out
}

func main() {
	stopChan := make(chan bool)
	for _ = range FilterCandidates(Generate(stopChan)) {
		fmt.Println(".")
	}
}
