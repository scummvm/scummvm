hollywood.dat: devtools/create_translations
	python3 $(srcdir)/devtools/extract_hollywood_translation.py pack \
		$(srcdir)/devtools/create_hollywood/files \
		$(srcdir)/dists/engine-data/hollywood.dat \
		--po $(srcdir)/engines/hollywood/monsters/po/en.po \
		--compiler devtools/create_translations/create_translations$(EXEEXT)

.PHONY: hollywood.dat
