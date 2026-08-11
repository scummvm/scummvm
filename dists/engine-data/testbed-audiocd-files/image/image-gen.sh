#!/bin/sh
if [ $# == 0 ]; then
  echo "Usage: `basename $0` [image]"
  exit 0
fi

base=$(basename "$1" | cut -d. -f1)
echo "Generating test images base on $1"
#magick $1 -fill white -background black -pointsize 14 +antialias -gravity south -size "98x16" label:'Test 24bpp' -gravity northwest -geometry +112+176 -depth 24 -type TrueColor -composite $base-test.bmp
magick $1 -depth 24 -type TrueColor $base-24bpp.bmp
magick $1 -depth 24 -type TrueColor $base-24bpp.jpg
magick $1 -depth 24 -type TrueColor $base-24bpp.pcx
magick $1 -depth 24 -type TrueColor $base-24bpp.pict
magick $1 -depth 24 -type TrueColor $base-24bpp.png
magick $1 -depth 24 -type TrueColor $base-24bpp.tga
magick $1 -depth 24 -type TrueColor -compress RLE $base-24bpp-rle.tga
magick $1 -depth 8 -type Palette $base-8bpp.bmp
magick $1 -depth 8 -type Palette $base-8bpp.gif
magick $1 -depth 8 -type Palette $base-8bpp.pcx
magick $1 -depth 8 -type Palette $base-8bpp.pict
magick $1 -depth 8 -type Palette $base-8bpp.png
magick $1 -depth 8 -type Palette $base-8bpp.tga
magick $1 -depth 8 -type Palette -compress RLE $base-8bpp-rle.tga
magick $1 -depth 8 -type Grayscale $base-8bpp-grey.png
magick $1 -depth 8 -type Grayscale $base-8bpp-grey.tga
magick $1 -depth 8 -type Grayscale -compress RLE $base-8bpp-grey-rle.tga
magick $1 -depth 1 -type Grayscale $base-1bpp.xbm
magick $1 -channel-fx 'gray=>alpha' $base-32bpp-grayalpha.png

