FROM ubuntu:xenial

RUN adduser --disabled-password --gecos '' ctf

RUN apt-get update && apt-get install -y xinetd

WORKDIR /home/ctf

ADD src/flag.txt /home/ctf
RUN chown root.root /home/ctf/flag.txt
RUN chmod 0644 /home/ctf/flag.txt

ADD src/straw-clutcher /home/ctf/
RUN chown root.root /home/ctf/straw-clutcher
RUN chmod 0755 /home/ctf/straw-clutcher

# Set up xinetd
ADD xinetd.conf /etc/xinetd.conf
RUN mkdir /var/log/xinetd/

ADD straw-clutcher.xinetd /etc/xinetd.d/straw-clutcher

USER root
EXPOSE 4321

CMD service xinetd restart && tail -F /var/log/xinetd/xinetd.log
