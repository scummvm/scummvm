#! /bin/sh

set -ex

# Run from the build folder

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
NPROC=$(nproc)

if [ -n "$1" ]; then
	GAMES_FOLDER=$(CDPATH= cd -- "$1" && pwd)
fi

# Get the version code and patch it for every build
VERSION_CODE=$(sed -n -e '/versionCode /s/[\t ]*versionCode //p' "${ROOT}/dists/android/build.gradle")
# Make sure the last digit is 0
VERSION_CODE=$((${VERSION_CODE} / 10 * 10))

patch_version() {
	local dir
	dir=$2
	if [ -z "$dir" ]; then
		dir=.
	fi
	# Make sure the file exists before patching
	make -C "$dir" android_project/build.gradle
	sed -i -e "/versionCode /s/\\([\t ]*versionCode \\).*/\\1$1/" "$dir/android_project/build.gradle"
}

# We don't handle games change correctly, force refresh
rm -rf "./android_project/mainAssets/src/main/assets/assets/games"

"${ROOT}/configure" --host=android-arm-v7a --disable-debug --enable-release

# Make sure we use the proper versionCode
patch_version ${VERSION_CODE}

# Build an AAB bundle with games
make -j${NPROC} androidfatbundlerelease GAMES_BUNDLE_DIRECTORY="$GAMES_FOLDER"

# For APK strip out the games
if [ -n "$GAMES_FOLDER" ]; then
	rm -rf "./android_project/mainAssets/src/main/assets/assets/games"
fi

# Reuse what we just built to create APKs

# Cleanup fat shared objects
rm -rf ./android_project/lib/*

plat_build() {
	local subcode subarch subbuild
	subcode=$1
	subarch=$2
	subbuild=$3
	if [ -z "$subbuild" ]; then
		subbuild=build-android${subarch}
	fi
	patch_version $((${VERSION_CODE} + ${subcode})) "${subbuild}"
	make -j${NPROC} -C ${subbuild} androidrelease
	mv ${subbuild}/ScummVM-release-unsigned.apk ScummVM-release-unsigned-${subarch}.apk
}

# Build ARMv7a with versionCode 1
plat_build 1 armeabi-v7a .

# Build ARM64 with versionCode 2
plat_build 2 arm64-v8a

# Build x86 with versionCode 3
plat_build 3 x86

# Build x86_64 with versionCode 4
plat_build 4 x86_64
