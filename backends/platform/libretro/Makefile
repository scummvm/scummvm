ROOT_PATH := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Output files prefix
TARGET_NAME = scummvm

HIDE := @
SPACE :=
SPACE := $(SPACE) $(SPACE)
BACKSLASH :=
BACKSLASH := \$(BACKSLASH)
filter_out1 = $(filter-out $(firstword $1),$1)
filter_out2 = $(call filter_out1,$(call filter_out1,$1))
unixpath = $(subst \,/,$1)
unixcygpath = /$(subst :,,$(call unixpath,$1))

ifeq ($(shell uname -a),)
   EXE_EXT = .exe
endif

ifeq ($(BUILD_64BIT),)
ifeq (,$(findstring 64,$(platform) $(TOOLSET)))
   BUILD_64BIT := 0
else
   BUILD_64BIT := 1
endif
endif
TARGET_64BIT := $(BUILD_64BIT)

LD        = $(CXX)
AR       := ar cru
RANLIB   := ranlib
LS       := ls
MKDIR    := mkdir -p
RM       := rm -f
RM_REC   := rm -rf
CP       := cp


LDFLAGS :=


# Raspberry Pi 1
ifeq ($(platform), rpi1)
   TARGET = $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DUSE_CXX11 -marm -DARM
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS += -fPIC -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -fomit-frame-pointer -ffast-math
   CXXFLAGS = $(CFLAGS) -frtti -std=c++11
   HAVE_OPENGLES2 := 1

# Raspberry Pi 3
else ifeq ($(platform), rpi3)
   TARGET = $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DUSE_CXX11 -marm -DARM
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS += -fPIC -mcpu=cortex-a53 -mtune=cortex-a53 -fomit-frame-pointer -ffast-math
   CXXFLAGS = $(CFLAGS) -frtti -std=c++11
   HAVE_OPENGLES2 := 1
   HAVE_NEON := 1

 # Raspberry Pi 3 (64 bit)
else ifeq ($(platform), rpi3_64)
   TARGET   = $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DUSE_CXX11 -DARM -DDEFAULT_PERF_TUNER
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS += -fPIC -mcpu=cortex-a53 -mtune=cortex-a53 -fomit-frame-pointer -ffast-math
   CXXFLAGS = $(CFLAGS) -frtti -std=c++11
   HAVE_OPENGLES2 := 1
   HAVE_NEON := 1

# Raspberry Pi 5 (64 bit)
else ifeq ($(platform), rpi5_64)
   TARGET = $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DUSE_CXX11 -DARM
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS += -fPIC -mcpu=cortex-a76 -mtune=cortex-a76 -fomit-frame-pointer -ffast-math
   CXXFLAGS = $(CFLAGS) -frtti -std=c++11
   HAVE_OPENGLES2 := 1
   HAVE_NEON := 1

# Raspberry Pi 4 (64 bit)
else ifeq ($(platform), rpi4_64)
   TARGET = $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DUSE_CXX11 -DARM
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS += -fPIC -mcpu=cortex-a72 -mtune=cortex-a72 -fomit-frame-pointer -ffast-math
   CXXFLAGS = $(CFLAGS) -frtti -std=c++11
   HAVE_OPENGLES2 := 1
   HAVE_NEON := 1

# iOS
else ifneq (,$(findstring ios,$(platform)))
   TARGET  := $(TARGET_NAME)_libretro_ios.dylib
   DEFINES += -fPIC -DHAVE_POSIX_MEMALIGN=1 -DIOS
   LDFLAGS += -dynamiclib -fPIC
   MINVERSION :=
   HAVE_OPENGLES2 := 1
   HAVE_NEON := 1

ifeq ($(IOSSDK),)
   IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
endif
ifeq ($(platform),ios-arm64)
   CC        = cc -arch arm64 -isysroot $(IOSSDK)
   CXX       = c++ -arch arm64 -isysroot $(IOSSDK)
else
   CC       = cc -arch armv7 -isysroot $(IOSSDK)
   CXX      = c++ -arch armv7 -isysroot $(IOSSDK)
   DEFINES  = -marm -mcpu=cortex-a8 -mtune=cortex-a8 -mfpu=neon
endif

ifeq ($(platform),$(filter $(platform),ios9 ios-arm64))
   MINVERSION += -miphoneos-version-min=8.0
else
   MINVERSION += -miphoneos-version-min=5.0
