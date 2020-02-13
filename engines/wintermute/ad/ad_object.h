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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_ADOBJECT_H
#define WINTERMUTE_ADOBJECT_H

#include "engines/wintermute/ad/ad_types.h"
#include "engines/wintermute/base/base_object.h"

namespace Wintermute {

class AdWaypointGroup;
class AdRegion;
class AdSentence;
class BaseFont;
class BaseRegion;
class AdInventory;
class PartEmitter;

#define MAX_NUM_REGIONS 10

class AdObject : public BaseObject {
public:
	virtual PartEmitter *createParticleEmitter(bool followParent = false, int offsetX = 0, int offsetY = 0);
	virtual bool updatePartEmitter();

	bool invalidateCurrRegions();
	AdRegion *_stickRegion;
	bool _sceneIndependent;

	bool updateBlockRegion();

	bool getExtendedFlag(const char *flagName) override;
	bool resetSoundPan() override;
	bool updateSounds() override;
	bool reset();
	DECLARE_PERSISTENT(AdObject, BaseObject)
	virtual void talk(const char *text, const char *sound = nullptr, uint32 duration = 0, const char *stances = nullptr, TTextAlign align = TAL_CENTER);
	int32 getHeight() override;

	bool setFont(const char *filename);
	bool update() override;
	bool display() override;

	bool _drawn;
	bool _active;
	virtual bool playAnim(const char *filename);

	TObjectType getType() const;
	AdObject(BaseGame *inGame);
	~AdObject() override;

	BaseRegion *_currentBlockRegion;
	AdWaypointGroup *_currentWptGroup;
	AdInventory *getInventory();

	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;
	bool afterMove() override;

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	bool updateSpriteAttachments();
	bool displaySpriteAttachments(bool preDisplay);

protected:
	PartEmitter *_partEmitter;
	bool _ignoreItems;
	bool _forcedTalkAnimUsed;
	char *_forcedTalkAnimName;
	BaseSprite *_animSprite;
	BaseSprite *_currentSprite;
	AdSentence *_sentence;
	TObjectState _state;
	TObjectState _nextState;
	TObjectType _type;
	BaseFont *_font;
	BaseSprite *_tempSprite2;
	BaseRegion *_blockRegion;
	AdWaypointGroup *_wptGroup;
	AdObject *_registerAlias;
	bool getScale(float *scaleX, float *scaleY);
private:
	bool _partFollowParent;
	int32 _partOffsetX;
	int32 _partOffsetY;
	bool _subtitlesModRelative;
	bool _subtitlesModXCenter;
	int32 _subtitlesModX;
	int32 _subtitlesModY;
	int32 _subtitlesWidth;
	AdRegion *_currentRegions[MAX_NUM_REGIONS];
	BaseArray<AdObject *> _attachmentsPre;
	BaseArray<AdObject *> _attachmentsPost;
	bool displaySpriteAttachment(AdObject *attachment);
	AdInventory *_inventory;
};

} // End of namespace Wintermute

#endif
