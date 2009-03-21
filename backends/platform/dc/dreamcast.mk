# $URL$
# $Id$

ronindir = /usr/local/ronin

CC := $(CXX)
ASFLAGS := $(CXXFLAGS)


dist : SCUMMVM.BIN plugins plugin_dist

plugin_dist :
	for p in plugins/*.plg; do \
	  if [ x'plugins/*.plg' != x"$$p" ]; then sh-elf-strip -g -o "`basename \"$$p\" | LC_CTYPE=C tr '[:lower:]' '[:upper:]'`" "$$p"; fi ; \
	done

SCUMMVM.BIN : scummvm.bin
	scramble $< $@

scummvm.bin : scummvm.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@


