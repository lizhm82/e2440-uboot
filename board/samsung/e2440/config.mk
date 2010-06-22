#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#

#
# E2440 has 1 bank of 64 MB DRAM
#
# 3000'0000 to 3400'0000
#
# Linux-Kernel is expected to be at 3000'8000, entry 3000'8000
# optionally with a ramdisk at 3080'0000
#
# we load ourself to 33F8'0000
#
# download area is 3300'0000
#


TEXT_BASE = 0x33F80000
