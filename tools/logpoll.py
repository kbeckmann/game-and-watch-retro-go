# /bin/env python3

from elftools.elf.elffile import ELFFile
from time import sleep

import socket
import sys

with open("build/gw_retro_go.elf", "rb") as f:
    elffile = ELFFile(f)
    logbuf_addr = elffile.get_section_by_name('.symtab').get_symbol_by_name("logbuf")[0].entry.st_value
    log_idx_addr = elffile.get_section_by_name('.symtab').get_symbol_by_name("log_idx")[0].entry.st_value
    print("logbuf_addr:  0x%08X" % logbuf_addr)
    print("log_idx_addr: 0x%08X" % log_idx_addr)



def strtohex(data):
    return map(strtohex, data) if isinstance(data, list) else int(data, 16)

# OpenOCD class cherry-picked/inspired from from https://github.com/zmarvel/python-openocd

class OpenOCD():
    COMMAND_TOKEN = '\x1a'

    def __init__(self, verbose=False, tcl_ip="127.0.0.1", tcl_port=6666):
        self.verbose = verbose
        self.tcl_ip = tcl_ip
        self.tcl_port = tcl_port
        self.buffer_size = 4096

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def __enter__(self):
        self.sock.connect((self.tcl_ip, self.tcl_port))
        return self

    def __exit__(self, type, value, traceback):
        try:
            self.send("exit")
        finally:
            self.sock.close()

    def send(self, cmd):
        """Send a command string to TCL RPC. Return the result that was read."""
        data = (cmd + OpenOCD.COMMAND_TOKEN).encode("utf-8")
        self.sock.send(data)
        return self._recv()

    def _recv(self):
        """Read from the stream until the token (\x1a) was received."""
        data = bytes()
        while True:
            chunk = self.sock.recv(self.buffer_size)
            data += chunk
            if bytes(OpenOCD.COMMAND_TOKEN, encoding="utf-8") in chunk:
                break

        data = data.decode("utf-8").strip()
        data = data[:-1]  # strip trailing \x1a

        return data

    def read_memory(self, wordLen, address, n):
        self.send("array unset output")  # better to clear the array before
        self.send("mem2array output %d 0x%x %d" % (wordLen, address, n))

        output = self.send("capture \"echo \$output\"").split(" ")

        return [int(output[2*i+1]) for i in range(len(output)//2)]


with OpenOCD() as ocd:
    last_idx = 0
    ocd.send("resume")
    while True:
        log_idx = ocd.read_memory(32, log_idx_addr, 1)[0]
        if log_idx > last_idx:
            # print the new data since last iteration
            logbuf = ocd.read_memory(8, logbuf_addr + last_idx,  log_idx - last_idx)
            logbuf_str = "".join([chr(c) for c in logbuf])
            sys.stdout.write(logbuf_str)
        elif log_idx > 0 and log_idx < last_idx:
            # print the whole buffer from 0
            logbuf = ocd.read_memory(8, logbuf_addr,  log_idx)
            logbuf_str = "".join([chr(c) for c in logbuf])
            sys.stdout.write(logbuf_str)
        last_idx = log_idx
        sleep(0.1)