endif
   CFLAGS   += $(MINVERSION)
   CXXFLAGS += $(MINVERSION) -std=c++11

else ifeq ($(platform), tvos-arm64)
   EXT?=dylib
   TARGET := $(TARGET_NAME)_libretro_tvos.$(EXT)
   DEFINES += -fPIC -DHAVE_POSIX_MEMALIGN=1 -DIOS
   LDFLAGS += -dynamiclib -fPIC
   HAVE_OPENGLES2 := 1
   HAVE_NEON := 1
ifeq ($(IOSSDK),)
   IOSSDK := $(shell xcodebuild -version -sdk appletvos Path)
endif
   CC  = cc -arch arm64 -isysroot $(IOSSDK)
   CXX = c++ -arch arm64 -isysroot $(IOSSDK) -std=c++11

# QNX
else ifeq ($(platform), qnx)
   TARGET  := $(TARGET_NAME)_libretro_$(platform).so
   DEFINES += -fPIC -DSYSTEM_NOT_SUPPORTING_D_TYPE -DDEFAULT_PERF_TUNER
   DEFINES += -D__BLACKBERRY_QNX__ -marm -mcpu=cortex-a9 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CC = qcc -Vgcc_ntoarmv7le
   CXX = QCC -Vgcc_ntoarmv7le
   LD = QCC -Vgcc_ntoarmv7le
   AR = qcc -Vgcc_ntoarmv7le -A
   RANLIB="${QNX_HOST}/usr/bin/ntoarmv7-ranlib"
   HAVE_OPENGLES2 := 1.
   HAVE_NEON := 1

# Genode
else ifeq ($(platform), genode)
   TARGET  := $(TARGET_NAME)_libretro_$(platform).so
   DEFINES += -fPIC -DSYSTEM_NOT_SUPPORTING_D_TYPE -DDEFAULT_PERF_TUNER
   C_PKGS   = libc
   CXX_PKGS = stdcxx genode-base
   CFLAGS   += -D__GENODE__ $(shell pkg-config --cflags $(C_PKGS))
   CXXFLAGS += -D__GENODE__ $(shell pkg-config --cflags $(CXX_PKGS))
   LIBS += $(shell pkg-config --libs $(C_PKGS) $(CXX_PKGS) genode-lib)
   CC  = $(shell pkg-config genode-base --variable=cc)
   CXX = $(shell pkg-config genode-base --variable=cxx)
   LD  = $(shell pkg-config genode-base --variable=ld)
   AR  = $(shell pkg-config genode-base --variable=ar) rcs
   RANLIB = genode-x86-ranlib
   HAVE_OPENGL := 1

# Lightweight PS3 Homebrew SDK
else ifneq (,$(filter $(platform), ps3 psl1ght))
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(PS3DEV)/ppu/bin/ppu-$(COMMONLV)gcc$(EXE_EXT)
   CXX = $(PS3DEV)/ppu/bin/ppu-$(COMMONLV)g++$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-$(COMMONLV)ar$(EXE_EXT) rcs
   DEFINES := -DPLAYSTATION3 -D__PS3__ -DRETRO_IS_BIG_ENDIAN=1 -DRETRO_IS_LITTLE_ENDIAN=0 -DWORDS_BIGENDIAN=1 -mcpu=cell -mno-fp-in-toc -I$(PS3DEV)/ppu/include
   ifeq ($(platform), psl1ght)
      DEFINES += -D__PSL1GHT__
      HAVE_OPENGL := 1
   endif
   STATIC_LINKING = 1

# Nintendo Wii
else ifeq ($(platform), wii)
   TARGET := $(TARGET_NAME)_libretro_wii.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT) rcs
   DEFINES += -DGEKKO -DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float -D__ppc__ -I$(DEVKITPRO)/libogc/include -DDEFAULT_PERF_TUNER -DRETRO_IS_BIG_ENDIAN=1 -DRETRO_IS_LITTLE_ENDIAN=0 -DWORDS_BIGENDIAN=1
   LITE := 1
   STATIC_LINKING = 1

