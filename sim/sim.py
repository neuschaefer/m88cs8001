#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

import argparse, time

from soc import *

def main():
    parser = argparse.ArgumentParser(description='M88CS8001 simulator')
    parser.add_argument('--flash', help='flash image to use', required=True)
    parser.add_argument('--uart0-tty', help='attach UART0 to TTY')
    parser.add_argument('--uart1-tty', help='attach UART1 to TTY')
    #parser.add_argument('--trace', help='enable tracing for the specified peripheral')
    #parser.add_argument('--no-trace', help='disable tracing for the specified peripheral')
    #parser.add_argument('--trace-all', help='enable tracing for all peripherals')
    #parser.add_argument('--trace-cpu', help='enable CPU tracing')
    args = parser.parse_args()

    global s
    s = SoC()

    try:
        if args.flash:          s.spi0.flash.load(args.flash)
        if args.uart0_tty:      s.uart0.attach_tty(args.uart0_tty)
        else:                   s.uart0.attach_tty('/dev/tty')
        if args.uart1_tty:      s.uart1.attach_tty(args.uart1_tty)

        #s.uart0.enable_trace = True
        #s.cpu.enable_trace = True

        s.simulate_bootrom()
        while True:
            s.simulate()

    finally:
        s.uart0.detach_tty()
        s.uart1.detach_tty()

if __name__ == '__main__':
    main()
