FROM debian:buster

RUN useradd -Ums /usr/sbin/nologin sequel

COPY src/sequel /home/sequel/
COPY src/static /home/sequel/static
COPY src/templates /home/sequel/templates

RUN chown -R root:root /home/sequel
RUN chmod -R ugo-w,ugo+rX /home/sequel

EXPOSE 8081

USER sequel

CMD ["/bin/sh", "-c", "cd /home/sequel;/home/sequel/sequel"]
