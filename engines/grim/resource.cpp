/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "gui/error.h"

#include "engines/grim/resource.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/keyframe.h"
#include "engines/grim/material.h"
#include "engines/grim/grim.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lab.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/model.h"
#include "engines/grim/sprite.h"
#include "engines/grim/inputdialog.h"
#include "engines/grim/debug.h"
#include "engines/grim/emi/animationemi.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/remastered/overlay.h"
#include "engines/grim/patchr.h"
#include "engines/grim/md5check.h"
#include "engines/grim/update/update.h"

#include "common/algorithm.h"
#include "common/zlib.h"
#include "common/memstream.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace Grim {

ResourceLoader *g_resourceloader = nullptr;

class LabListComperator {
	const Common::String _labName;
public:
	LabListComperator() {}
	LabListComperator(const Common::String &ln) : _labName(ln) {}

	bool operator()(const Common::ArchiveMemberPtr &l) {
		return _labName.compareToIgnoreCase(l->getName()) == 0;
	}

	bool operator()(const Common::ArchiveMemberPtr &l, const Common::ArchiveMemberPtr &r) {
		return (l->getName().compareToIgnoreCase(r->getName()) > 0);
	}
};

ResourceLoader::ResourceLoader() {
	_cacheDirty = false;
	_cacheMemorySize = 0;

	Lab *l;
	Common::ArchiveMemberList files, updFiles;

	// Load the update from the executable, if needed
	const char *updateFilename = nullptr;
	if ((g_grim->getGameType() == GType_GRIM && !g_grim->isRemastered())
	    || g_grim->getGameType() == GType_MONKEY4) {
		updateFilename = g_grim->getUpdateFilename();
	}
	if (updateFilename) {
		Common::File *updStream = new Common::File();
		if (updStream && updStream->open(updateFilename)) {
			Common::Archive *update = loadUpdateArchive(updStream);
			if (update)
				SearchMan.add("update", update, 1);
		} else
			delete updStream;

		// Check if the update has been correctly loaded
		if (!SearchMan.hasArchive("update")) {
			Common::U32String errorMessage;
			Common::String urlForPatchDownload = Common::String::format("https://downloads.scummvm.org/frs/extras/patches/%s",
			                                     (g_grim->getGameType() == GType_GRIM)? "gfupd101.exe" : "");
			if (g_grim->getGameType() == GType_GRIM) {
				errorMessage = Common::U32String::format(_("The original patch of Grim Fandango\n"
								"is missing. Please download it from\n"
								"%s\n"
								"and put it in the game data files directory"), urlForPatchDownload.c_str());
			} else if (g_grim->getGameType() == GType_MONKEY4) {
				errorMessage = Common::U32String::format(_("The original patch of Escape from Monkey Island is missing. \n"
								"Please download it from %s\n"
								"and put it in the game data files directory.\n"
								"Pay attention to download the correct version according to the game's language!"), urlForPatchDownload.c_str());
			}

			GUIErrorMessageWithURL(errorMessage, urlForPatchDownload.c_str());
			error("%s not found", updateFilename);
		}
	}

	if (g_grim->getGameType() == GType_GRIM) {
		if (g_grim->getGameFlags() & ADGF_DEMO) {
			SearchMan.listMatchingMembers(files, "gfdemo01.lab");
			SearchMan.listMatchingMembers(files, "gdemo001.lab"); // For the english demo with video.
			SearchMan.listMatchingMembers(files, "grimdemo.mus");
			SearchMan.listMatchingMembers(files, "sound001.lab");
			SearchMan.listMatchingMembers(files, "voice001.lab");
		} else {
			if (!SearchMan.hasFile("grim-patch.lab"))
				error("%s", "grim-patch.lab not found");

			SearchMan.listMatchingMembers(files, "grim-patch.lab");
			SearchMan.listMatchingMembers(files, "data005.lab");
			SearchMan.listMatchingMembers(files, "data004.lab");
			SearchMan.listMatchingMembers(files, "data003.lab");
			SearchMan.listMatchingMembers(files, "data002.lab");
			SearchMan.listMatchingMembers(files, "data001.lab");
			SearchMan.listMatchingMembers(files, "data000.lab");
			SearchMan.listMatchingMembers(files, "movie??.lab");
			SearchMan.listMatchingMembers(files, "vox????.lab");
			SearchMan.listMatchingMembers(files, "year?mus.lab");
			SearchMan.listMatchingMembers(files, "local.lab");
			SearchMan.listMatchingMembers(files, "credits.lab");

			if (g_grim->isRemastered()) {
				SearchMan.listMatchingMembers(files, "commentary.lab");
				SearchMan.listMatchingMembers(files, "images.lab");
			}
			// Sort the archives in order to ensure that they are loaded with the correct order
			Common::sort(files.begin(), files.end(), LabListComperator());

			// Check the presence of datausr.lab and if the user wants to load it.
			// In this case put it in the top of the list
			const char *datausr_name = "datausr.lab";
			if (SearchMan.hasFile(datausr_name) && ConfMan.getBool("datausr_load")) {
				warning("%s", "Loading datausr.lab. Please note that the ScummVM team doesn't provide support for using such patches");
				files.push_front(SearchMan.getMember(datausr_name));
			}
		}
	} else if (g_grim->getGameType() == GType_MONKEY4) {
		const char *emi_patches_filename = "monkey4-patch.m4b";
		if (!SearchMan.hasFile(emi_patches_filename))
			error("%s not found", emi_patches_filename);

		SearchMan.listMatchingMembers(files, emi_patches_filename);

		if (g_grim->getGameFlags() & ADGF_DEMO) {
			SearchMan.listMatchingMembers(files, "lip.lab");
			SearchMan.listMatchingMembers(files, "MagDemo.lab");
			SearchMan.listMatchingMembers(files, "tile.lab");
			SearchMan.listMatchingMembers(files, "voice.lab");
		} else {
			// Keep i9n.m4b before patch.m4b for a better efficiency
			// in decompressing from Monkey Update.exe
			SearchMan.listMatchingMembers(files, "i9n.m4b");
			SearchMan.listMatchingMembers(files, "patch.m4b");
			SearchMan.listMatchingMembers(files, "art???.m4b");
			SearchMan.listMatchingMembers(files, "lip.m4b");
			SearchMan.listMatchingMembers(files, "local.m4b");
			SearchMan.listMatchingMembers(files, "sfx.m4b");
			SearchMan.listMatchingMembers(files, "voice???.m4b");
			SearchMan.listMatchingMembers(files, "music?.m4b");

			if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
				SearchMan.listMatchingMembers(files, "???.m4b");
			}

			// Check the presence of datausr.m4b and if the user wants to load it.
			// In this case put it in the top of the list
			const char *datausr_name = "datausr.m4b";
			if (SearchMan.hasFile(datausr_name) && ConfMan.getBool("datausr_load")) {
				warning("%s", "Loading datausr.m4b. Please note that the ScummVM team doesn't provide support for using such patches");
				files.push_front(SearchMan.getMember(datausr_name));
			}
		}
	}

	if (files.empty())
		error("%s", "Cannot find game data - check configuration file");

	// Load labs
	int priority = files.size();
	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		Common::String filename = (*x)->getName();
		filename.toLowercase();

		// Avoid duplicates
		if (SearchMan.hasArchive(filename))
			continue;

		l = new Lab();
		// Caching "local.m4b" to speed up the launch of the mac version,
		// we _COULD_ protect this with a platform check, but the file isn't
		// really big anyhow...
		bool useCache = (filename == "local.m4b");
		if (l->open(filename, useCache))
			SearchMan.add(filename, l, priority--, true);
		else
			delete l;
	}

	files.clear();
}

