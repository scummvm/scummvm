/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_GFX_IMAGEMGR_H
#define ULTIMA4_GFX_IMAGEMGR_H

#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/shared/std/containers.h"

namespace Common {
class File;
}

namespace Ultima {
namespace Ultima4 {

class ConfigElement;
class Debug;
class ImageSet;
class Settings;

/*
 * The image manager is responsible for loading and keeping track of
 * the various images.
 */

#define BKGD_SHAPES "tiles"
#define BKGD_CHARSET "charset"
#define BKGD_BORDERS "borders"
#define BKGD_INTRO "title"
#define BKGD_OPTIONS_TOP "options_top"
#define BKGD_OPTIONS_BTM "options_btm"
#define BKGD_TREE "tree"
#define BKGD_PORTAL "portal"
#define BKGD_OUTSIDE "outside"
#define BKGD_INSIDE "inside"
#define BKGD_WAGON "wagon"
#define BKGD_GYPSY "gypsy"
#define BKGD_ABACUS "abacus"
#define BKGD_HONCOM "honcom"
#define BKGD_VALJUS "valjus"
#define BKGD_SACHONOR "sachonor"
#define BKGD_SPIRHUM "spirhum"
#define BKGD_ANIMATE "beasties"
#define BKGD_KEY "key"
#define BKGD_HONESTY "honesty"
#define BKGD_COMPASSN "compassn"
#define BKGD_VALOR "valor"
#define BKGD_JUSTICE "justice"
#define BKGD_SACRIFIC "sacrific"
#define BKGD_HONOR "honor"
#define BKGD_SPIRIT "spirit"
#define BKGD_HUMILITY "humility"
#define BKGD_TRUTH "truth"
#define BKGD_LOVE "love"
#define BKGD_COURAGE "courage"
#define BKGD_STONCRCL "stoncrcl"
#define BKGD_RUNE_INF "rune0"
#define BKGD_SHRINE_HON "rune1"
#define BKGD_SHRINE_COM "rune2"
#define BKGD_SHRINE_VAL "rune3"
#define BKGD_SHRINE_JUS "rune4"
#define BKGD_SHRINE_SAC "rune5"
#define BKGD_SHRINE_HNR "rune6"
#define BKGD_SHRINE_SPI "rune7"
#define BKGD_SHRINE_HUM "rune8"
#define BKGD_GEMTILES "gemtiles"

enum ImageFixup {
	FIXUP_NONE,
	FIXUP_INTRO,
	FIXUP_ABYSS,
	FIXUP_ABACUS,
	FIXUP_DUNGNS,
	FIXUP_BLACKTRANSPARENCYHACK,
	FIXUP_FMTOWNSSCREEN
};

/**
 * Image meta info.
 */
class ImageInfo {
public:
	~ImageInfo();

	Common::String _name;
	Common::String _filename;
	int _width, _height, _depth;
	int _prescale;
	Common::String _filetype;
	int _tiles;                  /**< used to scale the without bleeding colors between adjacent tiles */
	bool _introOnly;             /**< whether can be freed after the intro */
	int _transparentIndex;       /**< color index to consider transparent */
	bool _xu4Graphic;            /**< an original xu4 graphic not part of u4dos or the VGA upgrade */
	ImageFixup _fixup;           /**< a routine to do miscellaneous fixes to the image */
	Image *_image;               /**< the image we're describing */
	Std::map<Common::String, SubImage *> _subImages;

	bool hasBlackBackground();
};

/**
 * The image manager singleton that keeps track of all the images.
 */
class ImageMgr : Observer<Settings *> {
public:
	static ImageMgr *getInstance();
	static void destroy();

	/**
	 * Load in a background image from a ".ega" file.
	 */
	ImageInfo *get(const Common::String &name, bool returnUnscaled = false);

	/**
	 * Returns information for the given image set.
	 */
	SubImage *getSubImage(const Common::String &name);

	/**
	 * Free up any background images used only in the animations.
	 */
	void freeIntroBackgrounds();
	const Std::vector<Common::String> &getSetNames();
	Common::File *getImageFile(ImageInfo *info);
	bool imageExists(ImageInfo *info);

private:
	ImageMgr();
	~ImageMgr();
	void init();

	ImageSet *loadImageSetFromConf(const ConfigElement &conf);
	ImageInfo *loadImageInfoFromConf(const ConfigElement &conf);
	SubImage *loadSubImageFromConf(const ImageInfo *info, const ConfigElement &conf);

	/**
	 * Returns information for the given image set.
	 */
	ImageSet *getSet(const Common::String &setname);

	/**
	 * Returns image information for the current image set.
	 */
	ImageInfo *getInfo(const Common::String &name);

	/**
	 * Returns information for the given image set.
	 */
	ImageInfo *getInfoFromSet(const Common::String &name, ImageSet *set);

	Common::String guessFileType(const Common::String &filename);

	void fixupIntro(Image *im, int prescale);
	void fixupAbyssVision(Image *im, int prescale);
	void fixupAbacus(Image *im, int prescale);

	/**
	 * Swap blue and green for the dungeon walls when facing north or
	 * south.
	 */
	void fixupDungNS(Image *im, int prescale);

	/**
	 * The FMTowns images have a different screen dimension. This moves them
	 * up to what xu4 is accustomed to. south.
	 */
	void fixupFMTowns(Image *im, int prescale);

	/**
	 * Find the new base image set when settings have changed.
	 */
	void update(Settings *newSettings);

	static ImageMgr *_instance;
	Std::map<Common::String, ImageSet *> _imageSets;
	Std::vector<Common::String> _imageSetNames;
	ImageSet *_baseSet;
	ImageInfo _screenInfo;
	uint *_abyssData;
};

#define imageMgr (ImageMgr::getInstance())

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
