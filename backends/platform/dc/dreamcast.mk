# $URL$
# $Id$

ronindir = /usr/local/ronin

CC := $(CXX)
ASFLAGS := $(CXXFLAGS)


dist : SCUMMVM.BIN plugin_dist

plugin_dist : $(PLUGINS)
	for p in $(PLUGINS); do \
	  sh-elf-strip -g -o "`basename \"$$p\" | LC_CTYPE=C tr '[:lower:]' '[:upper:]'`" "$$p"; \
	done

SCUMMVM.BIN : scummvm.bin
	scramble $< $@

scummvm.bin : scummvm.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@


