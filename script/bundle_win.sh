./configure --disable-engine=myst3 --disable-engine=monkey4 --disable-vorbis --disable-mpeg2 --disable-jpeg --disable-png --disable-freetype2 && make && strip residualvm.exe
a="/c/Users/vbox/Desktop/grim_mouse_0_$1"
mkdir -p $a
rm $a/*
cp README $a/Readme.txt
cp README.ResidualVM $a/Readme_ResidualVM.txt
cp residualvm.exe $a/
cp dists/engine-data/mouse.lab $a/
rm $a.zip
