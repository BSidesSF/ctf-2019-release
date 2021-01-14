FROM debian:latest

RUN useradd -Ums /usr/sbin/nologin rsaos

RUN apt-get -y update && \
  apt-get -y install \
    xinetd \
    openssl

COPY --chown=rsaos:rsaos rsaos /home/rsaos/
COPY --chown=rsaos:rsaos flag.txt /home/rsaos/
COPY rsaos.conf /etc/xinetd.d/rsaos
COPY rsaos.key /home/rsaos/rsaos.key
RUN   chown rsaos:rsaos /home/rsaos/rsaos.key && \
      chmod 400 /home/rsaos/rsaos.key /home/rsaos/flag.txt && \
      chmod 555 /home/rsaos /home/rsaos/rsaos

EXPOSE 9999

CMD ["/usr/sbin/xinetd", "-dontfork"]
