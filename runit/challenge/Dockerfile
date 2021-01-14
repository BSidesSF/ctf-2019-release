FROM i386/ubuntu:xenial

RUN adduser --disabled-password --gecos '' ctf

RUN apt-get update && apt-get install -y xinetd

WORKDIR /home/ctf

ADD src/flag.txt /home/ctf
RUN chown root.root /home/ctf/flag.txt
RUN chmod 0644 /home/ctf/flag.txt

ADD src/runit /home/ctf/
RUN chown root.root /home/ctf/runit
RUN chmod 0755 /home/ctf/runit

# Set up xinetd
ADD xinetd.conf /etc/xinetd.conf
RUN mkdir /var/log/xinetd/

ADD runit.xinetd /etc/xinetd.d/runit

USER root
EXPOSE 5252

CMD service xinetd restart && tail -F /var/log/xinetd/xinetd.log
