FROM ruby:2.5.1

RUN apt-get update && \
    apt-get install -y net-tools

# Install gems
ENV APP_HOME /app
ENV HOME /root
ENV SESSION_SECRET 98ba7094315e253b1cb4e3aa4f4faa78
COPY challenge/ $APP_HOME
WORKDIR $APP_HOME
RUN gem install bundler
RUN bundle install

# Start server
ENV PORT 3000
ENV HOST 0.0.0.0
EXPOSE 3000
CMD ["rackup"]
