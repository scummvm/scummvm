harold.dat:
	cd $(srcdir)/devtools/create_harold/files && zip -FS -r9 $(abspath $(srcdir)/dists/engine-data/harold.dat) .

.PHONY: harold.dat
