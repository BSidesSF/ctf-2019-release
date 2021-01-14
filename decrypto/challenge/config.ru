$LOAD_PATH.unshift(File.expand_path(File.dirname(__FILE__) + '/lib'))

require 'sinatra'
require 'optimist'
require 'singlogger'

OPTS = Optimist::options do
  opt(
    :log_level,
    "The log level: DEBUG, INFO, WARN, ERROR, or FATAL",
    :type => :string,
    :default => 'DEBUG',
  )

  opt(
    :host,
    "The host to listen on",
    :type => :string,
    :default => ENV['HOST'] || '127.0.0.1',
  )

  opt(
    :port,
    "The port to listen on",
    :type => :integer,
    :default => ENV['PORT'] ? ENV['PORT'].to_i : 1234
  )
end

::SingLogger.set_level_from_string(level: OPTS[:log_level])

# A bit hacky, but define the module and create a params constant
module Challenge
  PARAMS = {
    :host => OPTS[:host],
    :port => OPTS[:port],
  }
end

# The main application that will be run
require 'app'
run ::Challenge::Server.run!()
