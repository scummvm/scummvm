# devtools/create_buried/module.mk

dists/engine-data/buried_subtitles.dat: $(srcdir)/devtools/create_buried/subtitles.json $(srcdir)/devtools/create_buried/create_buried_subtitles.py
	$(PYTHON) $(srcdir)/devtools/create_buried/create_buried_subtitles.py $< $@

buried_subtitles.dat: dists/engine-data/buried_subtitles.dat

$(srcdir)/engines/buried/po/buried_subtitles.pot: $(srcdir)/devtools/create_buried/subtitles.json $(srcdir)/devtools/create_buried/create_buried_pot.py
	$(PYTHON) $(srcdir)/devtools/create_buried/create_buried_pot.py $< $@

buried_subtitles.pot: $(srcdir)/engines/buried/po/buried_subtitles.pot

.PHONY: buried_subtitles.dat buried_subtitles.pot
