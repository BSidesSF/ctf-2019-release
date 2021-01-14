# encoding: ASCII-8BIT

##
# Demo.rb
# Created: February 10, 2013
# By: Ron Bowes
#
# A demo of how to use Poracle, that works against RemoteTestServer.
##
#
#require 'httparty'
require './Poracle'
require 'httparty'
require 'singlogger'
require 'uri'

# Note: set this to DEBUG to get full full output
SingLogger.set_level_from_string(level: "DEBUG")
L = SingLogger.instance()

# 16 is good for AES and 8 for DES
BLOCKSIZE = 16

def request(cookies)
  return HTTParty.get(
    'http://localhost:3000/',
    follow_redirects: false,
    headers: {
      'Cookie' => "signature=#{cookies[:signature]}; user=#{cookies[:user]}; rack.session=#{cookies[:session]}"
    }
  )
end

def get_cookies()
  reset = HTTParty.head('http://localhost:3000/?action=reset', follow_redirects: false)
  cookies = reset.headers['Set-Cookie']

  return {
    signature: cookies.scan(/signature=([0-9a-f]*)/).pop.pop,
    user:      cookies.scan(/user=([0-9a-f]*)/).pop.pop,
    session:   cookies.scan(/rack\.session=([^;]*)/).pop.pop,
  }
end

# Get the initial set of cookies
COOKIES = get_cookies()

# This is the do_decrypt block - you'll have to change it depending on what your
# service is expecting (eg, by adding cookies, making a POST request, etc)
poracle = Poracle.new(BLOCKSIZE) do |data|
  cookies = COOKIES.clone()
  cookies[:user] = data.unpack("H*").pop

  result = request(cookies)
  #result.parsed_response.force_encoding("ASCII-8BIT")

  # Split the response and find any line containing error / exception / fail
  # (case insensitive)
  errors = result.parsed_response.split(/\n/).select { |l| l =~ /bad decrypt/i }

  # Return true if there are zero errors
  errors.empty?
end

data = COOKIES[:user]

L.info("Trying to decrypt: %s" % data)

# Convert to a binary string using pack
data = [data].pack("H*")

result = poracle.decrypt_with_embedded_iv(data)

# Print the decryption result
puts("-----------------------------")
puts("Decryption result")
puts("-----------------------------")
puts result
puts("-----------------------------")
puts()

# Write it to a file
File.open("/tmp/decrypt", "wb") do |f|
  f.write(result)
end

# Call out to hash_extender and pull out the new data
append = "\nUID 0\n".unpack("H*").pop
out = `./hash_extender --file=/tmp/decrypt -s #{COOKIES[:signature]} -a '#{append}' --append-format=hex -f sha256 -l 8`
new_signature = out.scan(/New signature: ([0-9a-f]*)/).pop.pop
new_data = out.scan(/New string: ([0-9a-f]*)/).pop.pop

# Call out to Poracle to encrypt the new data
new_encrypted_data = poracle.encrypt([new_data].pack('H*'))

# Perform the request to get the flag
cookies = COOKIES.clone
cookies[:user] = new_encrypted_data.unpack("H*").pop
cookies[:signature] = new_signature
puts(request(cookies))
