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

require 'board'

LOGGER = ::SingLogger.instance()
FLAG = "CTF{i_beat_the_impossible}"
MOVES = {
  'ul' => 0,
  'u'  => 1,
  'ur' => 2,
  'l'  => 3,
  'c'  => 4,
  'r'  => 5,
  'bl' => 6,
  'b'  => 7,
  'br' => 8,
}

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
      end
    end

    not_found do
      return 404, "Unknown path (make sure you're using get/post properly!)"
    end

    post '/move' do
      begin
        move = MOVES[params[:move]]
        if(move.nil?)
          erb :error, :locals => { message: "Bad move!" }
          return
        end

        if(session[:board])
          session[:board].play(move)
          session[:board].computer_play()
        end

        redirect '/'
      rescue StandardError => e
        erb :error, :locals => { message: "#{e}" }
      end
    end

    post '/reset' do
      session[:board] = Board.new()

      redirect '/'
    end

    get '/' do
      begin
        if(session[:board].nil?)
          session[:board] = Board.new()
        end

        state, winner = Board.state(session[:board].board)
        locals = { board: session[:board].board }

        if state == WIN && winner == COMPUTER
          erb :lose, :locals => locals
        elsif state == WIN && winner == HUMAN
          locals[:flag] = FLAG
          erb :win, :locals => locals
        elsif state == DRAW
          erb :draw, :locals => locals
        else
          erb :index, :locals => locals
        end
      rescue StandardError => e
        erb :error, :locals => { message: "Uh oh! #{e}" }
      end
    end
  end
end
