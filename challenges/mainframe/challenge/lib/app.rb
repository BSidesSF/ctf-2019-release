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
FLAG = "CTF{letmein_plz}"

def fakerand(session)
  if(session[:seed].nil?)
    session[:seed] = rand(0x7FFFFFFF)
  end

  session[:seed] = (214013 * session[:seed] + 2531011) & 0x7fffffff
  return session[:seed] >> 16
end

def gen_password(session)
  p = ''
  0.upto(11) do
    p += '%02x' % (fakerand(session) % 0xFF)
  end

  return p
end

module Challenge
  class Server < Sinatra::Base
    use Rack::Session::Cookie, :secret => ENV['SESSION_SECRET']

    def initialize(*args)
      super(*args)
    end

    configure do
      if(defined?(PARAMS))
        set :port, PARAMS[:port]
        set :bind, PARAMS[:host]
        set :threaded, false
      end
    end

    not_found do
      return 404, "Unknown path (make sure you're using get/post properly!)"
    end

    get '/' do
      messages = []
      errors = []

      # Make sure an initial password is set
      if(session[:password].nil?)
        session[:password] = gen_password(session)
      end

      #puts("seed (start) = 0x%08x" % session[:seed])
      # Reset / display a fake password
      if(params[:action] == 'reset_me')
        # Generate one
        my_password = gen_password(session)
        messages << "Password successfully reset! New password: #{my_password}"
      elsif(params[:action] == 'reset_root')
        session[:password] = gen_password(session)
        messages << "<span class='highlight'>root</span> password successfully reset! Log in below"
      elsif(params[:action] == 'login')
        if(params[:password] == session[:password])
          messages << "Congratulations! You're now logged in as <span class='highlight'>root</span>! Your flag is: <span class='highlight'>#{FLAG}</a>"
        else
          errors << "Sorry, that password is incorrect! Don't forget to reset root's password before trying!"
        end
      end
      #puts("seed (end) = 0x%08x" % session[:seed])

      erb :index, :locals => {
        :completed => session[:completed],
        :messages  => messages,
        :errors    => errors,
        :test => session[:password],
      }
    end
  end
end
