# $URL$
# $Id$

ronindir = /usr/local/ronin

CC := $(CXX)
ASFLAGS := $(CXXFLAGS)


dist : SCUMMVM.BIN plugin_dist

plugin_dist : plugins
	@for p in $(PLUGINS); do \
	  t="`basename \"$$p\" | LC_CTYPE=C tr '[:lower:]' '[:upper:]'`"; \
	  if /usr/bin/test "$$p" -ot "$$t"; then :; else \
	    echo sh-elf-strip -g -o "$$t" "$$p"; \
	    sh-elf-strip -g -o "$$t" "$$p"; \
          fi;\
	done

SCUMMVM.BIN : scummvm.bin
	scramble $< $@

scummvm.bin : scummvm.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@


