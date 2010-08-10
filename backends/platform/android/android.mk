# Android specific build targets

AAPT = aapt
DX = dx
APKBUILDER = apkbuilder
ADB = adb -e
JAVAC ?= javac
JAVACFLAGS = -source 1.5 -target 1.5

# FIXME: find/mark plugin entry points and add all this back again:
#LDFLAGS += -Wl,--gc-sections
#CXXFLAGS += -ffunction-sections -fdata-sections -fvisibility=hidden -fvisibility-inlines-hidden

resources.ap_: $(srcdir)/dists/android/AndroidManifest.xml $(RESOURCES) $(ASSETS) $(ANDROID_JAR8) $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA)
	$(INSTALL) -d build.tmp/assets/
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) build.tmp/assets/
	$(AAPT) package -f -M $< -S $(srcdir)/dists/android/res -A build.tmp/assets -I $(ANDROID_JAR8) -F $@

build.tmp/%/resources.ap_: build.tmp/%/AndroidManifest.xml build.stage/%/res/values/strings.xml build.stage/%/res/drawable/scummvm.png $(ANDROID_JAR8)
	$(AAPT) package -f -M $< -S build.stage/$*/res -I $(ANDROID_JAR8) -F $@

scummvm.apk: build.tmp/libscummvm.so resources.ap_ classes.dex
	# Package installer won't delete old libscummvm.so on upgrade so
	# replace it with a zero size file
	$(INSTALL) -d build.stage/common/lib/armeabi
	touch build.stage/common/lib/armeabi/libscummvm.so
	# We now handle the library unpacking ourselves from mylib/
	$(INSTALL) -d build.stage/common/mylib/armeabi
	$(INSTALL) -c -m 644 build.tmp/libscummvm.so build.stage/common/mylib/armeabi/
	$(STRIP) build.stage/common/mylib/armeabi/libscummvm.so
	# "-nf lib/armeabi/libscummvm.so" builds bogus paths?
	$(APKBUILDER) $@ -z resources.ap_ -f classes.dex -rf build.stage/common || { $(RM) $@; exit 1; }

scummvm-engine-%.apk: plugins/lib%.so build.tmp/%/resources.ap_ build.tmp/plugins/classes.dex
	$(INSTALL) -d build.stage/$*/apk/mylib/armeabi/
	$(INSTALL) -c -m 644 plugins/lib$*.so build.stage/$*/apk/mylib/armeabi/
	$(STRIP) build.stage/$*/apk/mylib/armeabi/lib$*.so
	$(APKBUILDER) $@ -z build.tmp/$*/resources.ap_ -f build.tmp/plugins/classes.dex -rf build.stage/$*/apk || { $(RM) $@; exit 1; }

release/%.apk: %.apk
	@$(MKDIR) -p $(@D)
	@$(RM) $@
	$(CP) $< $@.tmp
	# remove debugging signature
	zip -d $@.tmp META-INF/\*
	jarsigner $(JARSIGNER_FLAGS) $@.tmp release
	zipalign 4 $@.tmp $@
	$(RM) $@.tmp

androidrelease: release/scummvm.apk $(patsubst plugins/lib%.so,release/scummvm-engine-%.apk,$(PLUGINS))

androidtest: scummvm.apk scummvm-engine-scumm.apk scummvm-engine-kyra.apk
	@set -e; for apk in $^; do \
	  echo $(ADB) install -r $$apk; \
	  $(ADB) install -r $$apk; \
	done
	$(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.inodes.gus.scummvm/.Unpacker

.PHONY: androidrelease androidtest
