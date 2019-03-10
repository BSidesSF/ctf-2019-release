TAG=gcr.io/bsides-sf-ctf-2019/pick-tac-toe:latest
all:
	docker build -t $(TAG) .

run:
	docker run --rm -p 3000:3000 -ti $(TAG)

push:
	docker push $(TAG)

clean:
	docker rm $(TAG)
