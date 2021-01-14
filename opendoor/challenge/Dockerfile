FROM debian:buster

RUN useradd -Ums /usr/sbin/nologin opendoor

COPY src/opendoor /home/opendoor/
COPY src/flag.txt /home/opendoor/
RUN chown -R root:root /home/opendoor && chmod -R ugo-w,ugo+rX /home/opendoor

EXPOSE 4141

USER opendoor

CMD ["/home/opendoor/opendoor"]
