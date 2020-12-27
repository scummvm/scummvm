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

#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imageloader.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/controllers/intro_controller.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

bool ImageInfo::hasBlackBackground() {
	return this->_filetype == "image/x-u4raw";
}


class ImageSet {
public:
	~ImageSet();

	Common::String _name;
	Common::String _location;
	Common::String _extends;
	Std::map<Common::String, ImageInfo *> _info;
};

ImageMgr *ImageMgr::_instance = nullptr;

ImageMgr *ImageMgr::getInstance() {
	if (_instance == nullptr) {
		_instance = new ImageMgr();
		_instance->init();
	}
	return _instance;
}

void ImageMgr::destroy() {
	if (_instance != nullptr) {
		delete _instance;
		_instance = nullptr;
	}
}

ImageMgr::ImageMgr() : _baseSet(nullptr), _abyssData(nullptr) {
	settings.addObserver(this);
}

ImageMgr::~ImageMgr() {
	settings.deleteObserver(this);

	for (Std::map<Common::String, ImageSet *>::iterator i = _imageSets.begin(); i != _imageSets.end(); i++)
		delete i->_value;

	delete[] _abyssData;
}

void ImageMgr::init() {
	/*
	 * register the "screen" image representing the entire screen
	 */
	Image *screen = Image::createScreenImage();

	_screenInfo._name = "screen";
	_screenInfo._filename = "";
	_screenInfo._width = screen->width();
	_screenInfo._height = screen->height();
	_screenInfo._depth = 0;
	_screenInfo._prescale = 0;
	_screenInfo._filetype = "";
	_screenInfo._tiles = 0;
	_screenInfo._introOnly = false;
	_screenInfo._transparentIndex = -1;
	_screenInfo._xu4Graphic = false;
	_screenInfo._fixup = FIXUP_NONE;
	_screenInfo._image = screen;

	/*
	 * register all the images declared in the config files
	 */
	const Config *config = Config::getInstance();
	Std::vector<ConfigElement> graphicsConf = config->getElement("graphics").getChildren();
	for (Std::vector<ConfigElement>::iterator conf = graphicsConf.begin(); conf != graphicsConf.end(); conf++) {
		if (conf->getName() == "imageset") {
			ImageSet *set = loadImageSetFromConf(*conf);
			_imageSets[set->_name] = set;

			// all image sets include the "screen" image
			set->_info[_screenInfo._name] = &_screenInfo;
		}
	}

	_imageSetNames.clear();
	for (Std::map<Common::String, ImageSet *>::const_iterator set = _imageSets.begin(); set != _imageSets.end(); set++)
		_imageSetNames.push_back(set->_key);

	update(&settings);
}

ImageSet *ImageMgr::loadImageSetFromConf(const ConfigElement &conf) {
	ImageSet *set;

	set = new ImageSet();
	set->_name = conf.getString("name");
	set->_location = conf.getString("location");
	set->_extends = conf.getString("extends");

	Std::vector<ConfigElement> children = conf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() == "image") {
			ImageInfo *info = loadImageInfoFromConf(*i);
			if (set->_info.contains(info->_name))
				delete set->_info[info->_name];
			set->_info[info->_name] = info;
		}
	}

	return set;
}

ImageInfo *ImageMgr::loadImageInfoFromConf(const ConfigElement &conf) {
	ImageInfo *info;
	static const char *fixupEnumStrings[] = { "none", "intro", "abyss", "abacus", "dungns", "blackTransparencyHack", "fmtownsscreen", nullptr };

	info = new ImageInfo();
	info->_name = conf.getString("name");
	info->_filename = conf.getString("filename");
	info->_width = conf.getInt("width", -1);
	info->_height = conf.getInt("height", -1);
	info->_depth = conf.getInt("depth", -1);
	info->_prescale = conf.getInt("prescale");
	info->_filetype = conf.getString("filetype");
	info->_tiles = conf.getInt("tiles");
	info->_introOnly = conf.getBool("introOnly");
	info->_transparentIndex = conf.getInt("transparentIndex", -1);

	info->_xu4Graphic = conf.getBool("xu4Graphic");
	info->_fixup = static_cast<ImageFixup>(conf.getEnum("fixup", fixupEnumStrings));
	info->_image = nullptr;

	Std::vector<ConfigElement> children = conf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() == "subimage") {
			SubImage *subimage = loadSubImageFromConf(info, *i);
			info->_subImages[subimage->_name] = subimage;
		}
	}

	return info;
}

