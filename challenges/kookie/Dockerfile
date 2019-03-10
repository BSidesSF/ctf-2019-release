FROM ruby:2.5.1

RUN apt-get update && \
    apt-get install -y net-tools

# Install gems
ENV APP_HOME /app
ENV HOME /root
ENV SESSION_SECRET 486741de825986be2438562ed95345f5
COPY challenge/ $APP_HOME
WORKDIR $APP_HOME
RUN gem install bundler
RUN bundle install

# Start server
ENV PORT 4455
ENV HOST 0.0.0.0
EXPOSE 4455
CMD ["rackup"]