template<typename T>
void clearList(Common::List<T> &list) {
	while (!list.empty()) {
		T p = list.front();
		list.erase(list.begin());
		delete p;
	}
}

ResourceLoader::~ResourceLoader() {
	for (Common::Array<ResourceCache>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		ResourceCache &r = *i;
		delete[] r.fname;
		delete[] r.resPtr;
	}
	clearList(_models);
	clearList(_colormaps);
	clearList(_keyframeAnims);
	clearList(_lipsyncs);
	MD5Check::clear();
}

static int sortCallback(const void *entry1, const void *entry2) {
	return scumm_stricmp(((const ResourceLoader::ResourceCache *)entry1)->fname, ((const ResourceLoader::ResourceCache *)entry2)->fname);
}

Common::SeekableReadStream *ResourceLoader::getFileFromCache(const Common::String &filename) const {
	ResourceLoader::ResourceCache *entry = getEntryFromCache(filename);
	if (!entry)
		return nullptr;

	return new Common::MemoryReadStream(entry->resPtr, entry->len);
}

ResourceLoader::ResourceCache *ResourceLoader::getEntryFromCache(const Common::String &filename) const {
	if (_cache.empty())
		return nullptr;

	if (_cacheDirty) {
		qsort(_cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
		_cacheDirty = false;
	}

	ResourceCache key;
	key.fname = const_cast<char *>(filename.c_str());

	return (ResourceLoader::ResourceCache *)bsearch(&key, _cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
}

Common::SeekableReadStream *ResourceLoader::loadFile(const Common::String &filename) const {
	Common::SeekableReadStream *rs = nullptr;
	if (SearchMan.hasFile(filename))
		rs = SearchMan.createReadStreamForMember(filename);
	else
		return nullptr;

	rs = wrapPatchedFile(rs, filename);
	return rs;
}

Common::SeekableReadStream *ResourceLoader::openNewStreamFile(Common::String fname, bool cache) const {
	Common::SeekableReadStream *s;
	fname.toLowercase();

	if (cache) {
		s = getFileFromCache(fname);
		if (!s) {
			s = loadFile(fname);
			if (!s)
				return nullptr;

			uint32 size = s->size();
			byte *buf = new byte[size];
			s->read(buf, size);
			putIntoCache(fname, buf, size);
			delete s;
			s = new Common::MemoryReadStream(buf, size);
		}
	} else {
		s = loadFile(fname);
	}
	// This will only have an effect if the stream is actually compressed.
	return Common::wrapCompressedReadStream(s);
}

void ResourceLoader::putIntoCache(const Common::String &fname, byte *res, uint32 len) const {
	ResourceCache entry;
	entry.resPtr = res;
	entry.len = len;
	entry.fname = new char[fname.size() + 1];
	strcpy(entry.fname, fname.c_str());
	_cacheMemorySize += len;
	_cache.push_back(entry);
	_cacheDirty = true;
}

CMap *ResourceLoader::loadColormap(const Common::String &filename) {
	Common::SeekableReadStream *stream = openNewStreamFile(filename.c_str());
	if (!stream) {
		error("Could not find colormap %s", filename.c_str());
	}

	CMap *result = new CMap(filename, stream);
	_colormaps.push_back(result);
	delete stream;

	return result;
}

Common::String ResourceLoader::fixFilename(const Common::String &filename, bool append) {
	Common::String fname(filename);
	if (g_grim->getGameType() == GType_MONKEY4) {
		int len = fname.size();
		for (int i = 0; i < len; i++) {
			if (fname[i] == '\\') {
				fname.setChar('/', i);
			}
		}
		// Append b to end of filename for EMI
		if (append)
			fname += "b";
	}
	return fname;
}

Costume *ResourceLoader::loadCostume(const Common::String &filename, Actor *owner, Costume *prevCost) {
	Common::String fname = fixFilename(filename);
	fname.toLowercase();

	Common::SeekableReadStream *stream = openNewStreamFile(fname.c_str(), true);
	if (!stream) {
		error("Could not find costume \"%s\"", filename.c_str());
	}
	Costume *result;
	if (g_grim->getGameType() == GType_MONKEY4) {
		result = new EMICostume(filename, owner, prevCost);
	} else {
		result = new Costume(filename, owner, prevCost);
	}
	result->load(stream);
	delete stream;

	return result;
}

Font *ResourceLoader::loadFont(const Common::String &filename) {
	Common::SeekableReadStream *stream;

	if (g_grim->getGameType() == GType_GRIM && g_grim->isRemastered()) {
		Common::String name = "FontsHD/" + filename + ".txt";
		stream = openNewStreamFile(name, true);
		if (stream) {
			Common::String line = stream->readLine();
			Common::String font;
			Common::String size;
			for (uint i = 0; i < line.size(); ++i) {
				if (line[i] == ' ') {
					font = "FontsHD/" + Common::String(line.c_str(), i);
					size = Common::String(line.c_str() + i + 1, line.size() - i - 2);
				}
			}

			int s = atoi(size.c_str());
			delete stream;
			stream = openNewStreamFile(font.c_str(), true);
			FontTTF *result = new FontTTF();
			result->loadTTF(font, stream, s);
			return result;
		}
	}

	stream = openNewStreamFile(filename.c_str(), true);
	if (!stream)
		error("Could not find font file %s", filename.c_str());

	Font *result = new Font();
	result->load(filename, stream);
	delete stream;

	return result;
}

KeyframeAnim *ResourceLoader::loadKeyframe(const Common::String &filename) {
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(filename.c_str());
	if (!stream)
		error("Could not find keyframe file %s", filename.c_str());

	KeyframeAnim *result = new KeyframeAnim(filename, stream);
	_keyframeAnims.push_back(result);
	delete stream;

	return result;
}

LipSync *ResourceLoader::loadLipSync(const Common::String &filename) {
	LipSync *result;
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(filename.c_str());
	if (!stream)
		return nullptr;

	result = new LipSync(filename, stream);

	// Some lipsync files have no data
	if (result->isValid())
		_lipsyncs.push_back(result);
	else {
		delete result;
		result = nullptr;
	}
	delete stream;

	return result;
}

Material *ResourceLoader::loadMaterial(const Common::String &filename, CMap *c, bool clamp) {
	Common::String fname = fixFilename(filename, false);
	fname.toLowercase();
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str(), true);
	if (!stream && !filename.hasPrefix("specialty")) {
		// FIXME: EMI demo references files that aren't included. Return a known material.
		// This should be fixed in the data files instead.
		if (g_grim->getGameType() == GType_MONKEY4 && g_grim->getGameFlags() & ADGF_DEMO) {
			const Common::String replacement("fx/candle.sprb");
			warning("Could not find material %s, using %s instead", filename.c_str(), replacement.c_str());
			return loadMaterial(replacement, nullptr, clamp);
		} else {
			error("Could not find material %s", filename.c_str());
		}
	}

	Material *result = new Material(fname, stream, c, clamp);
	delete stream;

	return result;
}

Model *ResourceLoader::loadModel(const Common::String &filename, CMap *c, Model *parent) {
	Common::String fname = fixFilename(filename);
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str());
	if (!stream)
		error("Could not find model %s", filename.c_str());

	Model *result = new Model(filename, stream, c, parent);
	_models.push_back(result);
	delete stream;

	return result;
}

