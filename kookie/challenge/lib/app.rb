##
# server.rb
# By Ron
# 2019-01
#
# This implements a sinatra-based server.
##

require 'sinatra'
require 'sinatra/base'
require 'sinatra/cookies'
require 'singlogger'
require 'securerandom'

LOGGER = ::SingLogger.instance()
FLAG = "CTF{kookie_cookies}"

ACCOUNTS = {
  'admin' => 'notguest',
  'cookie' => 'monster',
}

module Challenge
  class Server < Sinatra::Base
    helpers Sinatra::Cookies

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
      messages = []
      errors = []

      if(params[:action] == 'login' && !cookies['username'])
        if(ACCOUNTS[params[:username]] == params[:password])
          cookies['username'] = params[:username]
          messages << "Successfully logged in!"
        else
          errors << "Invalid username or password!"
        end
      elsif(params[:action] == 'logout')
        cookies.delete('username')
        messages << "Successfully logged out!"
      end

      if(cookies['username'] == 'admin')
        messages << "Congratulations! You're logged in as <span class='highlight'>admin</span>! Your flag is: <span class='highlight'>#{FLAG}</span>"
      end

      erb :index, :locals => {
        :username  => cookies[:username],
        :messages  => messages,
        :errors    => errors,
      }
    end

    post '/' do
      if params[:answer].downcase == "TODO".downcase
        erb :success, :locals => {
          :message => "Congratulations!",
        }
      else
        erb :failure, :locals => {
          :message => "Sorry, that answer is not correct!",
        }
      end
    end
  end
end
