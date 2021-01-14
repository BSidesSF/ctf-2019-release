TAG=gcr.io/bsides-sf-ctf-2019/genius
all:
	docker build -t $(TAG) .

run:
	docker run --rm -p 1338:1338 -ti $(TAG)

push:
	docker push $(TAG)

clean:
	docker rm $(TAG)
