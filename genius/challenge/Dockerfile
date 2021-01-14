FROM ubuntu:xenial

RUN adduser --disabled-password --gecos '' ctf

RUN apt-get update && apt-get install -y xinetd gcc make libc6-dev-i386

WORKDIR /home/ctf
ADD genius loader flag.txt /home/ctf/
RUN chown -R root:root /home/ctf
RUN chmod 0755 /home/ctf /home/ctf/genius /home/ctf/loader
RUN chmod 0644 /home/ctf/flag.txt
RUN ls -l /home/ctf

ADD genius.xinetd /etc/xinetd.d/genius

USER root
EXPOSE 1338

CMD service xinetd restart && sleep infinity
