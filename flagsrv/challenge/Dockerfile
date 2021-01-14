FROM debian:buster

RUN useradd -Ums /usr/sbin/nologin flagsrv

COPY src/flagsrv /home/flagsrv/
COPY src/static /home/flagsrv/static
COPY src/templates /home/flagsrv/templates

RUN chown -R root:root /home/flagsrv
RUN chmod -R ugo-w,ugo+rX /home/flagsrv

EXPOSE 8081

USER flagsrv

CMD ["/bin/sh", "-c", "cd /home/flagsrv;/home/flagsrv/flagsrv"]