SubImage *ImageMgr::loadSubImageFromConf(const ImageInfo *info, const ConfigElement &conf) {
	SubImage *subimage;
	static int x = 0,
	           y = 0,
	           last_width = 0,
	           last_height = 0;

	subimage = new SubImage();
	subimage->_name = conf.getString("name");
	subimage->setWidth(conf.getInt("width"));
	subimage->setHeight(conf.getInt("height"));
	subimage->_srcImageName = info->_name;
	if (conf.exists("x") && conf.exists("y")) {
		x = conf.getInt("x");
		y = conf.getInt("y");
		subimage->moveTo(x, y);
	} else {
		// Automatically increment our position through the base image
		x += last_width;
		if (x >= last_width) {
			x = 0;
			y += last_height;
		}

		subimage->moveTo(x, y);
	}

	// "remember" the width and height of this subimage
	last_width = subimage->width();
	last_height = subimage->height();

	return subimage;
}

void ImageMgr::fixupIntro(Image *im, int prescale) {
	const byte *sigData;
	int i, x, y;
	bool alpha = im->isAlphaOn();
	RGBA color;

	sigData = g_intro->getSigData();
	im->alphaOff();
	if (settings._videoType != "VGA-ALLPNG" && settings._videoType != "new") {
		/* ----------------------------
		 * update the position of "and"
		 * ---------------------------- */
		im->drawSubRectOn(im, 148 * prescale, 17 * prescale,
		                  153 * prescale,
		                  17 * prescale,
		                  11 * prescale,
		                  4 * prescale);
		im->drawSubRectOn(im, 159 * prescale, 17 * prescale,
		                  165 * prescale,
		                  18 * prescale,
		                  1 * prescale,
		                  4 * prescale);
		im->drawSubRectOn(im, 160 * prescale, 17 * prescale,
		                  164 * prescale,
		                  17 * prescale,
		                  16 * prescale,
		                  4 * prescale);
		/* ---------------------------------------------
		 * update the position of "Origin Systems, Inc."
		 * --------------------------------------------- */
		im->drawSubRectOn(im, 86 * prescale, 21 * prescale,
		                  88 * prescale,
		                  21 * prescale,
		                  114 * prescale,
		                  9 * prescale);
		im->drawSubRectOn(im, 199 * prescale, 21 * prescale,
		                  202 * prescale,
		                  21 * prescale,
		                  6 * prescale,
		                  9 * prescale);
		im->drawSubRectOn(im, 207 * prescale, 21 * prescale,
		                  208 * prescale,
		                  21 * prescale,
		                  28 * prescale,
		                  9 * prescale);
		/* ---------------------------------------------
		 * update the position of "Ultima IV"
		 * --------------------------------------------- */
		// move this *prior* to moving "present"
		im->drawSubRectOn(im, 59 * prescale, 33 * prescale,
		                  61 * prescale,
		                  33 * prescale,
		                  204 * prescale,
		                  46 * prescale);
		/* ---------------------------------------------
		 * update the position of "Quest of the Avatar"
		 * --------------------------------------------- */
		im->drawSubRectOn(im, 69 * prescale, 80 * prescale,     // quEst
		                  70 * prescale,
		                  80 * prescale,
		                  11 * prescale,
		                  13 * prescale);
		im->drawSubRectOn(im, 82 * prescale, 80 * prescale,     // queST
		                  84 * prescale,
		                  80 * prescale,
		                  27 * prescale,
		                  13 * prescale);
		im->drawSubRectOn(im, 131 * prescale, 80 * prescale,    // oF
		                  132 * prescale,
		                  80 * prescale,
		                  11 * prescale,
		                  13 * prescale);
		im->drawSubRectOn(im, 150 * prescale, 80 * prescale,    // THE
		                  149 * prescale,
		                  80 * prescale,
		                  40 * prescale,
		                  13 * prescale);
		im->drawSubRectOn(im, 166 * prescale, 80 * prescale,    // tHe
		                  165 * prescale,
		                  80 * prescale,
		                  11 * prescale,
		                  13 * prescale);
		im->drawSubRectOn(im, 200 * prescale, 80 * prescale,    // AVATAR
		                  201 * prescale,
		                  80 * prescale,
		                  81 * prescale,
		                  13 * prescale);
		im->drawSubRectOn(im, 227 * prescale, 80 * prescale,    // avAtar
		                  228 * prescale,
		                  80 * prescale,
		                  11 * prescale,
		                  13 * prescale);
	}
	/* -----------------------------------------------------------------------------
	 * copy "present" to new location between "Origin Systems, Inc." and "Ultima IV"
	 * ----------------------------------------------------------------------------- */
	// do this *after* moving "Ultima IV"
	im->drawSubRectOn(im, 132 * prescale, 33 * prescale,
	                  135 * prescale,
	                  0 * prescale,
	                  56 * prescale,
	                  5 * prescale);

	if (alpha) {
		im->alphaOn();
	}

	/* ----------------------------
	 * erase the original "present"
	 * ---------------------------- */
	im->fillRect(135 * prescale, 0 * prescale, 56 * prescale, 5 * prescale, 0, 0, 0);

	/* -------------------------
	 * update the colors for VGA
	 * ------------------------- */
	if (settings._videoType == "VGA") {
		ImageInfo *borderInfo = imageMgr->get(BKGD_BORDERS, true);
//        ImageInfo *charsetInfo = imageMgr->get(BKGD_CHARSET);
		if (!borderInfo)
			error("ERROR 1001: Unable to load the \"%s\" data file", BKGD_BORDERS);

		delete borderInfo->_image;
		borderInfo->_image = nullptr;
		borderInfo = imageMgr->get(BKGD_BORDERS, true);

		im->setPaletteFromImage(borderInfo->_image);

		// update the color of "and" and "present"
		(void)im->setPaletteIndex(15, im->setColor(226, 226, 255));

		// update the color of "Origin Systems, Inc."
		(void)im->setPaletteIndex(9, im->setColor(129, 129, 255));

#ifdef TODO
		borderInfo->_image->save("border.png");
#endif
		// update the border appearance
		borderInfo->_image->alphaOff();
		borderInfo->_image->drawSubRectOn(im, 0, 96, 0, 0, 16, 56);
		for (i = 0; i < 9; i++) {
			borderInfo->_image->drawSubRectOn(im, 16 + (i * 32), 96, 144, 0, 48, 48);
		}
		im->drawSubRectInvertedOn(im, 0, 144, 0, 104, 320, 40);
		im->drawSubRectOn(im, 0, 184, 0, 96, 320, 8);
		borderInfo->_image->alphaOn();

		delete borderInfo->_image;
		borderInfo->_image = nullptr;
	}

	/* -----------------------------
	 * draw "Lord British" signature
	 * ----------------------------- */
	color = im->setColor(0, 255, 255);  // cyan for EGA
	int blue[16] = {255, 250, 226, 226, 210, 194, 161, 161,
	                129,  97,  97,  64,  64,  32,  32,   0
	               };
	i = 0;
	while (sigData[i] != 0) {
		/* (x/y) are unscaled coordinates, i.e. in 320x200 */
		x = sigData[i] + 0x14;
		y = 0xBF - sigData[i + 1];

		if (settings._videoType != "EGA") {
			// yellow gradient
			color = im->setColor(255, (y == 1 ? 250 : 255), blue[y]);
		}

		im->fillRect(x * prescale, y * prescale,
		             2 * prescale, prescale,
		             color.r, color.g, color.b);
		i += 2;
	}

	/* --------------------------------------------------------------
	 * draw the red line between "Origin Systems, Inc." and "present"
	 * -------------------------------------------------------------- */
	/* we're still working with an unscaled surface */
	if (settings._videoType != "EGA") {
		color = im->setColor(0, 0, 161);    // dark blue
	} else {
		color = im->setColor(128, 0, 0);    // dark red for EGA
	}
	for (i = 84; i < 236; i++)  // 152 px wide
		im->fillRect(i * prescale, 31 * prescale,
		             prescale, prescale,
		             color.r, color.g, color.b);
}

