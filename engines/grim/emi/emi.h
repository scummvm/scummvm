/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef EMI_ENGINE_H
#define EMI_ENGINE_H

#include "engines/grim/grim.h"
#include "engines/grim/emi/layer.h"

namespace Grim {

class TextObject;
class Actor;

class EMIEngine : public GrimEngine {
public:
	EMIEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language);
	virtual ~EMIEngine();

	virtual const char *getUpdateFilename() override;

	void pushText();
	void popText();
	void purgeText();

	void invalidateActiveActorsList() override;
	void invalidateTextObjectsSortOrder() override;
	void invalidateSortOrder();
	void sortActiveActorsList();
	void temporaryStoreSaveGameImage();
	void storeSaveGameImage(SaveGame *state) override;

private:
	LuaBase *createLua() override;
	void drawNormalMode() override;
	void updateDrawMode() override;
	static bool compareTextLayer(const TextObject *x, const TextObject *y);
	void drawTextObjects() override;
	static bool compareActor(const Actor *x, const Actor *y);
	void sortTextObjects();
	static bool compareLayer(const Layer *x, const Layer *y);
	void sortLayers();

	Common::List<TextObject *> _textObjects;
	Common::List<Layer *> _layers;

	bool _textObjectsSortOrderInvalidated;
	bool _sortOrderInvalidated;
};

extern EMIEngine *g_emi;

} // end of namespace Grim

#endif
