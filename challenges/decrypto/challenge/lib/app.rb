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
require_relative './flag'

LOGGER = ::SingLogger.instance()
ALG = "AES-256-CBC"

def encrypt(key, text)
  aes = OpenSSL::Cipher::Cipher.new(ALG)
  aes.encrypt
  aes.key = key

  iv = OpenSSL::Cipher::Cipher.new(ALG).random_iv
  aes.iv = iv

  # Now we go ahead and encrypt our plain text.
  return (iv + aes.update(text) + aes.final).unpack("H*").pop
end

def decrypt(key, ciphertext)
  ciphertext = [ciphertext.gsub(/ /, '')].pack('H*')

  iv, ciphertext = ciphertext.unpack('a16a*')
  aes = OpenSSL::Cipher::Cipher.new(ALG)
  aes.decrypt
  aes.key = key
  aes.iv = iv

  return (aes.update(ciphertext) + aes.final)
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

      if(params[:action] == 'reset' || session[:key].nil? || cookies[:signature].nil? || cookies[:user].nil?)
        session[:secret] = SecureRandom.random_bytes(8)
        session[:key] = OpenSSL::Cipher::Cipher.new(ALG).random_key

        user_blob = "UID #{rand(32) + 32}\nNAME baseuser\nSKILLS n/a\n"
        cookies[:signature] = Digest::SHA256.hexdigest(session[:secret] + user_blob)
        cookies[:user] = encrypt(session[:key], user_blob)

        #redirect('/')
        #return
      end

      data = {}
      begin
        user_blob = decrypt(session[:key], cookies[:user])
        if(Digest::SHA256.hexdigest(session[:secret] + user_blob) != cookies[:signature])
          raise(StandardError, "Bad signature!")
        end

        user_blob.split(/\n/).each do |u|
          k, v = u.split(/ /, 2)
          data[k] = v
        end
      rescue StandardError => e
        errors << e
      end

      erb :index, :locals => {
        :messages  => messages,
        :errors    => errors,
        :data      => data,
        :session   => session,
      }
    end
  end
end
