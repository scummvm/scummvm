# WebOS specific build targets

PATH_DIST = $(srcdir)/dists/webos
PATH_MOJO = $(PATH_DIST)/mojo
PATH_THEMES = $(PATH_DIST)/themes

STAGING_DIR=STAGING/org.scummvm.scummvm

webosrelease:
	rm -rf $(STAGING_DIR)
	rm -f *.ipk
	mkdir -p $(STAGING_DIR)
	mkdir -p $(STAGING_DIR)/bin
	mkdir -p $(STAGING_DIR)/lib
	mkdir -p $(STAGING_DIR)/share/scummvm
	cp -f $(PATH_MOJO)/* $(STAGING_DIR)
	cp -f $(PATH_THEMES)/* $(STAGING_DIR)/share/scummvm
	cp -f gui/themes/translations.dat $(STAGING_DIR)/share/scummvm
	cp -f gui/themes/scummmodern.zip $(STAGING_DIR)/share/scummvm
	cp -f scummvm $(STAGING_DIR)/bin
	$(WEBOS_PDK)/arm-gcc/bin/arm-none-linux-gnueabi-strip $(STAGING_DIR)/bin/scummvm
	$(WEBOS_SDK)/bin/palm-package $(STAGING_DIR)
	rm -rf STAGING

.PHONY: webosrelease
