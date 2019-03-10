GOFILES=oracle/oracle.go

rsaos: $(GOFILES)
	go build -o pythia ${GOFILES}

docker: pythia
	docker build -t gcr.io/bsides-sf-ctf-2019/pythia .