EMIModel *ResourceLoader::loadModelEMI(const Common::String &filename, EMICostume *costume) {
	Common::String fname = fixFilename(filename);
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str());
	if (!stream) {
		warning("Could not find model %s", filename.c_str());
		return nullptr;
	}

	EMIModel *result = new EMIModel(filename, stream, costume);
	_emiModels.push_back(result);
	delete stream;

	return result;
}

Skeleton *ResourceLoader::loadSkeleton(const Common::String &filename) {
	Common::String fname = fixFilename(filename);
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str(), true);
	if (!stream) {
		warning("Could not find skeleton %s", filename.c_str());
		return nullptr;
	}

	Skeleton *result = new Skeleton(filename, stream);
	delete stream;

	return result;
}

Sprite *ResourceLoader::loadSprite(const Common::String &filename, EMICostume *costume) {
	assert(g_grim->getGameType() == GType_MONKEY4);
	Common::SeekableReadStream *stream;

	const Common::String fname = fixFilename(filename, true);

	stream = openNewStreamFile(fname.c_str(), true);
	if (!stream) {
		warning("Could not find sprite %s", fname.c_str());
		return nullptr;
	}

	Sprite *result = new Sprite();
	result->loadBinary(stream, costume);
	delete stream;

	return result;
}

AnimationEmi *ResourceLoader::loadAnimationEmi(const Common::String &filename) {
	Common::String fname = fixFilename(filename);
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str(), true);
	if (!stream) {
		warning("Could not find animation %s", filename.c_str());
		return nullptr;
	}

	AnimationEmi *result = new AnimationEmi(filename, stream);
	_emiAnims.push_back(result);
	delete stream;

	return result;
}

