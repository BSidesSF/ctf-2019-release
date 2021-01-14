TAG=gcr.io/bsides-sf-ctf-2019/straw-clutcher
all:
	docker build -t $(TAG) .

run:
	docker run --rm -p 4321:4321 -ti $(TAG)

push:
	docker push $(TAG)

clean:
	docker rm $(TAG)
