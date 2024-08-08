# Android specific build targets
PATH_DIST = $(srcdir)/dists/android

GRADLE_FILES = $(shell find $(PATH_DIST)/gradle -type f)

PATH_BUILD = ./android_project
PATH_BUILD_GRADLE = $(PATH_BUILD)/gradle/.timestamp $(PATH_BUILD)/gradlew $(PATH_BUILD)/build.gradle $(PATH_BUILD)/gradle.properties $(PATH_BUILD)/local.properties $(PATH_BUILD)/src.properties
PATH_BUILD_ASSETS = $(PATH_BUILD)/assets
PATH_BUILD_LIB = $(PATH_BUILD)/lib/$(ABI)
PATH_BUILD_LIBSCUMMVM = $(PATH_BUILD)/lib/$(ABI)/libscummvm.so

APK_MAIN = ScummVM-debug.apk
APK_MAIN_RELEASE = ScummVM-release-unsigned.apk
AAB_MAIN_RELEASE = ScummVM-release.aab

DIST_FILES_PLATFORM = $(PATH_DIST)/android-help.zip $(PATH_DIST)/gamepad.svg

$(PATH_BUILD):
	$(MKDIR) $(PATH_BUILD)

$(PATH_BUILD)/gradle/.timestamp: $(GRADLE_FILES) | $(PATH_BUILD)
	$(MKDIR) $(PATH_BUILD)/gradle
	$(CP) -r $(PATH_DIST)/gradle/. $(PATH_BUILD)/gradle/
	touch "$@"

$(PATH_BUILD)/gradlew: $(PATH_DIST)/gradlew | $(PATH_BUILD)
	$(INSTALL) -c -m 755 $(PATH_DIST)/gradlew $(PATH_BUILD)

$(PATH_BUILD)/build.gradle: $(PATH_DIST)/build.gradle | $(PATH_BUILD)
	$(INSTALL) -c -m 644 $(PATH_DIST)/build.gradle $(PATH_BUILD)

$(PATH_BUILD)/gradle.properties: $(PATH_DIST)/gradle.properties | $(PATH_BUILD)
	$(INSTALL) -c -m 644 $(PATH_DIST)/gradle.properties $(PATH_BUILD)

$(PATH_BUILD)/local.properties: configure.stamp | $(PATH_BUILD)
	$(ECHO) "sdk.dir=$(realpath $(ANDROID_SDK_ROOT))\n" > $(PATH_BUILD)/local.properties

$(PATH_BUILD)/src.properties: configure.stamp | $(PATH_BUILD)
	$(ECHO) "srcdir=$(realpath $(srcdir))\n" > $(PATH_BUILD)/src.properties

$(PATH_BUILD_ASSETS): $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) $(DIST_FILES_SOUNDFONTS) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) $(DIST_FILES_PLATFORM) | $(PATH_BUILD)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) $(DIST_FILES_SOUNDFONTS) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) $(DIST_FILES_PLATFORM) $(PATH_BUILD_ASSETS)/
ifneq ($(DIST_FILES_SHADERS),)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/shaders
	$(INSTALL) -c -m 644 $(DIST_FILES_SHADERS) $(PATH_BUILD_ASSETS)/shaders
endif

ifdef DIST_ANDROID_CACERT_PEM
$(PATH_BUILD_ASSETS)/cacert.pem: $(DIST_ANDROID_CACERT_PEM) | $(PATH_BUILD_ASSETS)
	$(INSTALL) -c -m 644 $(DIST_ANDROID_CACERT_PEM) $(PATH_BUILD_ASSETS)/cacert.pem
else
ifdef USE_CURL
$(PATH_BUILD_ASSETS)/cacert.pem: | $(PATH_BUILD_ASSETS)
	$(QUIET_CURL)$(CURL) -s https://curl.se/ca/cacert.pem --time-cond $(PATH_BUILD_ASSETS)/cacert.pem --output $(PATH_BUILD_ASSETS)/cacert.pem
androidcacert: | $(PATH_BUILD_ASSETS)
	$(QUIET_CURL)$(CURL) -s https://curl.se/ca/cacert.pem --time-cond $(PATH_BUILD_ASSETS)/cacert.pem --output $(PATH_BUILD_ASSETS)/cacert.pem
.PHONY: androidcacert
endif
endif

$(PATH_BUILD_LIBSCUMMVM): libscummvm.so | $(PATH_BUILD)
	$(INSTALL) -d  $(PATH_BUILD_LIB)
	$(INSTALL) -c -m 644 libscummvm.so $(PATH_BUILD_LIBSCUMMVM)

$(APK_MAIN): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_ASSETS)/cacert.pem $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew assembleDebug)
	$(CP) $(PATH_BUILD)/build/outputs/apk/debug/$(APK_MAIN) $@

$(APK_MAIN_RELEASE): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_ASSETS)/cacert.pem $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew assembleRelease)
	$(CP) $(PATH_BUILD)/build/outputs/apk/release/$(APK_MAIN_RELEASE) $@

$(AAB_MAIN_RELEASE): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_ASSETS)/cacert.pem $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew bundleRelease)
	$(CP) $(PATH_BUILD)/build/outputs/bundle/release/$(AAB_MAIN_RELEASE) $@

all: $(APK_MAIN)

clean: androidclean

androidclean:
	@$(RM) -rf $(PATH_BUILD) *.apk

androidrelease: $(APK_MAIN_RELEASE)
androidbundlerelease: $(AAB_MAIN_RELEASE)

androidtestmain: $(APK_MAIN)
	(cd $(PATH_BUILD); ./gradlew installDebug)
	# $(ADB) install -g -r $(APK_MAIN)
	# $(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.scummvm.scummvm/.ScummVMActivity

androidtest: $(APK_MAIN)
	# @set -e; for apk in $^; do \
	# 	$(ADB) install -g -r $$apk; \
	# done
	# $(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.scummvm.scummvm/.ScummVMActivity
	(cd $(PATH_BUILD); ./gradlew installDebug)

# used by buildbot!
androiddistdebug: all
	$(MKDIR) debug
	$(CP) $(APK_MAIN) debug/
	for i in $(DIST_FILES_DOCS); do \
		sed 's/$$/\r/' < $$i > debug/`basename $$i`.txt; \
	done

androiddistrelease: androidrelease
	$(MKDIR) release
	$(CP) $(APK_MAIN_RELEASE) release/
	for i in $(DIST_FILES_DOCS); do \
		sed 's/$$/\r/' < $$i > release/`basename $$i`.txt; \
	done

.PHONY: androidrelease androidbundlerelease androidtest $(PATH_BUILD_SRC)
