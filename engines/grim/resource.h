/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRIM_RESOURCE_H
#define GRIM_RESOURCE_H

#include "common/archive.h"
#include "common/array.h"

#include "engines/grim/object.h"

namespace Grim {

class AnimationEmi;
class CMap;
class Costume;
class Font;
class KeyframeAnim;
class Material;
class Model;
class EMIModel;
class LipSync;
class TrackedObject;
class SaveGame;
class Skeleton;
class Sprite;
class EMICostume;
class Lab;
class Actor;
class Overlay;

typedef ObjectPtr<Material> MaterialPtr;
typedef ObjectPtr<Model> ModelPtr;
typedef ObjectPtr<CMap> CMapPtr;
typedef ObjectPtr<KeyframeAnim> KeyframeAnimPtr;
typedef ObjectPtr<Font> FontPtr;
typedef ObjectPtr<LipSync> LipSyncPtr;
typedef ObjectPtr<AnimationEmi> AnimationEmiPtr;

class ResourceLoader {
public:
	ResourceLoader();
	~ResourceLoader();

	CMap *loadColormap(const Common::String &fname);
	Costume *loadCostume(const Common::String &fname, Actor *owner, Costume *prevCost);
	Font *loadFont(const Common::String &fname);
	KeyframeAnim *loadKeyframe(const Common::String &fname);
	Material *loadMaterial(const Common::String &fname, CMap *c, bool clamp);
	Model *loadModel(const Common::String &fname, CMap *c, Model *parent = NULL);
	EMIModel *loadModelEMI(const Common::String &fname, EMICostume *costume);
	LipSync *loadLipSync(const Common::String &fname);
	Skeleton *loadSkeleton(const Common::String &fname);
	Sprite *loadSprite(const Common::String &fname, EMICostume *costume);
	AnimationEmi *loadAnimationEmi(const Common::String &filename);
	Overlay *loadOverlay(const Common::String &filename);
	Common::SeekableReadStream *openNewStreamFile(Common::String fname, bool cache = false) const;

	ModelPtr getModel(const Common::String &fname, CMap *c);
	CMapPtr getColormap(const Common::String &fname);
	KeyframeAnimPtr getKeyframe(const Common::String &fname);
	LipSyncPtr getLipSync(const Common::String &fname);
	AnimationEmiPtr getAnimationEmi(const Common::String &fname);
	void uncacheModel(Model *m);
	void uncacheColormap(CMap *c);
	void uncacheKeyframe(KeyframeAnim *kf);
	void uncacheLipSync(LipSync *l);
	void uncacheAnimationEmi(AnimationEmi *a);

	struct ResourceCache {
		char *fname;
		byte *resPtr;
		uint32 len;
	};

	static Common::String fixFilename(const Common::String &filename, bool append = true);

private:
	Common::SeekableReadStream *loadFile(const Common::String &filename) const;
	Common::SeekableReadStream *getFileFromCache(const Common::String &filename) const;
	ResourceLoader::ResourceCache *getEntryFromCache(const Common::String &filename) const;
	void putIntoCache(const Common::String &fname, byte *res, uint32 len) const;
	void uncache(const char *fname) const;

	mutable Common::Array<ResourceCache> _cache;
	mutable bool _cacheDirty;
	mutable int32 _cacheMemorySize;

	Common::List<EMIModel *> _emiModels;
	Common::List<Model *> _models;
	Common::List<CMap *> _colormaps;
	Common::List<KeyframeAnim *> _keyframeAnims;
	Common::List<LipSync *> _lipsyncs;
	Common::List<AnimationEmi *> _emiAnims;
};

extern ResourceLoader *g_resourceloader;

} // end of namespace Grim

#endif
