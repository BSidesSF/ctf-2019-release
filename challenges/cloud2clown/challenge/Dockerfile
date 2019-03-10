FROM debian:latest

RUN apt-get -y update && apt-get -y install \
  ca-certificates

RUN useradd -Ums /usr/sbin/nologin clown

COPY src/cloud2clown /home/clown/
COPY src/flag.txt /home/clown/
COPY src/flag2.txt /home/clown/
COPY src/static /home/clown/static
COPY src/index.html /home/clown/
COPY src/robots.txt /home/clown/

RUN chown -R root:root /home/clown
RUN chmod -R ugo-w,ugo+rX /home/clown

EXPOSE 8081

USER clown

CMD ["/home/clown/cloud2clown"]
