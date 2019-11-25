# Android specific build targets

# These must be incremented for each market upload
ANDROID_VERSIONCODE = 40

# Historical version codes:
# ScummVM 2.1.0: 35-39 (armeabi, arm-v7a, arm64-v8a, x86, x86_64 respectively)
# ScummVM 2.0.0: 30-34
# ScummVM 1.9.0.1: 25-28
# ScummVM 1.9.0: 19
# ScummVM 1.8.1: 15

ANDROID_TARGET_VERSION = 28

# ndk-build will build the ScummVM library in release mode by default unless:
# - an Application.mk is provided in the jni folder with APP_OPTIM := debug
# - or AndroidManifest.xml declares android:debuggable within its <application> tag
NDK_BUILD = $(ANDROID_NDK)/ndk-build APP_ABI=$(ABI)
SDK_ANDROID = $(ANDROID_SDK)/tools/android

PATH_DIST = $(srcdir)/dists/android
PATH_RESOURCES = $(PATH_DIST)/res

PORT_DISTFILES = $(PATH_DIST)/README.Android
DIST_JAVA_SRC_DIR = $(srcdir)/backends/platform/android/org

RESOURCES = \
	$(PATH_BUILD_RES)/values/strings.xml \
	$(PATH_BUILD_RES)/values-television/margins.xml \
	$(PATH_BUILD_RES)/layout/main.xml \
	$(PATH_BUILD_RES)/drawable/scummvm.png \
	$(PATH_BUILD_RES)/drawable/scummvm_big.png \
	$(PATH_BUILD_RES)/drawable-xhdpi/leanback_icon.png \
	$(PATH_BUILD_RES)/drawable-xhdpi/ouya_icon.png \
	$(PATH_BUILD_RES)/drawable-hdpi/ic_action_keyboard.png \
	$(PATH_BUILD_RES)/drawable-mdpi/ic_action_keyboard.png \
	$(PATH_BUILD_RES)/drawable-xhdpi/ic_action_keyboard.png \
	$(PATH_BUILD_RES)/drawable-xxhdpi/ic_action_keyboard.png

DIST_ANDROID_MK = $(PATH_DIST)/jni/Android.mk
DIST_BUILD_XML = $(PATH_DIST)/custom_rules.xml

PATH_BUILD = ./build.tmp
PATH_BUILD_ASSETS = $(PATH_BUILD)/assets
PATH_BUILD_RES = $(PATH_BUILD)/res
PATH_BUILD_LIBSCUMMVM = $(PATH_BUILD)/lib/$(ABI)/libscummvm.so

FILE_MANIFEST_SRC = $(srcdir)/dists/android/AndroidManifest.xml
FILE_MANIFEST = $(PATH_BUILD)/AndroidManifest.xml

APK_MAIN = ScummVM-debug.apk
APK_MAIN_RELEASE = ScummVM-release-unsigned.apk

$(FILE_MANIFEST): $(FILE_MANIFEST_SRC) | $(PATH_BUILD)
	@$(MKDIR) -p $(@D)
	sed "s/@ANDROID_VERSIONCODE@/$(ANDROID_VERSIONCODE)/" < $< > $@

$(PATH_BUILD)/res/%: $(PATH_DIST)/res/% | $(PATH_BUILD)
	@$(MKDIR) -p $(@D)
	$(CP) $< $@

$(PATH_BUILD)/libs/%: $(PATH_DIST)/libs/% | $(PATH_BUILD)
	@$(MKDIR) -p $(@D)
	$(CP) $< $@

$(PATH_BUILD_ASSETS): $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_BUILD_XML) $(DIST_FILES_DOCS) $(PORT_DISTFILES) | $(PATH_BUILD)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) $(PORT_DISTFILES) $(PATH_BUILD_ASSETS)/
	$(INSTALL) -d $(PATH_BUILD)/jni
	$(INSTALL) -c -m 644 $(DIST_ANDROID_MK) $(PATH_BUILD)/jni
	$(INSTALL) -c -m 644 $(DIST_BUILD_XML) $(PATH_BUILD)

$(PATH_BUILD): $(DIST_ANDROID_MK)
	$(MKDIR) -p $(PATH_BUILD) $(PATH_BUILD)/res
	$(MKDIR) -p $(PATH_BUILD)/libs

$(PATH_BUILD_LIBSCUMMVM): libscummvm.so | $(PATH_BUILD)
	$(INSTALL) -c -m 644 libscummvm.so $(PATH_BUILD)
	$(STRIP) $(PATH_BUILD)/libscummvm.so
	cd $(PATH_BUILD); $(NDK_BUILD)

$(PATH_BUILD_RES): $(RESOURCES) | $(PATH_BUILD)

setupapk: $(FILE_MANIFEST) $(PATH_BUILD_RES) $(PATH_BUILD_ASSETS) $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	$(SDK_ANDROID) update project -p $(PATH_BUILD) -t android-$(ANDROID_TARGET_VERSION) -n ScummVM

$(APK_MAIN): setupapk | $(PATH_BUILD)
	(cd $(PATH_BUILD); ant debug -Dsource.dir="$(realpath $(DIST_JAVA_SRC_DIR))")
	$(CP) $(PATH_BUILD)/bin/ScummVM-debug.apk $@

$(APK_MAIN_RELEASE): setupapk | $(PATH_BUILD)
	(cd $(PATH_BUILD); ant release -Dsource.dir="$(realpath $(DIST_JAVA_SRC_DIR))")
	$(CP) $(PATH_BUILD)/bin/ScummVM-release-unsigned.apk $@

all: $(APK_MAIN)

clean: androidclean

androidclean:
	@$(RM) -rf $(PATH_BUILD) *.apk release debug

androidrelease: $(APK_MAIN_RELEASE)

androidtestmain: $(APK_MAIN)
	$(ADB) install -r $(APK_MAIN)
	$(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.scummvm.scummvm/.ScummVMActivity

androidtest: $(APK_MAIN)
	@set -e; for apk in $^; do \
		$(ADB) install -r $$apk; \
	done
	$(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.scummvm.scummvm/.ScummVMActivity

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
