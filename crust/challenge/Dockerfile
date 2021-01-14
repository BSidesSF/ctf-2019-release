FROM ubuntu:xenial

RUN adduser --disabled-password --gecos '' ctf

#RUN apt-get update && apt-get install -y xinetd gcc make libc6-dev-i386

WORKDIR /home/ctf
ADD crust flag.txt /home/ctf/
RUN chown -R root:root /home/ctf
RUN chmod 0755 /home/ctf /home/ctf/crust
RUN chmod 0644 /home/ctf/flag.txt
RUN ls -l /home/ctf

USER root
EXPOSE 53535/udp

CMD /home/ctf/crust && sleep infinity
