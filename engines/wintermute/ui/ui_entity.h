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

#ifndef WINTERMUTE_UIENTITY_H
#define WINTERMUTE_UIENTITY_H

#include "engines/wintermute/ui/ui_object.h"

namespace Wintermute {
class AdEntity;
class UIEntity : public UIObject {
public:
	DECLARE_PERSISTENT(UIEntity, UIObject)
	UIEntity(BaseGame *inGame);
	~UIEntity() override;
	bool loadFile(const char *filename);
	bool loadBuffer(char *buffer, bool complete);
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;

	bool display() override { return display(0, 0); }
	bool display(int offsetX, int offsetY) override;
	bool setEntity(const char *filename);

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;

private:
	AdEntity *_entity;
};

} // End of namespace Wintermute

#endif
