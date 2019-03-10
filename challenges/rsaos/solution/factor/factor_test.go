package factor

import (
	"crypto/md5"
	"fmt"
	"math/big"
	"math/rand"
	"testing"
)

func sameBigElements(a, b []*big.Int) bool {
	if len(a) != len(b) {
		return false
	}
	for _, c := range a {
		found := false
		for i, d := range b {
			if c.Cmp(d) == 0 {
				found = true
				b = append(b[:i], b[i+1:]...)
				break
			}
		}
		if !found {
			return false
		}
	}
	return true
}

func TestPrimeFactor_Factors(t *testing.T) {
	n := big.NewInt(1)
	exp := make([]*big.Int, 0)
	for len(exp) < 4 {
		v := big.NewInt(int64(rand.Int31()))
		exp = append(exp, v)
		n.Mul(n, v)
	}
	factors, err := PrimeFactor(n)
	if err != nil {
		t.Errorf("Got unexpected factoring error %s", err)
	}
	if len(factors) < 4 {
		t.Errorf("Expected at least 4 factors, got: %v", factors)
	}
	res := big.NewInt(1)
	for _, v := range factors {
		res.Mul(res, v)
	}
	if res.Cmp(n) != 0 {
		t.Errorf("%v != %v, %v", res, n, factors)
	}
}

func TestPrimeFactor_FactorsRandom(t *testing.T) {
	for i := 0; i < 4; i++ {
		n := big.NewInt(1)
		exp := make([]*big.Int, 0)
		for len(exp) < 4 {
			v := big.NewInt(int64(rand.Int31()))
			exp = append(exp, v)
			n.Mul(n, v)
		}
		factors, err := PrimeFactor(n)
		fmt.Printf("%v -> %v\n", n, factors)
		if err != nil {
			t.Errorf("Got unexpected factoring error %s", err)
		}
		if len(factors) < 4 {
			t.Errorf("Expected at least 4 factors, got: %v", factors)
		}
		res := big.NewInt(1)
		for _, v := range factors {
			res.Mul(res, v)
		}
		if res.Cmp(n) != 0 {
			t.Errorf("%v != %v, %v", res, n, factors)
		}
	}
}

func DisabledTestPrimeFactor_MD5(t *testing.T) {
	s := "get-flag #"
	for {
		h := md5.New()
		h.Write([]byte(s))
		n := big.NewInt(0)
		n.SetBytes(h.Sum(nil))
		fmt.Printf("MD5(%s) = 0x%0x\n", s, n)
		factors, err := PrimeFactor(n)
		if err != nil {
			s = s + " "
			continue
			//t.Errorf("Got factoring error: %s", err)
		}
		fmt.Printf("Factors: %v\n", factors)
		break
	}
}

func BenchmarkPrimeFactor(b *testing.B) {
	b.StopTimer()
	randInt := func() *big.Int {
		n := big.NewInt(1)
		for i := 0; i < 4; i++ {
			v := big.NewInt(int64(rand.Int31()))
			n.Mul(n, v)
		}
		return n
	}
	n := randInt()
	b.StartTimer()
	PrimeFactor(n)
}
