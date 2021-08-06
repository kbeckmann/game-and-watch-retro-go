#!/usr/bin/env python3

from elftools.elf.elffile import ELFFile

from openocd import OpenOCD

import argparse
import time
import sys

def get_symbol_by_symbol_name(elffile, symbol_name):
    return elffile.get_section_by_name('.symtab').get_symbol_by_name(symbol_name)[0]

def strtohex(data):
    return map(strtohex, data) if isinstance(data, list) else int(data, 16)

# OpenOCD class cherry-picked/inspired from from https://github.com/zmarvel/python-openocd

def logpoll(args):
    with OpenOCD(host=args.host, port=args.port) as ocd:
        last_idx = 0

        with open(args.elf, "rb") as f:
            elffile = ELFFile(f)
            logbuf = get_symbol_by_symbol_name(elffile, "logbuf")
            logbuf_addr = logbuf.entry.st_value
            logbuf_size = logbuf.entry.st_size
            log_idx_addr = get_symbol_by_symbol_name(elffile, "log_idx").entry.st_value

        ocd.send("resume")

        while True:
            if args.halt:
                ocd.send("halt")

            log_idx = ocd.read_memory(32, log_idx_addr, 1)[0]

            if log_idx > last_idx:
                # print the new data since last iteration
                logbuf = ocd.read_memory(8, logbuf_addr + last_idx, log_idx - last_idx)
                logbuf_str = "".join([chr(c) for c in logbuf])
                sys.stdout.write(logbuf_str)
            elif log_idx > 0 and log_idx < last_idx:
                # Get new data from the end of the buffer until the first null byte
                logbuf = ocd.read_memory(8, logbuf_addr + last_idx, logbuf_size - last_idx)
                if 0 in logbuf:
                    end = logbuf.index(0) - 1
                else:
                    end = len(logbuf)
                logbuf_str = "".join([chr(c) for c in logbuf[:end]])

                # Read new data from the beginning and append it
                logbuf = ocd.read_memory(8, logbuf_addr, log_idx)
                logbuf_str += "".join([chr(c) for c in logbuf])
                sys.stdout.write(logbuf_str)

            if args.halt:
                ocd.send("resume")

            last_idx = log_idx
            time.sleep(args.interval / 1000)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Polls the stdout log from a running target")
    parser.add_argument(
        "--elf",
        type=str,
        default="build/gw_retro_go.elf",
        help="Game and Watch Retro-Go ELF file",
    )
    parser.add_argument(
        "--interval",
        "-i",
        type=int,
        default=100,
        help="Polling interval (ms)",
    )
    parser.add_argument(
        "--host",
        type=str,
        default="127.0.0.1",
        help="OpenOCD TCL hostname",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=6666,
        help="OpenOCD TCL port",
    )
    parser.add_argument(
        "--halt",
        dest="halt",
        action="store_true",
        help="Halts the target during memory reads",
    )

    logpoll(parser.parse_args())
