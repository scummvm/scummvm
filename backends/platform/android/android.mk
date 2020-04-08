# Android specific build targets

PATH_DIST = $(srcdir)/dists/android

PORT_DISTFILES = $(PATH_DIST)/README.Android

GRADLE_FILES = $(shell find $(PATH_DIST)/gradle -type f) $(PATH_DIST)/gradlew $(PATH_DIST)/build.gradle

PATH_BUILD = ./android_project
PATH_BUILD_GRADLE = $(PATH_BUILD)/build.gradle
PATH_BUILD_ASSETS = $(PATH_BUILD)/assets
PATH_BUILD_LIBSCUMMVM = $(PATH_BUILD)/lib/$(ABI)/libscummvm.so

APK_MAIN = ScummVM-debug.apk
APK_MAIN_RELEASE = ScummVM-release-unsigned.apk

$(PATH_BUILD):
	$(MKDIR) $(PATH_BUILD)

$(PATH_BUILD_GRADLE): $(GRADLE_FILES) | $(PATH_BUILD)
	$(CP) -r $(PATH_DIST)/gradle/ $(PATH_BUILD)
	$(INSTALL) -c -m 755 $(PATH_DIST)/gradlew $(PATH_BUILD)
	$(INSTALL) -c -m 644 $(PATH_DIST)/build.gradle $(PATH_BUILD)
	$(ECHO) "srcdir=$(realpath $(srcdir))" > $(PATH_BUILD)/gradle.properties
	$(ECHO) "sdk.dir=$(realpath $(ANDROID_SDK_ROOT))\n" > $(PATH_BUILD)/local.properties
	$(ECHO) "ndk.dir=$(realpath $(ANDROID_NDK_ROOT))\n" >> $(PATH_BUILD)/local.properties

$(PATH_BUILD_ASSETS): $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) $(PORT_DISTFILES) | $(PATH_BUILD)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) $(PORT_DISTFILES) $(PATH_BUILD_ASSETS)/

$(PATH_BUILD_LIBSCUMMVM): libscummvm.so | $(PATH_BUILD)
	$(INSTALL) -D -c -m 644 libscummvm.so $(PATH_BUILD_LIBSCUMMVM)

$(APK_MAIN): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew assembleDebug)
	$(CP) $(PATH_BUILD)/build/outputs/apk/debug/ScummVM-debug.apk $@

$(APK_MAIN_RELEASE): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew build)
	$(CP) $(PATH_BUILD)/build/outputs/apk/release/ScummVM-release-unsigned.apk $@

all: $(APK_MAIN)

clean: androidclean

androidclean:
	@$(RM) -rf $(PATH_BUILD) *.apk

androidrelease: $(APK_MAIN_RELEASE)

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
	for i in $(DIST_FILES_DOCS) $(PORT_DISTFILES); do \
		sed 's/$$/\r/' < $$i > debug/`basename $$i`.txt; \
	done

androiddistrelease: androidrelease
	$(MKDIR) release
	$(CP) $(APK_MAIN_RELEASE) release/
	for i in $(DIST_FILES_DOCS) $(PORT_DISTFILES); do \
		sed 's/$$/\r/' < $$i > release/`basename $$i`.txt; \
	done

.PHONY: androidrelease androidtest $(PATH_BUILD_SRC)
