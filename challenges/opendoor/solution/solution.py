import pwn
import struct
import time
from Crypto.Cipher import AES


def pack_message(msg_type, msg_data):
    rv = struct.pack(
            '>II', msg_type, int(time.time())) + msg_data
    print(rv.encode('hex'))
    return rv


def send_recv_ping(conn):
    conn.send(pack_message(1, ''))
    r = conn.recv()
    print(r.encode('hex'))


def send_recv_unlock(conn):
    msg = struct.pack(
            '>QQ', 0x55AA55AA5A5AA5A5, 0x55AA55AA5A5AA5A5)
    msg_id = 3
    packed = pack_message(msg_id, msg)
    conn.send(packed)
    r = conn.recv()
    print(r.encode('hex'))


def send_recv_debug_enable(conn):
    msg = struct.pack('>I?', 1, True)
    msg_id = 5
    packed = pack_message(msg_id, msg)
    conn.send(packed)
    r = conn.recv()
    print(r.encode('hex'))


def send_recv_read_file(conn):
    s = './flag.txt'
    msg = struct.pack('>II', 2, len(s)) + s
    msg_id = 5
    packed = pack_message(msg_id, msg)
    conn.send(packed)
    r = conn.recv()
    print(r.encode('hex'))
    msg_id, timestamp, op, str_len = struct.unpack('>IIII', r[:16])
    print(r[16:])


class EncryptedConn(object):

    def __init__(self, conn):
        self.conn = conn
        self.key = ''.join(chr(c) for c in [
                0x97, 0x8b, 0x8b, 0x8f, 0x8c, 0xc5, 0xd0, 0xd0,
                0x88, 0x88, 0x88, 0xd1, 0x8c, 0x86, 0x8c, 0x8b,
                0x9a, 0x92, 0x90, 0x89, 0x9a, 0x8d, 0x93, 0x90,
                0x8d, 0x9b, 0xd1, 0x9c, 0x90, 0x92, 0xd0, 0xff
                ])
        self.iv = ''.join(chr(c) for c in [
                0x6e, 0x6f, 0x74, 0x61, 0x66, 0x6c, 0x61, 0x67,
                0x6e, 0x6f, 0x74, 0x61, 0x66, 0x6c, 0x61, 0x67
                ])

    def send(self, buf):
        aes = AES.new(self.key, mode=AES.MODE_CBC, IV=self.iv)
        pad_len = 16 - (len(buf) % 16)
        pad = chr(pad_len) * pad_len
        buf = aes.encrypt(buf + pad)
        buf = struct.pack('>I', len(buf)) + buf
        return self.conn.send(buf)

    def recv(self):
        aes = AES.new(self.key, mode=AES.MODE_CBC, IV=self.iv)
        r = self.conn.recv()
        print('Raw: "%s"' % r.encode('hex'))
        l = struct.unpack('>I', r[:4])[0]
        print(l)
        r = r[4:]
        #assert(len(r) == l)
        pt = aes.decrypt(r)
        pad_len = ord(pt[-1])
        print('Pad len: %d' % pad_len)
        pt = pt[:-pad_len]
        print('Plaintext: "%s"' % pt.encode('hex'))
        return pt


class PlainConn(object):

    def __init__(self, conn):
        self.conn = conn

    def send(self, buf):
        buf = struct.pack('>I', len(buf)) + buf
        return self.conn.send(buf)

    def recv(self):
        rv = self.conn.recv()
        l = struct.unpack('>I', rv[:4])[0]
        rv = rv[4:]
        assert(len(rv) == l)
        return rv


def main():
    conn = pwn.connect('localhost', 4848)
    conn = EncryptedConn(conn)
    send_recv_ping(conn)
    send_recv_unlock(conn)
    send_recv_debug_enable(conn)
    send_recv_read_file(conn)


if __name__ == '__main__':
    main()
