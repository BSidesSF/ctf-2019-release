FROM debian:latest

RUN useradd -Ums /usr/sbin/nologin fastflag

RUN apt-get -y update && \
  apt-get -y install \
    xinetd

COPY --chown=fastflag:fastflag src/fastflag /home/fastflag/
COPY --chown=fastflag:fastflag src/flag.txt /home/fastflag/
COPY fastflag.conf /etc/xinetd.d/fastflag

EXPOSE 9999

CMD ["/usr/sbin/xinetd", "-dontfork"]
