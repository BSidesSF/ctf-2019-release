FROM debian:latest

RUN useradd -Ums /usr/sbin/nologin pythia

RUN apt-get -y update && \
  apt-get -y install \
    xinetd

COPY --chown=pythia:pythia pythia /home/pythia/
COPY --chown=pythia:pythia oracle/secret_rsa.pem /home/pythia/
COPY pythia.conf /etc/xinetd.d/pythia

EXPOSE 1521

CMD ["/usr/sbin/xinetd", "-dontfork"]
