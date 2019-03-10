FROM ruby:2.5.1

RUN apt-get update && \
    apt-get install -y net-tools

# Install gems
ENV APP_HOME /app
ENV HOME /root
COPY challenge/ $APP_HOME
WORKDIR $APP_HOME
RUN gem install bundler
RUN bundle install

# Start server
ENV PORT 6789
ENV HOST 0.0.0.0
EXPOSE 6789
CMD ["rackup"]