void ImageMgr::fixupAbyssVision(Image *im, int prescale) {
	// Ignore fixups for xu4 PNG images - they're already correct
	if (im->isIndexed())
		return;

	/*
	 * Each VGA vision components must be XORed with all the previous
	 * vision components to get the actual image.
	 */
	if (_abyssData) {
		for (int y = 0; y < im->height(); y++) {
			for (int x = 0; x < im->width(); x++) {
				uint index;
				im->getPixelIndex(x, y, index);
				index ^= _abyssData[y * im->width() + x];
				im->putPixelIndex(x, y, index);
			}
		}
	} else {
		_abyssData = new uint[im->width() * im->height()];
	}

	for (int y = 0; y < im->height(); y++) {
		for (int x = 0; x < im->width(); x++) {
			uint index;
			im->getPixelIndex(x, y, index);
			_abyssData[y * im->width() + x] = index;
		}
	}
}

void ImageMgr::fixupAbacus(Image *im, int prescale) {

	/*
	 * surround each bead with a row green pixels to avoid artifacts
	 * when scaling
	 */

	im->fillRect(7 * prescale, 186 * prescale, prescale, 14 * prescale, 0, 255, 80); /* green */
	im->fillRect(16 * prescale, 186 * prescale, prescale, 14 * prescale, 0, 255, 80); /* green */
	im->fillRect(8 * prescale, 186 * prescale, prescale * 8, prescale, 0, 255, 80); /* green */
	im->fillRect(8 * prescale, 199 * prescale, prescale * 8, prescale, 0, 255, 80); /* green */

	im->fillRect(23 * prescale, 186 * prescale, prescale, 14 * prescale, 0, 255, 80); /* green */
	im->fillRect(32 * prescale, 186 * prescale, prescale, 14 * prescale, 0, 255, 80); /* green */
	im->fillRect(24 * prescale, 186 * prescale, prescale * 8, prescale, 0, 255, 80); /* green */
	im->fillRect(24 * prescale, 199 * prescale, prescale * 8, prescale, 0, 255, 80); /* green */
}

