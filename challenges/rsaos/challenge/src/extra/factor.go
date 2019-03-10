package extra

import (
	"fmt"
	"math/big"
)

func PrimeFactor(n *big.Int) ([]*big.Int, error) {
	res := make([]*big.Int, 0)

	testFactor := func(div *big.Int) bool {
		q, r := new(big.Int).QuoRem(n, div, new(big.Int))
		if r.Sign() == 0 {
			n = q
			res = append(res, new(big.Int).Set(div))
			return true
		}
		return false
	}

	// Wheel factorization
	basis := []*big.Int{
		big.NewInt(2),
		big.NewInt(3),
		big.NewInt(5),
		big.NewInt(7),
		big.NewInt(11),
		big.NewInt(13),
	}

	step := big.NewInt(1)
	wheel := make([]*big.Int, 0)
	for _, b := range basis {
		step.Mul(step, b)
	}

	// Make the wheel
	one := big.NewInt(1)
	for i := big.NewInt(2); i.Cmp(step) <= 0; i.Add(i, one) {
		if new(big.Int).GCD(nil, nil, i, step).Cmp(one) == 0 {
			wheel = append(wheel, new(big.Int).Set(i))
		}
	}
	wheel = append(wheel, new(big.Int).Add(step, one))

	// test basis
	for _, b := range basis {
		for testFactor(b) {
		} // intentionally blank
	}

	// spin the wheel
	for {
		if n.Cmp(wheel[0]) < 0 {
			break
		}
		// loop around wheel
		for _, w := range wheel {
			for testFactor(w) {
			} // intentionally blank
			w.Add(w, step)
		}
		if wheel[0].BitLen() > 32 {
			return res, fmt.Errorf("Value is not 2^32-smooth.")
		}
	}

	if n.Cmp(big.NewInt(1)) != 0 {
		res = append(res, new(big.Int).Set(n))
	}

	return res, nil
}
