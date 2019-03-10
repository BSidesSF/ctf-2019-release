FROM debian:latest

RUN useradd -Ums /usr/sbin/nologin dribbles

RUN apt-get -y update && \
  apt-get -y install \
    xinetd

COPY --chown=dribbles:dribbles src/dribbles /home/dribbles/
COPY --chown=dribbles:dribbles src/flag.txt /home/dribbles/
COPY dribbles.conf /etc/xinetd.d/dribbles

EXPOSE 9999

CMD ["/usr/sbin/xinetd", "-dontfork"]