void ImageMgr::fixupDungNS(Image *im, int prescale) {
	for (int y = 0; y < im->height(); y++) {
		for (int x = 0; x < im->width(); x++) {
			uint index;
			im->getPixelIndex(x, y, index);
			if (index == 1)
				im->putPixelIndex(x, y, 2);
			else if (index == 2)
				im->putPixelIndex(x, y, 1);
		}
	}
}

void ImageMgr::fixupFMTowns(Image *im, int prescale) {
	for (int y = 20; y < im->height(); y++) {
		for (int x = 0; x < im->width(); x++) {
			uint index;
			im->getPixelIndex(x, y, index);
			im->putPixelIndex(x, y - 20, index);
		}
	}
}

ImageSet *ImageMgr::getSet(const Common::String &setname) {
	Std::map<Common::String, ImageSet *>::iterator i = _imageSets.find(setname);
	if (i != _imageSets.end())
		return i->_value;
	else
		return nullptr;
}

ImageInfo *ImageMgr::getInfo(const Common::String &name) {
	return getInfoFromSet(name, _baseSet);
}

ImageInfo *ImageMgr::getInfoFromSet(const Common::String &name, ImageSet *imageset) {
	if (!imageset)
		return nullptr;

	/* if the image set contains the image we want, AND IT EXISTS we are done */
	Std::map<Common::String, ImageInfo *>::iterator i = imageset->_info.find(name);
	if (i != imageset->_info.end())
		if (imageExists(i->_value))
			return i->_value;

	/* otherwise if this image set extends another, check the base image set */
	while (imageset->_extends != "") {
		imageset = getSet(imageset->_extends);
		return getInfoFromSet(name, imageset);
	}

	//warning("Searched recursively from imageset %s through to %s and couldn't find %s", baseSet->name.c_str(), imageset->name.c_str(), name.c_str());
	return nullptr;
}

Common::String ImageMgr::guessFileType(const Common::String &filename) {
	if (filename.size() >= 4 && filename.hasSuffixIgnoreCase(".png")) {
		return "image/png";
	} else {
		return "";
	}
}

bool ImageMgr::imageExists(ImageInfo *info) {
	if (info->_filename == "") //If it is an abstract image like "screen"
		return true;
	Common::File *file = getImageFile(info);
	if (file) {
		delete file;
		return true;
	}
	return false;
}


Common::File *ImageMgr::getImageFile(ImageInfo *info) {
	Common::String filename = info->_filename;

	if (filename.empty())
		return nullptr;

	Common::File *file = new Common::File();
	if (!info->_xu4Graphic) {
		// It's a file in the game folder
		if (file->open(filename))
			return file;
	}

	if (file->open("data/graphics/" + filename))
		return file;

	delete file;
	return nullptr;
}