Overlay *ResourceLoader::loadOverlay(const Common::String &filename) {
	Common::String fname = fixFilename(filename);
	Common::SeekableReadStream *stream;

	stream = openNewStreamFile(fname.c_str(), true);
	if (!stream) {
		warning("Could not find overlay %s", filename.c_str());
		return nullptr;
	}

	Overlay *result = new Overlay(filename, stream);
	delete stream;

	return result;
}

void ResourceLoader::uncache(const char *filename) const {
	Common::String fname = filename;
	fname.toLowercase();

	if (_cacheDirty) {
		qsort(_cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
		_cacheDirty = false;
	}

	for (unsigned int i = 0; i < _cache.size(); i++) {
		if (fname.compareTo(_cache[i].fname) == 0) {
			delete[] _cache[i].fname;
			_cacheMemorySize -= _cache[i].len;
			delete[] _cache[i].resPtr;
			_cache.remove_at(i);
			_cacheDirty = true;
		}
	}
}

void ResourceLoader::uncacheModel(Model *m) {
	_models.remove(m);
}

void ResourceLoader::uncacheColormap(CMap *c) {
	_colormaps.remove(c);
}

void ResourceLoader::uncacheKeyframe(KeyframeAnim *k) {
	_keyframeAnims.remove(k);
}

void ResourceLoader::uncacheLipSync(LipSync *s) {
	_lipsyncs.remove(s);
}

void ResourceLoader::uncacheAnimationEmi(AnimationEmi *a) {
	_emiAnims.remove(a);
}

ModelPtr ResourceLoader::getModel(const Common::String &fname, CMap *c) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<Model *>::const_iterator i = _models.begin(); i != _models.end(); ++i) {
		Model *m = *i;
		if (filename == m->getFilename() && *m->getCMap() == *c) {
			return m;
		}
	}

	return loadModel(fname, c);
}

