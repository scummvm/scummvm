# Android specific build targets
PATH_DIST = $(srcdir)/dists/android

GRADLE_FILES = $(shell find $(PATH_DIST)/gradle -type f)

PATH_BUILD = ./android_project
PATH_BUILD_GRADLE = $(PATH_BUILD)/gradle/.timestamp $(PATH_BUILD)/gradlew $(PATH_BUILD)/build.gradle $(PATH_BUILD)/settings.gradle $(PATH_BUILD)/mainAssets/build.gradle $(PATH_BUILD)/gradle.properties $(PATH_BUILD)/local.properties $(PATH_BUILD)/src.properties
# $(PATH_BUILD)/mainAssets/src/main/assets is the root and everything is placed in the assets subdirectory
PATH_BUILD_ASSETS = $(PATH_BUILD)/mainAssets/src/main/assets
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
	$(INSTALL) -c -m 755 $< $@

$(PATH_BUILD)/build.gradle: $(PATH_DIST)/build.gradle | $(PATH_BUILD)
	$(INSTALL) -c -m 644 $< $@

$(PATH_BUILD)/settings.gradle: $(PATH_DIST)/settings.gradle | $(PATH_BUILD)
	$(INSTALL) -c -m 644 $< $@

$(PATH_BUILD)/gradle.properties: $(PATH_DIST)/gradle.properties | $(PATH_BUILD)
	$(INSTALL) -c -m 644 $< $@

$(PATH_BUILD)/local.properties: configure.stamp | $(PATH_BUILD)
	$(ECHO) "sdk.dir=$(realpath $(ANDROID_SDK_ROOT))\n" > $(PATH_BUILD)/local.properties

$(PATH_BUILD)/src.properties: configure.stamp | $(PATH_BUILD)
	$(ECHO) "srcdir=$(realpath $(srcdir))\n" > $(PATH_BUILD)/src.properties

$(PATH_BUILD)/mainAssets/build.gradle: $(PATH_DIST)/mainAssets.gradle | $(PATH_BUILD_ASSETS)/MD5SUMS
	$(INSTALL) -c -m 644 $< $@

$(PATH_BUILD_ASSETS)/MD5SUMS: $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) $(DIST_FILES_SOUNDFONTS) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) $(DIST_FILES_PLATFORM) $(DIST_FILES_SHADERS) | $(PATH_BUILD)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/assets/
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) $(DIST_FILES_SOUNDFONTS) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) $(DIST_FILES_PLATFORM) $(PATH_BUILD_ASSETS)/assets/
ifneq ($(DIST_FILES_SHADERS),)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/assets/shaders
	$(INSTALL) -c -m 644 $(DIST_FILES_SHADERS) $(PATH_BUILD_ASSETS)/assets/shaders
endif
ifneq ($(GAMES_BUNDLE_DIRECTORY),)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/assets/games
	$(CP) -r $(GAMES_BUNDLE_DIRECTORY)/. $(PATH_BUILD_ASSETS)/assets/games/
endif
	(cd $(PATH_BUILD_ASSETS)/ && find assets -type f | sort | xargs md5sum) > $@

ifdef DIST_ANDROID_CACERT_PEM
$(PATH_BUILD_ASSETS)/assets/cacert.pem: $(DIST_ANDROID_CACERT_PEM)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/assets/
	$(INSTALL) -c -m 644 $(DIST_ANDROID_CACERT_PEM) $(PATH_BUILD_ASSETS)/assets/cacert.pem
$(PATH_BUILD_ASSETS)/MD5SUMS: $(PATH_BUILD_ASSETS)/assets/cacert.pem
else
ifdef USE_CURL
$(PATH_BUILD_ASSETS)/assets/cacert.pem:
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/assets/
	$(QUIET_CURL)$(CURL) -s https://curl.se/ca/cacert.pem --time-cond $(PATH_BUILD_ASSETS)/assets/cacert.pem --output $(PATH_BUILD_ASSETS)/assets/cacert.pem
androidcacert:
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/assets/
	$(QUIET_CURL)$(CURL) -s https://curl.se/ca/cacert.pem --time-cond $(PATH_BUILD_ASSETS)/assets/cacert.pem --output $(PATH_BUILD_ASSETS)/assets/cacert.pem

.PHONY: androidcacert
endif
endif
# Make MD5SUMS depend on cacert. If it's not here and neither DIST_ANDROID_CACERT_PEM nor USE_CURL are defined, it will error out
$(PATH_BUILD_ASSETS)/MD5SUMS: $(PATH_BUILD_ASSETS)/assets/cacert.pem

$(PATH_BUILD_LIBSCUMMVM): libscummvm.so | $(PATH_BUILD)
	$(INSTALL) -d  $(PATH_BUILD_LIB)
	$(INSTALL) -c -m 644 libscummvm.so $(PATH_BUILD_LIBSCUMMVM)

$(APK_MAIN): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS)/MD5SUMS $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew assembleDebug)
	$(CP) $(PATH_BUILD)/build/outputs/apk/debug/$(APK_MAIN) $@

$(APK_MAIN_RELEASE): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS)/MD5SUMS $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew assembleRelease)
	$(CP) $(PATH_BUILD)/build/outputs/apk/release/$(APK_MAIN_RELEASE) $@

$(AAB_MAIN_RELEASE): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS)/MD5SUMS $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew bundleRelease -PsplitAssets)
	$(CP) $(PATH_BUILD)/build/outputs/bundle/release/$(AAB_MAIN_RELEASE) $@

clean: androidclean

# SUBPATH_BUILDS is set in fatbundle.mk
androidclean:
	$(RM) -rf $(PATH_BUILD) $(SUBPATH_BUILDS) *.apk *.aab

all: $(APK_MAIN)

androidrelease: $(APK_MAIN_RELEASE)
androidbundlerelease: $(AAB_MAIN_RELEASE)

androidtest: $(APK_MAIN)
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

ANDROID_BUILD_RULES := androidrelease androidbundlerelease androidtest androiddistdebug androiddistrelease
.PHONY: androidclean $(ANDROID_BUILD_RULES)

include $(srcdir)/backends/platform/android/fatbundle.mk