ImageInfo *ImageMgr::get(const Common::String &name, bool returnUnscaled) {
	ImageInfo *info = getInfo(name);
	if (!info)
		return nullptr;

	/* return if already loaded */
	if (info->_image != nullptr)
		return info;

	Common::File *file = getImageFile(info);
	Image *unscaled = nullptr;
	if (file) {
		if (info->_filetype.empty())
			info->_filetype = guessFileType(info->_filename);
		Common::String filetype = info->_filetype;
		ImageLoader *loader = g_ultima->_imageLoaders->getLoader(filetype);
		if (loader == nullptr) {
			warning("can't find loader to load image \"%s\" with type \"%s\"", info->_filename.c_str(), filetype.c_str());
		} else {
			unscaled = loader->load(*file, info->_width, info->_height, info->_depth);
			if (info->_width == -1) {
				// Write in the values for later use.
				info->_width = unscaled->width();
				info->_height = unscaled->height();
				// ###            info->depth = ???
			}
		}

		delete file;
	} else {
		warning("Failed to open file %s for reading.", info->_filename.c_str());
		return nullptr;
	}

	if (unscaled == nullptr)
		return nullptr;

	if (info->_transparentIndex != -1)
		unscaled->setTransparentIndex(info->_transparentIndex);

	if (info->_prescale == 0)
		info->_prescale = 1;

	/*
	 * fixup the image before scaling it
	 */
	switch (info->_fixup) {
	case FIXUP_NONE:
		break;
	case FIXUP_INTRO:
		fixupIntro(unscaled, info->_prescale);
		break;
	case FIXUP_ABYSS:
		fixupAbyssVision(unscaled, info->_prescale);
		break;
	case FIXUP_ABACUS:
		fixupAbacus(unscaled, info->_prescale);
		break;
	case FIXUP_DUNGNS:
		fixupDungNS(unscaled, info->_prescale);
		break;
	case FIXUP_FMTOWNSSCREEN:
		fixupFMTowns(unscaled, info->_prescale);
		break;
	case FIXUP_BLACKTRANSPARENCYHACK:
		//Apply transparency shadow hack to ultima4 ega and vga upgrade classic graphics.
		Image *unscaled_original = unscaled;
		unscaled = Image::duplicate(unscaled);
		delete unscaled_original;
		if (Settings::getInstance()._enhancements && Settings::getInstance()._enhancementsOptions._u4TileTransparencyHack) {
			int transparency_shadow_size = Settings::getInstance()._enhancementsOptions._u4TrileTransparencyHackShadowBreadth;
			int black_index = 0;
			int opacity = Settings::getInstance()._enhancementsOptions._u4TileTransparencyHackPixelShadowOpacity;

			int frames = info->_tiles;
			for (int f = 0; f < frames; ++f)
				unscaled->performTransparencyHack(black_index, frames, f, transparency_shadow_size, opacity);
		}
		break;
	}

	if (returnUnscaled) {
		info->_image = unscaled;
		return info;
	}

	int imageScale = settings._scale;
	if ((settings._scale % info->_prescale) != 0) {
		int orig_scale = settings._scale;
		settings._scale = info->_prescale;
		settings.write();
		error("image %s is prescaled to an incompatible size: %d\nResetting the scale to %d. Sorry about the inconvenience, please restart.", info->_filename.c_str(), orig_scale, settings._scale);
	}
	imageScale /= info->_prescale;

	info->_image = g_screen->screenScale(unscaled, imageScale, info->_tiles, 1);

	delete unscaled;
	return info;
}

SubImage *ImageMgr::getSubImage(const Common::String &name) {
	Common::String setname;

	ImageSet *set = _baseSet;

	while (set != nullptr) {
		for (Std::map<Common::String, ImageInfo *>::iterator i = set->_info.begin(); i != set->_info.end(); i++) {
			ImageInfo *info = (ImageInfo *) i->_value;
			Std::map<Common::String, SubImage *>::iterator j = info->_subImages.find(name);
			if (j != info->_subImages.end())
				return j->_value;
		}

		set = getSet(set->_extends);
	}

	return nullptr;
}

void ImageMgr::freeIntroBackgrounds() {
	for (Std::map<Common::String, ImageSet *>::iterator i = _imageSets.begin(); i != _imageSets.end(); i++) {
		ImageSet *set = i->_value;
		for (Std::map<Common::String, ImageInfo *>::iterator j = set->_info.begin(); j != set->_info.end(); j++) {
			ImageInfo *info = j->_value;
			if (info->_image != nullptr && info->_introOnly) {
				delete info->_image;
				info->_image = nullptr;
			}
		}
	}
}

const Std::vector<Common::String> &ImageMgr::getSetNames() {
	return _imageSetNames;
}

void ImageMgr::update(Settings *newSettings) {
	Common::String setname;

	setname = newSettings->_videoType;

	_baseSet = getSet(setname);
}

ImageSet::~ImageSet() {
	for (Std::map<Common::String, ImageInfo *>::iterator i = _info.begin(); i != _info.end(); i++) {
		ImageInfo *imageInfo = i->_value;
		if (imageInfo->_name != "screen")
			delete imageInfo;
	}
}

ImageInfo::~ImageInfo() {
	for (Std::map<Common::String, SubImage *>::iterator i = _subImages.begin(); i != _subImages.end(); i++)
		delete i->_value;
	if (_image != nullptr)
		delete _image;
}

} // End of namespace Ultima4
} // End of namespace Ultima
