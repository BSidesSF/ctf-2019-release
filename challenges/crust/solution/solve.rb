require 'base32'

target = ARGV[0]
port = ARGV[1]
filename = Base32::encode(ARGV[2]).gsub(/=/, '')

out = `dig @#{target} -p #{port} -t A #{filename} | grep '^#{filename}'`

cleaned = []
out.split("\n").each do |line|
  line = line.gsub(/ +/, "\t")
  a, ttl, b, c, addr = line.split(/\t/)
  puts(line)
  if(!addr)
    puts("Bad!")
    next
  end
  ttl = ttl.to_i / 1000
  a, b, c, d = addr.split(/\./)
  a = a.to_i ^ ttl
  b = b.to_i ^ ttl
  c = c.to_i ^ ttl
  d = d.to_i ^ ttl

  cleaned[a] = [b.chr, c.chr, d.chr]
end

cleaned = cleaned.flatten.join('')
puts(cleaned)
