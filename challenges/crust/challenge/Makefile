TAG=gcr.io/bsides-sf-ctf-2019/crust
all: src/main.rs Cargo.toml
	cargo build --release
	ls -lh crust
	cp target/release/crust .
	ls -lh crust
	strip crust
	ls -lh crust
	docker build -t $(TAG) .

run:
	docker run --rm -p 53535:53535/udp -ti $(TAG)

push:
	docker push $(TAG)
