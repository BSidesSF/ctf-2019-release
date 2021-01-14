FROM debian:latest

RUN useradd -Ums /usr/sbin/nologin dragon

RUN apt-get -y update && \
  apt-get -y install \
    xinetd

COPY --chown=dragon:dragon src/WaterDragon /home/dragon/
COPY --chown=dragon:dragon flag.txt /home/dragon/
COPY dragon.conf /etc/xinetd.d/dragon

EXPOSE 1901

CMD ["/usr/sbin/xinetd", "-dontfork"]
