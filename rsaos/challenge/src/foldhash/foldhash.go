package foldhash

import (
	"crypto/sha1"
	"hash"
	"math/big"
)

type foldHash struct {
	embed hash.Hash
}

func New() hash.Hash {
	return &foldHash{sha1.New()}
}

func (f *foldHash) Write(buf []byte) (int, error) {
	return f.embed.Write(buf)
}

func (f *foldHash) Sum(buf []byte) []byte {
	a, b := big.NewInt(0), big.NewInt(0)
	h := f.embed.Sum(nil)
	a.SetBytes(h[:len(h)/2])
	b.SetBytes(h[len(h)/2:])
	a.Xor(a, b)
	h = a.Bytes()
	return append(buf, h...)
}

func (f *foldHash) Size() int {
	return f.embed.Size() / 2
}

func (f *foldHash) Reset() {
	f.embed.Reset()
}

func (f *foldHash) BlockSize() int {
	return f.embed.BlockSize()
}
