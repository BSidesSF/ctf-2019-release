import subprocess

min_char = 0x20
max_char = 0x7F


class fastflag(object):

    def __init__(self):
        self.known = ""
        self.conn = subprocess.Popen(
                ["../challenge/fastflag"],
                stdout=subprocess.PIPE,
                stdin=subprocess.PIPE)
        self.finished = False

    def find_flag(self):
        while not self.finished:
            self.known += self.bin_search()
        return self.known

    def bin_search(self, min_char=min_char, max_char=max_char):
        c = chr((min_char + max_char)/2)
        val = self.known + c + '\n'
        rv = self.iterate_value(val)
        if rv == 0:
            self.finished = True
            return c
        exp = len(self.known) % 16 + 1
        if abs(rv) - exp:
            return c
        if min_char == max_char:
            raise Exception('WTF')
        if rv < 0:
            return self.bin_search(min_char, ord(c))
        return self.bin_search(ord(c)+1, max_char)

    def iterate_value(self, val):
        self.conn.stdin.write(val)
        rv = self.conn.stdout.readline()
        if '[' not in rv:
            return 0
        _, rv = rv.split('[')
        n, _ = rv.split(']')
        return int(n)

f = fastflag()
print f.find_flag()
