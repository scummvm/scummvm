ALL_ABIS = armeabi-v7a arm64-v8a x86 x86_64
OTHER_ABIS = $(filter-out $(ABI), $(ALL_ABIS))

PATH_BUILD_LIBSSCUMMVM = $(foreach abi, $(OTHER_ABIS), $(PATH_BUILD)/lib/$(abi)/libscummvm.so)

ANDROID_CONFIGURE_PATH := $(realpath $(srcdir)/configure)
ANDROID_CONFIGFLAGS := $(filter-out --host=android-%, $(SAVED_CONFIGFLAGS))
define BUILD_ANDROID
SUBPATH_BUILD_LIBSCUMMVM_abi := ./build-android$(1)/libscummvm.so
PATH_BUILD_LIBSCUMMVM_abi := $(PATH_BUILD)/lib/$(1)/libscummvm.so

SUBPATH_BUILDS += ./build-android$(1)

$$(SUBPATH_BUILD_LIBSCUMMVM_abi): SUBPATH_BUILD=./build-android$(1)
$$(SUBPATH_BUILD_LIBSCUMMVM_abi): config.mk $$(EXECUTABLE)
	$$(INSTALL) -d "$$(SUBPATH_BUILD)"
	(cd "$$(SUBPATH_BUILD)" && \
	$$(foreach VAR,$$(SAVED_ENV_VARS),$$(VAR)="$$(SAVED_$$(VAR))") \
	"$$(ANDROID_CONFIGURE_PATH)" --host=android-$(1) $$(ANDROID_CONFIGFLAGS))
	$$(MAKE) -C "$$(SUBPATH_BUILD)" $$(EXECUTABLE)

$$(PATH_BUILD_LIBSCUMMVM_abi): PATH_BUILD_LIB=$(PATH_BUILD)/lib/$(1)
$$(PATH_BUILD_LIBSCUMMVM_abi): $$(SUBPATH_BUILD_LIBSCUMMVM_abi)
	$$(INSTALL) -d "$$(PATH_BUILD_LIB)"
	$$(INSTALL) -c -m 644 "$$<" "$$@"

endef

SUBPATH_BUILDS :=
$(foreach abi,$(OTHER_ABIS),$(eval $(call BUILD_ANDROID,$(abi))))

androidfatall $(subst android,androidfat,$(ANDROID_BUILD_RULES)): androidfat%: $(PATH_BUILD_LIBSSCUMMVM)
	$(MAKE) $(if $(filter all,$*),$*,android$*)

.PHONY: androidfatall $(subst android,androidfat,$(ANDROID_BUILD_RULES))
