GOFILES=$(wildcard src/*.go) $(wildcard src/*/*.go)

rsaos: $(GOFILES)
	cd src/cmd && go build -o ../../rsaos .

docker: rsaos
	docker build -t gcr.io/bsides-sf-ctf-2019/rsaos .
