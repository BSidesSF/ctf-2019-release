TAG=gcr.io/bsides-sf-ctf-2019/mainframe
all:
	docker build -t $(TAG) .

run:
	docker run --rm -p 3000:3000 -ti $(TAG)

push:
	docker push $(TAG)

clean:
	docker rm test
