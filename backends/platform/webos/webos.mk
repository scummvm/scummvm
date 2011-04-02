# WebOS specific build targets

PATH_DIST = $(srcdir)/dists/webos
PATH_RESOURCES = $(PATH_DIST)/mojo

STAGING_DIR=STAGING/org.scummvm

webosrelease:
	rm -rf $(STAGING_DIR)
	rm -f *.ipk
	mkdir -p $(STAGING_DIR)
	mkdir -p $(STAGING_DIR)/bin
	mkdir -p $(STAGING_DIR)/lib
	cp -f $(PATH_RESOURCES)/* $(STAGING_DIR)
	cp -f scummvm $(STAGING_DIR)/bin
	$(WEBOS_PDK)/arm-gcc/bin/arm-none-linux-gnueabi-strip $(STAGING_DIR)/bin/scummvm
	$(WEBOS_SDK)/bin/palm-package $(STAGING_DIR)
	rm -rf STAGING

.PHONY: webosrelease
