MODULE := backends/platform/android

MODULE_OBJS := \
	android.o asset-archive.o video.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))

JAVA_SRC = \
	$(MODULE)/org/inodes/gus/scummvm/ScummVM.java \
	$(MODULE)/org/inodes/gus/scummvm/ScummVMApplication.java \
	$(MODULE)/org/inodes/gus/scummvm/ScummVMActivity.java \
	$(MODULE)/org/inodes/gus/scummvm/EditableSurfaceView.java \
	$(MODULE)/org/inodes/gus/scummvm/Unpacker.java \
	$(MODULE)/org/inodes/gus/scummvm/Manifest.java \
	$(MODULE)/org/inodes/gus/scummvm/R.java

JAVA_PLUGIN_SRC = \
	$(MODULE)/org/inodes/gus/scummvm/PluginProvider.java

RESOURCES = \
	$(srcdir)/dists/android/res/values/strings.xml \
	$(srcdir)/dists/android/res/layout/main.xml \
	$(srcdir)/dists/android/res/layout/splash.xml \
	$(srcdir)/dists/android/res/drawable/gradient.xml \
	$(srcdir)/dists/android/res/drawable/scummvm.png \
	$(srcdir)/dists/android/res/drawable/scummvm_big.png

ASSETS = $(DIST_FILES_ENGINEDATA) $(DIST_FILES_THEMES)

PLUGIN_RESOURCES = \
	$(srcdir)/dists/android/res/values/strings.xml \
	$(srcdir)/dists/android/res/drawable/scummvm.png

# These must be incremented for each market upload
#ANDROID_VERSIONCODE = 6  Specified in dists/android/AndroidManifest.xml.in
ANDROID_PLUGIN_VERSIONCODE = 6

# This is a bit silly.  I want to compile against the 1.6 android.jar,
# to make the compiler check that I don't use something that requires
# a newer Android.  However, in order to use android:installLocation,
# we need to give aapt a version >=8 android.jar - even though the
# result will work ok on 1.5+.
ANDROID_JAR = $(ANDROID_SDK)/platforms/android-1.5/android.jar
ANDROID_JAR8 = $(ANDROID_SDK)/platforms/android-8/android.jar

# This library contains scummvm proper
build.tmp/libscummvm.so: $(OBJS)
	@$(MKDIR) -p $(@D)
	$(QUIET_LINK)$(CXX) -shared $(LDFLAGS) -Wl,-Bsymbolic -Wl,-soname,$(@F) -Wl,--no-undefined -o $@ $(PRE_OBJS_FLAGS) $(OBJS) $(POST_OBJS_FLAGS) $(LIBS)


backends/platform/android/org/inodes/gus/scummvm/R.java backends/platform/android/org/inodes/gus/scummvm/Manifest.java: $(srcdir)/dists/android/AndroidManifest.xml $(filter %.xml,$(RESOURCES)) $(ANDROID_JAR8)
	$(AAPT) package -m -J backends/platform/android -M $< -S $(srcdir)/dists/android/res -I $(ANDROID_JAR8)

build.tmp/classes/%.class: $(srcdir)/backends/platform/android/%.java $(srcdir)/backends/platform/android/org/inodes/gus/scummvm/R.java
	@$(MKDIR) -p $(@D)
	$(JAVAC) $(JAVACFLAGS) -cp $(srcdir)/backends/platform/android -d build.tmp/classes -bootclasspath $(ANDROID_JAR) $<

build.tmp/classes.plugin/%.class: $(srcdir)/backends/platform/android/%.java
	@$(MKDIR) -p $(@D)
	$(JAVAC) $(JAVACFLAGS) -cp $(srcdir)/backends/platform/android -d build.tmp/classes.plugin -bootclasspath $(ANDROID_JAR) $<

classes.dex: $(JAVA_SRC:backends/platform/android/%.java=build.tmp/classes/%.class)
	$(DX) --dex --output=$@ build.tmp/classes

build.tmp/plugins/classes.dex: $(JAVA_PLUGIN_SRC:backends/platform/android/%.java=build.tmp/classes.plugin/%.class)
	@$(MKDIR) -p $(@D)
	$(DX) --dex --output=$@ build.tmp/classes.plugin

build.tmp/%/AndroidManifest.xml build.stage/%/res/values/strings.xml: dists/android/mkmanifest.pl configure dists/android/AndroidManifest.xml
	dists/android/mkmanifest.pl --id=$* --configure=configure \
	  --version-name=$(VERSION) \
	  --version-code=$(ANDROID_PLUGIN_VERSIONCODE) \
	  --stringres=build.stage/$*/res/values/strings.xml \
	  --manifest=build.tmp/$*/AndroidManifest.xml \
	  --master-manifest=dists/android/AndroidManifest.xml \
	  --unpacklib=mylib/armeabi/lib$*.so

build.stage/%/res/drawable/scummvm.png: dists/android/res/drawable/scummvm.png
	@$(MKDIR) -p $(@D)
	$(CP) $< $@
