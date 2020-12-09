MODULE := engines/hadesch

MODULE_OBJS = \
    metaengine.o \
    pod_file.o \
    tag_file.o \
    pod_image.o \
    video.o \
    hadesch.o \
    baptr.o \
    rooms/olympus.o \
    rooms/walloffame.o \
    rooms/argo.o \
    rooms/crete.o \
    rooms/minos.o \
    rooms/daedalus.o \
    rooms/seriphos.o \
    rooms/medisle.o \
    rooms/troy.o \
    rooms/quiz.o \
    rooms/minotaur.o \
    rooms/catacombs.o \
    rooms/priam.o \
    rooms/athena.o \
    rooms/volcano.o \
    rooms/riverstyx.o \
    rooms/hadesthrone.o \
    rooms/credits.o \
    rooms/intro.o \
    rooms/ferry.o \
    rooms/options.o \
    rooms/monster.o \
    rooms/monster/projectile.o \
    rooms/monster/typhoon.o \
    rooms/monster/cyclops.o \
    rooms/monster/illusion.o \
    rooms/medusa.o \
    rooms/trojan.o \
    gfx_context.o \
    ambient.o \
    herobelt.o \
    hotzone.o \
    table.o \
    persistent.o

DETECT_OBJS += $(MODULE)/detection.o

# This module can be built as a plugin
ifeq ($(ENABLE_HADESCH), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

ifneq "$(HADESCH_RULES_INCLUDED)" "1"

HADESCH_RULES_INCLUDED := 1
HADESCH_POTFILE := $(srcdir)/engines/hadesch/po/hadesch.pot
HADESCH_POFILES := $(wildcard $(srcdir)/engines/hadesch/po/*.po)

hadesch-updatepot:
	cat $(srcdir)/engines/hadesch/po/POTFILES_hadesch | \
	xgettext -f - -D $(srcdir) -d hadesch --c++ -k_ -k_s -k_c:1,2c -k_sc:1,2c -kTranscribedSound:2 --add-comments=I18N\
		-kDECLARE_TRANSLATION_ADDITIONAL_CONTEXT:1,2c -o $(HADESCH_POTFILE) \
		--copyright-holder="ScummVM Team" --package-name=ScummVM \
		--package-version=$(VERSION) --msgid-bugs-address=scummvm-devel@lists.scummvm.org -o $(HADESCH_POTFILE)_

	sed -e 's/SOME DESCRIPTIVE TITLE/LANGUAGE translation for ScummVM/' \
		-e 's/UTF-8/CHARSET/' -e 's/PACKAGE/ScummVM/' $(HADESCH_POTFILE)_ > $(HADESCH_POTFILE).new

	rm $(HADESCH_POTFILE)_
	if test -f $(HADESCH_POTFILE); then \
		sed -f $(srcdir)/po/remove-potcdate.sed < $(HADESCH_POTFILE) > $(HADESCH_POTFILE).1 && \
		sed -f $(srcdir)/po/remove-potcdate.sed < $(HADESCH_POTFILE).new > $(HADESCH_POTFILE).2 && \
		if cmp $(HADESCH_POTFILE).1 $(HADESCH_POTFILE).2 >/dev/null 2>&1; then \
			rm -f $(HADESCH_POTFILE).new; \
		else \
			rm -f $(HADESCH_POTFILE) && \
			mv -f $(HADESCH_POTFILE).new $(HADESCH_POTFILE); \
		fi; \
		rm -f $(HADESCH_POTFILE).1 $(HADESCH_POTFILE).2; \
	else \
		mv -f $(HADESCH_POTFILE).new $(HADESCH_POTFILE); \
	fi;

engines/hadesch/po/%.po: $(HADESCH_POTFILE)
	msgmerge $@ $(HADESCH_POTFILE) -o $@.new
	if cmp $@ $@.new >/dev/null 2>&1; then \
		rm -f $@.new; \
	else \
		mv -f $@.new $@; \
	fi;

hadesch-translations-dat: devtools/create_translations
	devtools/create_translations/create_translations hadesch_translations.dat $(HADESCH_POFILES)
	mv hadesch_translations.dat $(srcdir)/dists/engine-data/hadesch_translations.dat

update-hadesch-translations: hadesch-updatepot $(HADESCH_POFILES) hadesch-translations-dat

update-hadesch-translations: hadesch-updatepot $(HADESCH_POFILES)
	@$(foreach file, $(HADESCH_POFILES), echo -n $(notdir $(basename $(file)))": ";msgfmt --statistic $(file);)
	@rm -f messages.mo

.PHONY: updatehadeschpot hadesch-translations-dat update-hadesch-translations
endif # HADESCH_RULES_INCLUDED
