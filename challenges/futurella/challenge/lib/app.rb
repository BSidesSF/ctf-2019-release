##
# server.rb
# By Ron
# 2019-01
#
# This implements a sinatra-based server.
##

require 'sinatra'
require 'sinatra/base'
require 'singlogger'
require 'securerandom'

LOGGER = ::SingLogger.instance()
FLAG = "CTF{bring_it_back}"

module Challenge
  class Server < Sinatra::Base
    configure do
      if(defined?(PARAMS))
        set :port, PARAMS[:port]
        set :bind, PARAMS[:host]
      end
    end

    not_found do
      return 404, "Unknown path (make sure you're using get/post properly!)"
    end

    get '/' do
      erb :index, :locals => { :flag => FLAG }
    end
  end
end
