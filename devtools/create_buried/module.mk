# devtools/create_buried/module.mk

dists/engine-data/buried_subtitles.dat: $(srcdir)/devtools/create_buried/subtitles.json $(srcdir)/devtools/create_buried/create_buried_subtitles.py
	$(PYTHON) $(srcdir)/devtools/create_buried/create_buried_subtitles.py $< $@

buried_subtitles.dat: dists/engine-data/buried_subtitles.dat

.PHONY: buried_subtitles.dat