# Nintendo Switch (libnx)
else ifeq ($(platform), libnx)
    export DEPSDIR := $(CURDIR)
    include $(DEVKITPRO)/libnx/switch_rules
    EXT=a
    TARGET := $(TARGET_NAME)_libretro_$(platform).$(EXT)
    AR = $(DEVKITPRO)/devkitA64/aarch64-none-elf/bin/ar$(EXE_EXT) rcs
    DEFINES := -DSWITCH=1 -U__linux__ -U__linux
    DEFINES   += -g -O3 -fPIE -I$(LIBNX)/include/ -ffunction-sections -fdata-sections -ftls-model=local-exec -ffast-math -mcpu=cortex-a57+crc+fp+simd -march=armv8-a -mtune=cortex-a57 -mtp=soft
    DEFINES += $(INCDIRS)
    DEFINES += -D__SWITCH__ -DHAVE_LIBNX -D__arm64__ -D__ARM_NEON__
    CXXFLAGS := $(ASFLAGS) -std=gnu++11 -fpermissive
    STATIC_LINKING = 1
    HAVE_OPENGL := 1
    HAVE_NEON := 1

# Nintendo Wii U
else ifeq ($(platform), wiiu)
   TARGET := $(TARGET_NAME)_libretro_wiiu.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT) rcs
   AR_ALONE = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   DEFINES += -mwup -mcpu=750 -meabi -mhard-float -D__POWERPC__ -D__ppc__ -DRETRO_IS_BIG_ENDIAN=1 -DRETRO_IS_LITTLE_ENDIAN=0 -DWORDS_BIGENDIAN=1
   DEFINES += -U__INT32_TYPE__ -U __UINT32_TYPE__ -D__INT32_TYPE__=int
   DEFINES += -DHAVE_STRTOUL -DWIIU
   CXXFLAGS := -fpermissive
   LITE := 1
   STATIC_LINKING = 1

# Nintendo 3DS
else ifeq ($(platform), ctr)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(DEVKITARM)/bin/arm-none-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITARM)/bin/arm-none-eabi-g++$(EXE_EXT)
   AR = $(DEVKITARM)/bin/arm-none-eabi-ar$(EXE_EXT) rcs
   RANLIB = $(DEVKITARM)/bin/arm-none-eabi-ranlib$(EXE_EXT)
   ifeq ($(strip $(CTRULIB)),)
      $(error "Please set CTRULIB in your environment. export CTRULIB=<path to>libctru")
   endif
   DEFINES += -DARM11 -D_3DS -I$(CTRULIB)/include
   DEFINES += -march=armv6k -mtune=mpcore -mfloat-abi=hard
   DEFINES += -Wall -mword-relocations
   DEFINES += -fomit-frame-pointer -ffast-math -DDEFAULT_PERF_TUNER
   CXXFLAGS += -std=gnu++11 -fpermissive
   USE_VORBIS = 0
   USE_THEORADEC = 0
   USE_TREMOR = 1
   USE_MT32EMU = 0
   STATIC_LINKING = 1

# Vita
else ifeq ($(platform), vita)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = arm-vita-eabi-gcc$(EXE_EXT)
   CXX = arm-vita-eabi-g++$(EXE_EXT)
   AR = arm-vita-eabi-ar$(EXE_EXT) rcs
   DEFINES += -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -mfpu=neon -marm
   DEFINES += -fno-optimize-sibling-calls -mlong-calls -mword-relocations
   DEFINES += -DVITA -DREDUCE_MEMORY_USAGE -DUNCACHED_PLUGINS -DPSP2 -DSYSTEM_NOT_SUPPORTING_D_TYPE
   USE_LIBCO = 0
   LITE := 1
   STATIC_LINKING = 1
   HAVE_OPENGLES2 := 1
   HAVE_NEON := 1

# GCW0
else ifeq ($(platform), gcw0)
   TARGET := $(TARGET_NAME)_libretro.so
   CC = /opt/gcw0-toolchain/usr/bin/mipsel-linux-gcc
   CXX = /opt/gcw0-toolchain/usr/bin/mipsel-linux-g++
   LD = /opt/gcw0-toolchain/usr/bin/mipsel-linux-g++
   AR = /opt/gcw0-toolchain/usr/bin/mipsel-linux-ar cru
   RANLIB = /opt/gcw0-toolchain/usr/bin/mipsel-linux-ranlib
   DEFINES += -DDINGUX -fomit-frame-pointer -ffast-math -march=mips32 -mtune=mips32r2 -mhard-float -fPIC
   DEFINES += -DPRId64 #Workaround to avoid compilation error in retro_common_api.h. Root cause to be investigated.
   DEFINES += -ffunction-sections -fdata-sections -DDEFAULT_PERF_TUNER
   LDFLAGS += -shared -Wl,--gc-sections -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS += -std=gnu99
   CXXFLAGS += -std=c++11
   HAVE_OPENGLES2 := 1
   USE_VORBIS = 0
   USE_THEORADEC = 0
   USE_TREMOR = 1
   USE_LIBCO  = 0
   USE_CURL = 0
   USE_MT32EMU = 0
   USE_HIGHRES := 0

