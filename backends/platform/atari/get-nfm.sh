#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

# downloads and unpacks in ./deps folder precompiled nFM library release for m68k Atari TOS / mintelf gcc stdlib
NFM_VERSION=0.3.0
COMMIT_SHORT_SHA=ee64d5f7

curl -o ./backends/platform/atari/nfm.zip https://nokturnal.pl/downloads/atari/nokturnfm/nfmplay/builds/releases/${NFM_VERSION}/ataritos-mintelf-gcc-stdlib-m68k-${NFM_VERSION}-${COMMIT_SHORT_SHA}.zip
rm -rf ./backends/platform/atari/deps/nfm

unzip ./backends/platform/atari/nfm.zip "nFM_${NFM_VERSION}/*" -d ./backends/platform/atari/deps/
mv ./backends/platform/atari/deps/nFM_${NFM_VERSION} ./backends/platform/atari/deps/nfm
rm ./backends/platform/atari/nfm.zip