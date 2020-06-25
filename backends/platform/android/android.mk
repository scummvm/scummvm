# Android specific build targets

# These must be incremented for each market upload
ANDROID_VERSIONCODE ?= 50000
ANDROID_VERSIONNAME ?= 0.4

PATH_DIST = $(srcdir)/dists/android

PORT_DISTFILES = $(PATH_DIST)/README.Android
DIST_ANDROID_CONTROLS = $(PATH_DIST)/assets/arrows.tga
GRADLE = $(PATH_DIST)/gradlew


PATH_BUILD = ./android
PATH_BUILD_ASSETS = $(PATH_BUILD)/assets
PATH_BUILD_JNI = $(PATH_BUILD)/jni/$(ABI)/libresidualvm.so
PATH_BUILD_GRADLE = $(PATH_BUILD)/settings.gradle
PATH_BUILD_SETUPAPK = $(PATH_BUILD)/.setupapk

APK_MAIN = $(PATH_BUILD)/build/outputs/apk/debug/ResidualVM-debug.apk
APK_MAIN_RELEASE = $(PATH_BUILD)/build/outputs/apk/release/ResidualVM-release$(if $(KEYSTORE),,-unsigned).apk

$(PATH_BUILD_ASSETS): $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_SHADERS) $(DIST_ANDROID_CONTROLS) | $(PATH_BUILD)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA)  $(DIST_ANDROID_CONTROLS) $(PATH_BUILD_ASSETS)/
ifdef USE_OPENGL_SHADERS
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/shaders
	$(INSTALL) -c -m 644 $(DIST_FILES_SHADERS) $(PATH_BUILD_ASSETS)/shaders
endif

$(PATH_BUILD):
	$(INSTALL) -d $(PATH_BUILD_ASSETS)

$(PATH_BUILD_JNI): libresidualvm.so
	$(INSTALL) -d $(dir $(PATH_BUILD_JNI))
	$(INSTALL) -C -m 644 libresidualvm.so $(PATH_BUILD_JNI)

$(PATH_BUILD_GRADLE): $(PATH_BUILD_ASSETS) $(PATH_DIST)/build.gradle
	$(eval ABIS = $(notdir $(wildcard $(PATH_BUILD)/jni/*)))
	@echo "gradle.ext.versionCode = $(ANDROID_VERSIONCODE)" > $@
	@echo "gradle.ext.versionName = '$(ANDROID_VERSIONNAME)'" >> $@
	@echo "gradle.ext.sourceDir = '$(abspath $(srcdir))'" >> $@
	@echo "gradle.ext.buildDir = '$(CURDIR)'" >> $@
	@echo "gradle.ext.androidAbi = '$(ABIS)'" >> $@
	@echo "include ':ResidualVM'" >> $@
	@echo "project(':ResidualVM').projectDir = new File('$(abspath $(PATH_DIST))')" >> $@
	@echo "ndk.dir=$(ANDROID_NDK)" > $(PATH_BUILD)/local.properties
	@echo "sdk.dir=$(ANDROID_SDK)" >> $(PATH_BUILD)/local.properties


$(PATH_BUILD_SETUPAPK): $(PATH_BUILD_ASSETS) $(PATH_BUILD_JNI) $(PATH_BUILD_GRADLE) | $(PATH_BUILD) 
	touch $(PATH_BUILD_SETUPAPK)

$(APK_MAIN): $(PATH_BUILD_SETUPAPK) libresidualvm.so
	$(GRADLE) assembleDebug -p "$(PATH_BUILD)" && touch $@

$(APK_MAIN_RELEASE): $(PATH_BUILD_SETUPAPK) libresidualvm.so
	$(GRADLE) assembleRelease -p "$(PATH_BUILD)" && touch $@

all: $(APK_MAIN)

clean: androidclean

androidclean:
	@$(RM) -rf $(PATH_BUILD)

androiddebug: $(APK_MAIN)
androidrelease: $(APK_MAIN_RELEASE)

androidtestmain: $(APK_MAIN)
	$(GRADLE) installDebug -p "$(PATH_BUILD)"
	$(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.residualvm.residualvm/.Unpacker

androidtest: $(APK_MAIN) $(APK_PLUGINS)
	@set -e; for apk in $^; do \
		$(ADB) install -r $$apk; \
	done
	$(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.residualvm.residualvm/.Unpacker

# used by buildbot!
androiddistdebug: all
	$(MKDIR) debug
	$(CP) $(APK_MAIN) $(APK_PLUGINS) debug/
	for i in $(DIST_FILES_DOCS) $(PORT_DISTFILES); do \
		sed 's/$$/\r/' < $$i > debug/`basename $$i`.txt; \
	done

.PHONY: androidrelease androidtest
