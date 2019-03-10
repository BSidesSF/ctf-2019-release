TAG=gcr.io/bsides-sf-ctf-2019/futurella
all:
	docker build -t $(TAG) .

run:
	docker run --rm -p 6789:6789 -ti $(TAG)

push:
	docker push $(TAG)

clean:
	docker rm $(TAG)
