# $URL$
# $Id$

ronindir = /usr/local/ronin

CC := $(CXX)
ASFLAGS := $(CXXFLAGS)

SCUMMVM.BIN : scummvm.bin
	scramble $< $@

scummvm.bin : scummvm.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@


