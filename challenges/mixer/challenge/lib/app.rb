##
# server.rb
# By Ron
# 2019-01
#
# This implements a sinatra-based server.
##

require 'rubygems'
require 'sinatra'
require 'sinatra/base'
require 'sinatra/cookies'
require 'singlogger'
require 'securerandom'

LOGGER = ::SingLogger.instance()
FLAG = "CTF{is_fun!!uffling_block_sh}"

def encrypt_aes_ecb(session, text)
  alg = "AES-256-ECB"

  if(session[:aes_key].nil?)
    session[:aes_key] = OpenSSL::Cipher::Cipher.new(alg).random_key
  end

  aes = OpenSSL::Cipher::Cipher.new(alg)
  aes.encrypt
  aes.key = session[:aes_key]

  # Now we go ahead and encrypt our plain text.
  return (aes.update(text) + aes.final).unpack("H*").pop
end

def decrypt_aes_ecb(session, ciphertext)
  alg = "AES-256-ECB"

  ciphertext = [ciphertext.gsub(/ /, '')].pack('H*')
  if(session[:aes_key].nil?)
    session[:aes_key] = OpenSSL::Cipher::Cipher.new(alg).random_key
  end

  aes = OpenSSL::Cipher::Cipher.new(alg)
  aes.decrypt
  aes.key = session[:aes_key]
  aes.padding = 0

  return (aes.update(ciphertext) + aes.final).bytes().map { |b| (b < 0x20 || b > 0x7F) ? '?'.ord : b }.pack('c*').gsub(/\?*$/, '')
end

module Challenge
  class Server < Sinatra::Base
    use Rack::Session::Cookie, :secret => ENV['SESSION_SECRET']
    helpers Sinatra::Cookies

    def initialize(*args)
      super(*args)
    end

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

      if(params[:action] == 'login')
        cookies[:user] = encrypt_aes_ecb(session, {
          'first_name' => params[:first_name],
          'last_name' => params[:last_name],
          'is_admin' => 0,
        }.to_json)

        redirect('/')
      elsif(params[:action] == 'logout')
        cookies.delete(:user)

        redirect('/')
      end

      user = nil
      if(cookies[:user])
        begin
          user = JSON.parse(decrypt_aes_ecb(session, cookies[:user]))
        rescue JSON::ParserError => e
          errors << "Error parsing JSON: #{e}"
        rescue StandardError => e
          errors << "Uh oh, something went wrong: #{e}"
        end
      end

      erb :index, :locals => {
        :messages  => messages,
        :errors    => errors,
        :user => user,
      }
    end
  end
end