CMapPtr ResourceLoader::getColormap(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<CMap *>::const_iterator i = _colormaps.begin(); i != _colormaps.end(); ++i) {
		CMap *c = *i;
		if (filename.equals(c->_fname)) {
			return c;
		}
	}

	return loadColormap(fname);
}

KeyframeAnimPtr ResourceLoader::getKeyframe(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<KeyframeAnim *>::const_iterator i = _keyframeAnims.begin(); i != _keyframeAnims.end(); ++i) {
		KeyframeAnim *k = *i;
		if (filename == k->getFilename()) {
			return k;
		}
	}

	return loadKeyframe(fname);
}

LipSyncPtr ResourceLoader::getLipSync(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<LipSync *>::const_iterator i = _lipsyncs.begin(); i != _lipsyncs.end(); ++i) {
		LipSync *l = *i;
		if (filename == l->getFilename()) {
			return l;
		}
	}

	return loadLipSync(fname);
}

AnimationEmiPtr ResourceLoader::getAnimationEmi(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<AnimationEmi *>::const_iterator i = _emiAnims.begin(); i != _emiAnims.end(); ++i) {
		AnimationEmi *a = *i;
		if (filename == a->getFilename()) {
			return a;
		}
	}

	return loadAnimationEmi(fname);
}

} // end of namespace Grim
