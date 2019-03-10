This implements a simple but annoying DNS backdoor on port 53535, written in
rust.

You send it a filename as an A request, base32 encoded:

    dig @localhost -p 53535 -t A F5SXIYZPOBQXG43XMQ

And the response is encoded into IP addresses, where:

* Each octet is XORed by `(TTL / 1000)`
* The first octet is the index
* The next three octets are data

For example, in this:

    F5SXIYZPOBQXG43XMQ.     206143  IN      A       60.187.189.188
    F5SXIYZPOBQXG43XMQ.     82187   IN      A       5.51.32.125
    F5SXIYZPOBQXG43XMQ.     21038   IN      A       26.47.109.47
    F5SXIYZPOBQXG43XMQ.     2130    IN      A       217.34.79.99
    F5SXIYZPOBQXG43XMQ.     30201   IN      A       147.112.113.114
    F5SXIYZPOBQXG43XMQ.     111204  IN      A       121.6.1.85
    F5SXIYZPOBQXG43XMQ.     212034  IN      A       68.161.183.164
    F5SXIYZPOBQXG43XMQ.     196001  IN      A       176.169.165.173
    F5SXIYZPOBQXG43XMQ.     215015  IN      A       66.167.237.248
    F5SXIYZPOBQXG43XMQ.     84114   IN      A       125.94.39.45
    F5SXIYZPOBQXG43XMQ.     78175   IN      A       162.45.42.116

The first entry has a TTL of 206143, so we XOR each octet by 206:

    irb(main):005:0> 60 ^ 206
    => 242
    irb(main):006:0> (187 ^ 206).chr
    => "u"
    irb(main):007:0> (189 ^ 206).chr
    => "s"
    irb(main):008:0> (188 ^ 206).chr
    => "r"


That means the 242th 3-character string is "usr".

There is a [solve.rb](./solve.rb) script in this folder that implements this
solution by wrapping `dig`. It's super ugly code, but works. :)

    bundle install
    ruby ./solve.rb localhost 53535 /home/ctf/flag.txt
    ...
    CTF{how_do_you_like_rust?}
