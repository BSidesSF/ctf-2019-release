package foldhash

import (
	"bytes"
	"testing"
)

func TestFoldHash(t *testing.T) {
	exp := []byte{140, 232, 70, 102, 133, 106, 29, 186, 98, 163}
	h := New()
	h.Write([]byte("abc"))
	if !bytes.Equal(exp, h.Sum(nil)) {
		t.Fatalf("Got %v, expected %v", h.Sum(nil), exp)
	}
}