# MIYOO
else ifeq ($(platform), miyoo)
   TARGET := $(TARGET_NAME)_libretro.so
   CC = /opt/miyoo/usr/bin/arm-linux-gcc
   CXX = /opt/miyoo/usr/bin/arm-linux-g++
   LD = /opt/miyoo/usr/bin/arm-linux-g++
   AR = /opt/miyoo/usr/bin/arm-linux-ar cru
   RANLIB = /opt/miyoo/usr/bin/arm-linux-ranlib
   DEFINES += -DDINGUX -fomit-frame-pointer -ffast-math -march=armv5te -mtune=arm926ej-s -fPIC
   DEFINES += -ffunction-sections -fdata-sections -DDEFAULT_PERF_TUNER -DREDUCE_MEMORY_USAGE -DUNCACHED_PLUGINS
   LDFLAGS += -shared -Wl,--gc-sections -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   USE_VORBIS = 0
   USE_THEORADEC = 0
   USE_TREMOR = 1
   USE_LIBCO  = 0
   USE_CURL = 0
   USE_MT32EMU = 0
   USE_HIGHRES := 0

# MIYOOMINI
else ifeq ($(platform), miyoomini)
   TARGET := $(TARGET_NAME)_libretro.so
   CC = /usr/bin/arm-linux-gnueabihf-gcc
   CXX = /usr/bin/arm-linux-gnueabihf-g++
   LD = /usr/bin/arm-linux-gnueabihf-g++
   AR = /usr/bin/arm-linux-gnueabihf-ar cru
   RANLIB = /usr/bin/arm-linux-gnueabihf-ranlib
   DEFINES += -fomit-frame-pointer -ffast-math -marm -march=armv7ve+simd -mtune=cortex-a7 -fPIC
   DEFINES += -ffunction-sections -fdata-sections -DDEFAULT_PERF_TUNER -DREDUCE_MEMORY_USAGE -DUNCACHED_PLUGINS
   LDFLAGS += -shared -Wl,--gc-sections -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   USE_VORBIS = 0
   USE_THEORADEC = 0
   USE_TREMOR = 1
   USE_LIBCO  = 0
   USE_MT32EMU = 0
   USE_HIGHRES = 0

