TAG=gcr.io/bsides-sf-ctf-2019/runit
all:
	docker build -t $(TAG) .

run:
	docker run --rm -p 5252:5252 -ti $(TAG)

push:
	docker push $(TAG)

clean:
	docker rm $(TAG)
