Welcome to Sailfish port of ScummVM.

# Building

First you need to download dependencies as we link statically with them:

``` shell
./sailfish-download.sh
```

You have to do this only once

Suppose that your SDK is in `$SDK_ROOT`

First you need to figure out your target config

``` shell
$SDK_ROOT/bin/sfdk engine exec sb2-config -l
```

``` text

SailfishOS-armv7hl
SailfishOS-i486-x86
```

Then run:

``` shell
$SDK_ROOT/bin/sfdk config --push target $TARGET
$SDK_ROOT/bin/sfdk build
```

And finally you need to sign them with your developer key:

``` shell
$SDK_ROOT/bin/sfdk engine exec -tt sb2 -t $TARGET rpmsign-external sign -k $KEY_PATH/regular_key.pem -c $KEY_PATH/regular_cert.pem RPMS/*.rpm
```


# Known issues

* Screen dimming and sleep aren't inhibited in game
* If you close ScummVM window on the panel ScummVM isn't closed and you
can't open it again
* When switching From Surface SDL to OpenGL renderer touchscreen coordinates
are garbled until restart
* make install doesn't install Sailfish-specific file. It's done in RPM spec
