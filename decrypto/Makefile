all:
	docker build -t test .

run:
	docker run --rm -p 3000:3000 -ti test

clean:
	docker rm test
