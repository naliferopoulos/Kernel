#! /bin/bash
qemu-system-i386 -kernel ./bin/kernel -s -S&
gdb
