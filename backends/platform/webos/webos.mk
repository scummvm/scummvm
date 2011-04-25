# WebOS specific build targets
# ============================================================================
#
# Run "make webosrelease" to create a release package to be uploaded to the
# Palm app catalog.
#
# Run "make webosbeta" to create a beta package to be uploaded to the Palm
# Beta app catalog.
#
# Before calling these targets the ScummVM source must be configured for a
# WebOS build.  See
# http://wiki.scummvm.org/index.php/Compiling_ScummVM/WebOS for details.
#
# VER_PACKAGE must be set to a number which is higher than the currently
# used package version in the app catalog.  So when creating an updated
# package for ScummVM 1.3.9 and the current ScummVM package in the app
# catalog is version 1.3.0902 then you must specify VER_PACKAGE=3 to create
# the ScummVM package with version 1.3.0903.  Yeah, I know that's ugly but
# WebOS package version numbers are restricted to three numeric components.
#
# As long as Palm doesn't support Team-maintained apps the uploaded packages
# MUST NOT be packaged with the default "org.scummvm" base id.  Instead apps
# must be uploaded with a user-specific base id.  A good practice is using
# the github user as base id: com.github.<username>.  It is also necessary
# to use a user-specific app name when submitting the created package to the
# Palm app catalog.  Use "ScummVM (<username>)" instead of "ScummVM" and
# "ScummVM Beta (<username>)" instead of "ScummVM Beta".

PATH_DIST = $(srcdir)/dists/webos
PATH_MOJO = $(PATH_DIST)/mojo
BASE_APP_ID = org.scummvm
APP_ID = $(BASE_APP_ID).scummvm
BETA_APP_ID = $(APP_ID)-beta
APP_VERSION = $(shell printf "%d.%d.%02d%02d" $(VER_MAJOR) $(VER_MINOR) $(VER_PATCH) $(VER_PACKAGE))
STAGING_DIR=STAGING/$(APP_ID)

webosprepare: all
	$(QUIET)if [ "$(VER_PACKAGE)" = "" ]; \
	then \
	    echo "ERROR: VER_PACKAGE is not set"; \
            echo "Example: export VER_PACKAGE=1"; \
	    exit 1; \
	fi
	$(QUIET)$(RM_REC) $(STAGING_DIR)
	$(QUIET)$(MKDIR) $(STAGING_DIR)
	$(QUIET)$(MKDIR) $(STAGING_DIR)/bin
	$(QUIET)$(MKDIR) $(STAGING_DIR)/lib
	$(QUIET)$(MKDIR) $(STAGING_DIR)/share/scummvm
	$(QUIET)$(CP) $(PATH_MOJO)/* $(STAGING_DIR)
	$(QUIET)$(CP) gui/themes/translations.dat $(STAGING_DIR)/share/scummvm
	$(QUIET)$(CP) gui/themes/scummmodern.zip $(STAGING_DIR)/share/scummvm
	$(QUIET)$(CP) scummvm $(STAGING_DIR)/bin
	$(QUIET)$(STRIP) $(STAGING_DIR)/bin/scummvm
	$(QUIET)sed -i s/'APP_VERSION'/'$(APP_VERSION)'/ $(STAGING_DIR)/appinfo.json

webosrelease: webosprepare
	$(QUIET)$(RM) $(APP_ID)_*.ipk
	$(QUIET)sed -i s/'APP_ID'/'$(APP_ID)'/ $(STAGING_DIR)/appinfo.json
	$(QUIET)sed -i s/'APP_TITLE'/'ScummVM'/ $(STAGING_DIR)/appinfo.json
	$(QUIET)$(WEBOS_SDK)/bin/palm-package --use-v1-format $(STAGING_DIR)
	$(QUIET)$(RM_REC) STAGING

webosbeta: webosprepare
	$(QUIET)$(RM) $(BETA_APP_ID)_*.ipk
	$(QUIET)sed -i s/'APP_ID'/'$(BETA_APP_ID)'/ $(STAGING_DIR)/appinfo.json
	$(QUIET)sed -i s/'APP_TITLE'/'ScummVM Beta'/ $(STAGING_DIR)/appinfo.json
	$(QUIET)$(WEBOS_SDK)/bin/palm-package --use-v1-format $(STAGING_DIR)
	$(QUIET)$(RM_REC) STAGING

.PHONY: webosrelease webosbeta
