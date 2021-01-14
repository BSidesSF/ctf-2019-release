FROM ruby:2.5.1

RUN apt-get update && \
    apt-get install -y net-tools

# Install gems
ENV APP_HOME /app
ENV HOME /root
ENV SESSION_SECRET adcc5fe71154bc60f234b6291f795f48
COPY challenge/ $APP_HOME
WORKDIR $APP_HOME
RUN gem install bundler
RUN bundle install

# Start server
ENV PORT 3000
ENV HOST 0.0.0.0
EXPOSE 3000
CMD ["rackup"]
