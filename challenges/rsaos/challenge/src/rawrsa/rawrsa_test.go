package rawrsa

import (
	"hash/crc32"
	"strings"
	"testing"
)

var testKey = `
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA5GJLLqLAOS6wL9RxleXVyIoeuD9VuykteDHL0Zs5m8CYO14d
ht3dx/KQGekVUtDy/JP0VKiPgw5p8F5uVP3x3CpB9Max0BH++G4z0tfafUFJOmJc
mW6mzizBpABCFkPc3k+vcfQt8aNXOxWKwbgT+kPu5ZFPI62GhHQpwILH78twWwQq
gWUA4FPFwDqo7aGaGY7mKgSJqzMz+tcHJybp38G4LP5PWI7LhOpqKZQrb/ZrB2zw
EtYnje5yl+1p2z+JiumcHYknsTzDcuYEtAnVKj4HbbbOzapZnyf94onNDEHrR+pd
5/lLv6zOtkqXtnIiuzyqEECoYavD+j4UF+5+xQIDAQABAoIBAQDScbR/W56u0lOt
LyAfFPLCYGtgMZQo6w+n8pKyKUuTNovQILAqhySGyv93sKxibIGKtjbtD4l2JQD/
abRXAs0rp5CxIanVZc/BUaLVROtigZcdgmd7RlfUIa+pFEfRRmUuQNUUtwDi6TVH
D1mVma5m7NQkm5oF8S7DvFqAtsHPNbQf9/3Bzmna4kxLDht4xlOZMTXQXTHL2PhY
QRZKEn/+a2kIlCEdAO9l7qo0bMzIVcRj49MbD+Liomac2McfdXClB4TjMisWH3eL
1SRtzHHdV0L3oHkdY+/hpQT15kpJ4qXFL7CrN7ENGm84WTdKKH7Clr1UEXRdEana
tz6gdBFhAoGBAPiVwlqFj2ZYMTF7nZ7whMxRBROzwutWtSQkewr80pO1c11MgUtl
IYH1H8R85Te3M2wF05qTZ4cOFhCz3WS2uOvk3heOfPvFPVkFQZjMCiLmYVgavdAt
gQ48uGYulm/94lqPVqdvOKB1iEfLdVmaE+jOKXbq3XIkbK04atNSwijJAoGBAOsy
RpDTM3tVmWCVGMukwHkNd+HCOx1pfoxuIgh4lNb29Zn+gEW+EFmcnnytUbtFrrm7
IU9dOeige0NxqqT3o5TrC6Qe5lTYf9bEjz7lFTHV8ZmEqq1Ej5JzssKHCigjP0+S
7l00EEi3fpzOHWqOm0IU98OfWHN0ft845AWSQOAdAoGAb99GBmMRNCgAi5psRzlL
KoBT3pgJ4su03b0lOr6pLhx8/MIPdhTfTZM5Zr4mkjp6g+TjZC9uahNmzsGDo22w
L0liwgs4KD6N62A4M0GwViCEMphmUCgKapPdEO9qeKAznDnh75xYSa5bVKQ1/csf
pqgRdRkh69qEpjg7qzsVcjECgYEAxgU8WG8Zpzu4uRKPH9YEn2bxikhIEUli3MVV
LiCn4B1pR1B5mz6c8XXmEj6y06OlqoadUv7A5NVJMs2eyOoe9iC3WmURtFnbuG0h
1eh2YdEpawHB2cQhxUJYPWQI7yZOBadFvX86Pp8PoOIA7lhG6+pPeIrOo9Zvg22Y
vgAXUxkCgYBD+89zQ6y2yV7M3Yc+KPMs0pmMYqlWGNYtjIDbKfYqnOtuFy6BlSrm
ndH7MExA8Tn/0Ir7bB368jAJcpmnsZ/iPO/jSVoQZFss4hMKxhlT2fvXaa3QkKSH
UI8v9B87bDzWet/xngdu75xNPW857Sfe4CPTWYfqK5qwJBUYUfYSbw==
-----END RSA PRIVATE KEY-----`

func TestRawRsa(t *testing.T) {
	pk := loadPEMPrivateKey([]byte(strings.TrimSpace(testKey)))
	if pk == nil {
		t.Fatal("Unable to load private key!")
		return
	}
	r := &RSA{}
	r.extractPrivateComponents(pk)
	msg := []byte("This is not a secret.")
	sig := r.HashAndSign(msg, crc32.NewIEEE())
	if !r.HashAndVerify(msg, crc32.NewIEEE(), sig) {
		t.Fatal("Verification failed!")
	}
}
