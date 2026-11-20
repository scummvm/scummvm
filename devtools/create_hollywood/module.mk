MODULE := devtools/create_hollywood

MODULE_OBJS := create_hollywood.o
TOOL_EXECUTABLE := create_hollywood
TOOL_DEPS := devtools/create_translations/po_parser.o

include $(srcdir)/rules.mk

hollywood.dat: devtools/create_translations devtools/create_hollywood
	python3 $(srcdir)/devtools/extract_hollywood_translation.py pack \
		$(srcdir)/devtools/create_hollywood/files \
		$(srcdir)/dists/engine-data/hollywood.dat \
		--po $(wildcard $(srcdir)/engines/hollywood/monsters/po/*.po) \
		--runaway-po $(wildcard $(srcdir)/engines/hollywood/runaway/po/*.po) \
		--runaway-compiler devtools/create_hollywood/create_hollywood$(EXEEXT) \
		--compiler devtools/create_translations/create_translations$(EXEEXT)

.PHONY: hollywood.dat
