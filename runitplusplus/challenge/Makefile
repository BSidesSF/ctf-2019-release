TAG=gcr.io/bsides-sf-ctf-2019/runitplusplus
all:
	docker build -t $(TAG) .

run:
	docker run --rm -p 5353:5353 -ti $(TAG)

push:
	docker push $(TAG)

clean:
	docker rm $(TAG)
