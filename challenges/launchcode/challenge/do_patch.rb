IN = './calc-original.exe'
OUT = './calc-patched.exe'

srand()

FLAG = "CTF{LAUNCHED!~}\0".chars.map { |b| "%c\0" % b }.join
if(FLAG.length != 32)
  puts("Flag should be exactly 32 characters! Not %d!" % FLAG.length)
  exit
end

DIGITS = {
  0x7c => '0',
  0x7d => '1',
  0x7e => '2',
  0x7f => '3',
  0x80 => '4',
  0x81 => '5',
  0x82 => '6',
  0x83 => '7',
  0x84 => '8',
  0x85 => '9',
  0x5c => '+',
  0x5d => '-',
  0x5b => '*',
  0x5a => '/',
}

CODE = ([0]*(FLAG.length/4)).map() { DIGITS.keys.sample.chr }.join()

puts("Raw code: #{CODE.bytes.map {|i| '%02x' % i }.join(' ')}")
puts("Code: #{CODE.bytes.map {|i| DIGITS[i] }.join(' ')}")

ENCODED_FLAG = CODE.bytes.each_with_index.map { |b, i| (b ^ FLAG[i*4].ord).chr + (b ^ FLAG[(i*4)+1].ord).chr + (b ^ FLAG[(i*4)+2].ord).chr + (b ^ FLAG[(i*4)+3].ord).chr }.join()

puts("Code: #{CODE.unpack('H*')}")
puts("Encoded flag: #{ENCODED_FLAG.bytes.map { |b| '%02x' % b }.join(' ')}")

patches = [
  # This patch calls the handler code at each character press by adding a "call"
  { offset: 0x1ca6,  max: 0x05, data: "\xe8\x82\x55\x00\x00" },

  # This patch gets rid of the "real" functionality by making it simply "return"
  { offset: 0x722c,  max: 0x01, data: "\xc3" },

  # This patch removs some console i/o stuff, giving us 20 bytes of freed-up
  # r/w memory where we store the counter
  { offset: 0x11d86, max: 0x0f, data: "\x90" * 0x0f },

  # This is the actual binary patch, which adds all the functionality (except
  # for the encoded flag)
  { offset: 0x722d,  max: 0x9c, file: 'realpatch.bin' },

  # This is the counter for the current byte we're at
  { offset: 0x1300c, max: 0x04, data: "\0\0\0\0" },

  # This is the r/w "encrypted" data (in maybe-unused memory)
  { offset: 0x130c0, max: 0x20, data: ENCODED_FLAG },

  # This is the r/o "validator" data (it's 9 past the start of the realpatch data)
  { offset: 0x722d+9, max: 0x08, data: CODE },

  # This kills the annoying "This operation is taking too long!!!" thread
  { offset: 0x3be6, max: 0x10, data: "\x90" * 0x10 },
]


data = File.open(IN, "rb").read()

patches.each do |p|
  patch = p[:data]
  if(patch.nil?)
    patch = File.open(p[:file], 'rb').read()
  end

  if(patch.length > p[:max])
    puts("Patch %s is too long: %d" % [p[:file] || '(inline data)', patch.length])
    exit
  end

  puts("Patching %d bytes @ 0x%04x from %s" % [patch.length, p[:offset], p[:file] ? p[:file] : "(inline data)"])

  patch.bytes.each_with_index do |b, i|
    data[p[:offset] + i] = b.chr
  end
end

File.open(OUT, "wb").write(data)
