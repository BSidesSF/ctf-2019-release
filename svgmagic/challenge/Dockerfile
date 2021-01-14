FROM debian:latest

RUN useradd -Us /usr/sbin/nologin svgmagic

RUN apt-get -y update && apt-get install -y \
  python3 \
  python3-pip \
  libffi-dev \
  libcairo2 \
  gunicorn3

COPY --chown=svgmagic:svgmagic src /srv/

RUN pip3 install -r /srv/requirements.txt

EXPOSE 9999/tcp

CMD gunicorn3 \
  -w 16 \
  --preload \
  --chdir /srv \
  -u svgmagic \
  -g svgmagic \
  -b 0.0.0.0:9999 \
  svgmagic:app