# ARM v7
else ifneq (,$(findstring armv7,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DUSE_CXX11 -marm -DARM -DDEFAULT_PERF_TUNER
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   USE_VORBIS = 0
   USE_THEORADEC = 0
   USE_TREMOR = 1
   USE_MT32EMU = 0
   CXXFLAGS := -std=c++11
ifneq (,$(findstring cortexa8,$(platform)))
   DEFINES += -marm -mcpu=cortex-a8
else ifneq (,$(findstring cortexa9,$(platform)))
   DEFINES += -marm -mcpu=cortex-a9
endif
ifneq (,$(findstring neon,$(platform)))
   DEFINES += -mfpu=neon
   HAVE_NEON := 1
endif
ifneq (,$(findstring softfloat,$(platform)))
   DEFINES += -mfloat-abi=softfp
else ifneq (,$(findstring hardfloat,$(platform)))
   DEFINES += -mfloat-abi=hard
endif
   HAVE_OPENGLES2 := 1

# ARM v8
else ifneq (,$(findstring armv8,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DARM -marm -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard -march=armv8-a+crc -DDEFAULT_PERF_TUNER
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS   += -fPIC
   HAVE_NEON := 1
   HAVE_OPENGLES2 := 1

# Odroid Go Advance
else ifneq (,$(findstring odroidgo2,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DARM -DUSE_CXX11 -marm -mtune=cortex-a35 -mfpu=neon-fp-armv8 -mfloat-abi=hard -march=armv8-a+crc
   DEFINES += -fPIC -D_ARM_ASSEM_ -DUSE_CXX11 -DARM
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS += -fPIC -mcpu=cortex-a35 -fomit-frame-pointer -ffast-math
   CXXFLAGS = $(CFLAGS) -frtti -std=c++11
   HAVE_OPENGLES2 := 1
   HAVE_NEON = 1

# RG353x
else ifneq (,$(findstring rg353x,$(platform)))
   TARGET = $(TARGET_NAME)_libretro.so
   DEFINES += -fPIC -D_ARM_ASSEM_ -DUSE_CXX11 -DARM
   LDFLAGS += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS += -fPIC -mcpu=cortex-a55 -mtune=cortex-a55 -fomit-frame-pointer -ffast-math
   CXXFLAGS = $(CFLAGS) -frtti -std=c++11
   HAVE_OPENGLES2 := 1
   HAVE_NEON = 1

# Emscripten
else ifeq ($(platform), emscripten)
   TARGET := $(TARGET_NAME)_libretro_$(platform).bc
   AR_ALONE := emar
   AR := emar rcs
   DEFINES += -DEMSCRIPTEN -DUSE_CXX11
   CXXFLAGS += -std=c++11
   STATIC_LINKING = 1
   USE_LIBCO  = 0
   USE_CLOUD = 0

# Windows MSVC 2017 all architectures
else ifneq (,$(findstring windows_msvc2017,$(platform)))

    NO_GCC := 1
    CFLAGS += -DNOMINMAX
    CXXFLAGS += -DNOMINMAX
    WINDOWS_VERSION = 1

   PlatformSuffix = $(subst windows_msvc2017_,,$(platform))
   ifneq (,$(findstring desktop,$(PlatformSuffix)))
      WinPartition = desktop
      MSVC2017CompileFlags = -DWINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP -FS
      LDFLAGS += -MANIFEST -LTCG:incremental -NXCOMPAT -DYNAMICBASE -DEBUG -OPT:REF -INCREMENTAL:NO -SUBSYSTEM:WINDOWS -MANIFESTUAC:"level='asInvoker' uiAccess='false'" -OPT:ICF -ERRORREPORT:PROMPT -NOLOGO -TLBID:1
      LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
   else ifneq (,$(findstring uwp,$(PlatformSuffix)))
      WinPartition = uwp
      MSVC2017CompileFlags = -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WINDLL -D_UNICODE -DUNICODE -D__WRL_NO_DEFAULT_LIB__ -EHsc -FS
      LDFLAGS += -APPCONTAINER -NXCOMPAT -DYNAMICBASE -MANIFEST:NO -LTCG -OPT:REF -SUBSYSTEM:CONSOLE -MANIFESTUAC:NO -OPT:ICF -ERRORREPORT:PROMPT -NOLOGO -TLBID:1 -DEBUG:FULL -WINMD:NO
      LIBS += WindowsApp.lib
   endif

   CFLAGS += $(MSVC2017CompileFlags)
   CXXFLAGS += $(MSVC2017CompileFlags)

   TargetArchMoniker = $(subst $(WinPartition)_,,$(PlatformSuffix))

   CC  = cl.exe
   CXX = cl.exe
   LD = link.exe

   reg_query = $(call filter_out2,$(subst $2,,$(shell reg query "$2" -v "$1" 2>nul)))
   fix_path = $(subst $(SPACE),\ ,$(subst \,/,$1))

   ProgramFiles86w := $(shell cmd /c "echo %PROGRAMFILES(x86)%")
   ProgramFiles86 := $(shell cygpath "$(ProgramFiles86w)")

   WindowsSdkDir ?= $(call reg_query,InstallationFolder,HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v10.0)
   WindowsSdkDir ?= $(call reg_query,InstallationFolder,HKEY_CURRENT_USER\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v10.0)
   WindowsSdkDir ?= $(call reg_query,InstallationFolder,HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0)
   WindowsSdkDir ?= $(call reg_query,InstallationFolder,HKEY_CURRENT_USER\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0)
   WindowsSdkDir := $(WindowsSdkDir)

   WindowsSDKVersion ?= $(firstword $(foreach folder,$(subst $(subst \,/,$(WindowsSdkDir)Include/),,$(wildcard $(call fix_path,$(WindowsSdkDir)Include\*))),$(if $(wildcard $(call fix_path,$(WindowsSdkDir)Include/$(folder)/um/Windows.h)),$(folder),)))$(BACKSLASH)
   WindowsSDKVersion := $(WindowsSDKVersion)

   VsInstallBuildTools = $(ProgramFiles86)/Microsoft Visual Studio/2017/BuildTools
   VsInstallEnterprise = $(ProgramFiles86)/Microsoft Visual Studio/2017/Enterprise
   VsInstallProfessional = $(ProgramFiles86)/Microsoft Visual Studio/2017/Professional
   VsInstallCommunity = $(ProgramFiles86)/Microsoft Visual Studio/2017/Community

   VsInstallRoot ?= $(shell if [ -d "$(VsInstallBuildTools)" ]; then echo "$(VsInstallBuildTools)"; fi)
   ifeq ($(VsInstallRoot), )
      VsInstallRoot = $(shell if [ -d "$(VsInstallEnterprise)" ]; then echo "$(VsInstallEnterprise)"; fi)
   endif
   ifeq ($(VsInstallRoot), )
      VsInstallRoot = $(shell if [ -d "$(VsInstallProfessional)" ]; then echo "$(VsInstallProfessional)"; fi)
   endif
   ifeq ($(VsInstallRoot), )
      VsInstallRoot = $(shell if [ -d "$(VsInstallCommunity)" ]; then echo "$(VsInstallCommunity)"; fi)
   endif
   VsInstallRoot := $(VsInstallRoot)

   VcCompilerToolsVer := $(shell cat "$(VsInstallRoot)/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt" | grep -o '[0-9\.]*')
   VcCompilerToolsDir := $(VsInstallRoot)/VC/Tools/MSVC/$(VcCompilerToolsVer)

   WindowsSDKSharedIncludeDir := $(shell cygpath -w "$(WindowsSdkDir)\Include\$(WindowsSDKVersion)\shared")
   WindowsSDKUCRTIncludeDir := $(shell cygpath -w "$(WindowsSdkDir)\Include\$(WindowsSDKVersion)\ucrt")
   WindowsSDKUMIncludeDir := $(shell cygpath -w "$(WindowsSdkDir)\Include\$(WindowsSDKVersion)\um")
   WindowsSDKUCRTLibDir := $(shell cygpath -w "$(WindowsSdkDir)\Lib\$(WindowsSDKVersion)\ucrt\$(TargetArchMoniker)")
   WindowsSDKUMLibDir := $(shell cygpath -w "$(WindowsSdkDir)\Lib\$(WindowsSDKVersion)\um\$(TargetArchMoniker)")

   # For some reason the HostX86 compiler doesn't like compiling for x64
   # ("no such file" opening a shared library), and vice-versa.
   # Work around it for now by using the strictly x86 compiler for x86, and x64 for x64.
   # NOTE: What about ARM?
   ifneq (,$(findstring x64,$(TargetArchMoniker)))
      VCCompilerToolsBinDir := $(VcCompilerToolsDir)\bin\HostX64
   else
      VCCompilerToolsBinDir := $(VcCompilerToolsDir)\bin\HostX86
   endif

   PATH := $(shell IFS=$$'\n'; cygpath "$(VCCompilerToolsBinDir)/$(TargetArchMoniker)"):$(PATH)
   PATH := $(PATH):$(shell IFS=$$'\n'; cygpath "$(VsInstallRoot)/Common7/IDE")
   INCLUDE := $(shell IFS=$$'\n'; cygpath -w "$(VcCompilerToolsDir)/include")
   LIB := $(shell IFS=$$'\n'; cygpath -w "$(VcCompilerToolsDir)/lib/$(TargetArchMoniker)")
   ifneq (,$(findstring uwp,$(PlatformSuffix)))
      LIB := $(LIB);$(shell IFS=$$'\n'; cygpath -w "$(LIB)/store")
   endif

   export INCLUDE := $(INCLUDE);$(WindowsSDKSharedIncludeDir);$(WindowsSDKUCRTIncludeDir);$(WindowsSDKUMIncludeDir)
   export LIB := $(LIB);$(WindowsSDKUCRTLibDir);$(WindowsSDKUMLibDir)
   TARGET := $(TARGET_NAME)_libretro.dll
   PSS_STYLE :=2
   LDFLAGS += -DLL
   HAVE_OPENGL := 1

else ifeq ($(platform), win)
   # let out for next test block

else ifeq ($(platform), osx)
   # let out for next test block

else ifeq ($(platform), unix)
   # let out for next test block

else
   # Nothing found for specified platform or none set

   override platform = unix
   ifeq ($(shell uname -a),)
      override platform = win
   else ifneq ($(findstring MINGW,$(shell uname -a)),)
      override platform = win
   else ifneq ($(findstring Darwin,$(shell uname -a)),)
      override platform = osx
      override arch = intel
      ifeq ($(shell uname -p),arm64)
         override arch = arm
      endif
      ifeq ($(shell uname -p),powerpc)
         override arch = ppc
      endif
   else ifneq ($(findstring win,$(shell uname -a)),)
      override platform = win
   endif

   ifeq (,$(findstring 64,$(shell uname -m)))
      BUILD_64BIT := 0
   else
      BUILD_64BIT := 1
   endif
   TARGET_64BIT := $(BUILD_64BIT)
endif

# Unix fallback
ifeq ($(platform), unix)
   TARGET   := $(TARGET_NAME)_libretro.so
   DEFINES  += -DHAVE_POSIX_MEMALIGN=1 -DUSE_CXX11
   LDFLAGS  += -shared -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
   CFLAGS   += -fPIC
   CXXFLAGS += $(CFLAGS) -std=c++11
   HAVE_OPENGL := 1
# Win fallback
else ifeq ($(platform), win)
   CC ?= gcc
   TARGET  := $(TARGET_NAME)_libretro.dll
   DEFINES += -DHAVE_FSEEKO -DHAVE_INTTYPES_H -fPIC
   CXXFLAGS += -fno-permissive
   LDFLAGS += -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=$(ROOT_PATH)/link.T -fPIC
ifneq ($(TARGET_64BIT),1)
   DEFINES += -DRETRO_CALLCONV=__cdecl
endif
   HAVE_OPENGL := 1
# OS X
else ifeq ($(platform), osx)
   TARGET  := $(TARGET_NAME)_libretro.dylib
   DEFINES += -fPIC -Wno-undefined-var-template -Wno-pragma-pack -DHAVE_POSIX_MEMALIGN=1 -DUSE_CXX11
   LDFLAGS += -dynamiclib -fPIC
   CXXFLAGS := -std=c++11
   HAVE_OPENGL := 1
   ifeq ($(CROSS_COMPILE),1)
      TARGET_RULE   = -target $(LIBRETRO_APPLE_PLATFORM) -isysroot $(LIBRETRO_APPLE_ISYSROOT)
      CFLAGS   += $(TARGET_RULE)
      CPPFLAGS += $(TARGET_RULE)
      CXXFLAGS += $(TARGET_RULE)
      LDFLAGS  += $(TARGET_RULE)
      # Hardcode TARGET_64BIT for now
      TARGET_64BIT = 1
   endif
endif

ifneq (,$(findstring $(platform), wiiu vita))
   DEFINES += -Os
else ifneq (,$(findstring $(platform), ios osx))
   DEFINES += -O1
else ifneq (,$(findstring $(platform), msvc genode rpi))
   DEFINES += -O2
else
   DEFINES += -O3
endif

ifeq ($(TARGET_64BIT), 1)
   DEFINES += -DSIZEOF_SIZE_T=8 -DSCUMM_64BITS
else
   DEFINES += -DSIZEOF_SIZE_T=4
endif

# Define toolset
ifdef TOOLSET
   CC        = $(TOOLSET)gcc
   CXX       = $(TOOLSET)g++
   LD        = $(TOOLSET)g++
   AR        = $(TOOLSET)ar cru
   RANLIB    = $(TOOLSET)ranlib
endif

# Define build flags
DEPDIR        = .deps

CXXFLAGS += -Wno-reorder

# Compile platform specific parts (e.g. filesystem)
ifeq ($(platform), win)
WIN32 = 1
DEFINES += -DWIN32 -DUSE_CXX11
CXXFLAGS += -std=c++11
LIBS += -lwinmm
endif

$(info Platform is $(platform) $(shell test $(TARGET_64BIT) = 1 && echo 64bit || echo 32bit))

include $(ROOT_PATH)/Makefile.common

######################################################################
# The build rules follow - normally you should have no need to
# touch whatever comes after here.
######################################################################

# Concat DEFINES and INCLUDES to form the CPPFLAGS
CPPFLAGS := $(DEFINES) $(INCLUDES)
CXXFLAGS += $(DEFINES) $(INCLUDES)
CFLAGS += $(DEFINES) $(INCLUDES)

# Include the build instructions for all modules
include $(addprefix $(SCUMMVM_PATH)/, $(addsuffix /module.mk,$(MODULES)))

# Depdir information
DEPDIRS := $(addsuffix $(DEPDIR),$(MODULE_PATHS))

# Hack for libnx DEPSDIR issues
libnx-ln:
ifeq ($(platform), libnx)
	ln -s $(SCUMMVM_PATH)/audio/ audio
	ln -s $(SCUMMVM_PATH)/backends/ backends
	ln -s $(SCUMMVM_PATH)/base/ base
	ln -s $(SCUMMVM_PATH)/common/ common
	ln -s $(SCUMMVM_PATH)/engines/ engines
	ln -s $(SCUMMVM_PATH)/graphics/ graphics
	ln -s $(SCUMMVM_PATH)/gui/ gui
	ln -s $(SCUMMVM_PATH)/image/ image
	ln -s $(SCUMMVM_PATH)/video/ video
	touch libnx-ln
endif

OBJOUT   = -o
LINKOUT  = -o

ifneq (,$(findstring msvc,$(platform)))
	OBJOUT = -Fo
	LINKOUT = -out:
ifeq ($(STATIC_LINKING),1)
	LD ?= lib.exe
	STATIC_LINKING=0
else
	LD ?= link.exe
endif
endif

ifneq (,$(filter $(platform), wiiu emscripten))
$(TARGET): $(OBJS) libdeps.a libdetect.a
	$(MKDIR) libtemp
	$(CP) $+ libtemp/
	$(AR_ALONE) -M < $(ROOT_PATH)/script.mri
else ifeq ($(platform), libnx)
$(TARGET): libnx-ln $(OBJS) libdeps.a libdetect.a
	$(MKDIR) libtemp
	cp $+ libtemp/
	$(AR) -M < $(ROOT_PATH)/script.mri
else ifeq ($(STATIC_LINKING), 1)
$(TARGET): $(OBJS) libdeps.a libdetect.a
	$(MKDIR) libtemp
	$(CP) $+ libtemp/
	@echo Linking $@...
	$(AR) -M < $(ROOT_PATH)/script.mri
else
$(TARGET): $(DETECT_OBJS) $(OBJS) libdeps.a
	@echo Linking $@...
	$(HIDE)$(LD) $(LDFLAGS) $+ $(LIBS) $(LINKOUT)$@
endif

libdeps.a: $(OBJS_DEPS)
	@echo Linking $@...
	$(HIDE)$(AR) $@ $^

libdetect.a: $(DETECT_OBJS)
	@echo Linking $@...
	$(HIDE)$(AR) $@ $^

%.o: %.c
	@echo Compiling $(<F)...
	@$(MKDIR) $(*D)
	$(HIDE)$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

%.o: %.cpp
	@echo Compiling $(<F)...
	@$(MKDIR) $(*D)
	$(HIDE)$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

# Dumb compile rule, for C++ compilers that don't allow dependency tracking or
# where it is broken (such as GCC 2.95).
.cpp.o:
	@$(MKDIR) $(*D)
	@echo Compiling $<...
	$(HIDE)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $(<) $(OBJOUT)$*.o

clean:
	@echo Cleaning project...
	$(HIDE)$(RM_REC) $(DEPDIRS) $(DEPS_PATH)
	$(HIDE)$(RM) $(OBJS) $(DETECT_OBJS) $(OBJS_DEPS) libdeps.a libdetect.a $(TARGET) *.a
	$(HIDE)$(RM_REC) libtemp $(MODULES)
	$(HIDE)$(RM) libnx-ln
	$(HIDE)$(RM) scummvm.zip  $(TARGET_NAME)_libretro.* script.mri config.mk.engines.lite ScummVM.dat
	$(HIDE)$(RM) $(ROOT_PATH)/shared_modules.mk $(SCUMMVM_PATH)/engines/engines.mk $(SCUMMVM_PATH)/engines/detection_table.h $(SCUMMVM_PATH)/engines/plugins_table.h

# Include the dependency tracking files.
-include $(wildcard $(addsuffix /*.d,$(DEPDIRS)))

# Mark *.d files and most *.mk files as PHONY. This stops make from trying to
# recreate them (which it can't), and in particular from looking for potential
# source files. This can save quite a bit of disk access time.
.PHONY: $(wildcard $(addsuffix /*.d,$(DEPDIRS))) $(addprefix $(SCUMMVM_PATH)/, $(addsuffix /module.mk,$(MODULES))) \
	$(SCUMMVM_PATH)/$(port_mk) $(SCUMMVM_PATH)/rules.mk $(SCUMMVM_PATH)/engines/engines.mk

.PHONY: clean
