FROM debian:latest

RUN useradd -Ums /usr/bin/nologin slowfire

USER slowfire

WORKDIR /home/slowfire

COPY src/slowfire /home/slowfire/slowfire
COPY flag.txt /home/slowfire/flag.txt

EXPOSE 4141

CMD ["/home/slowfire/slowfire"]
