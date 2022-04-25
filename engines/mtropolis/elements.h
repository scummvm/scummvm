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

#ifndef MTROPOLIS_ELEMENTS_H
#define MTROPOLIS_ELEMENTS_H

#include "mtropolis/data.h"
#include "mtropolis/runtime.h"

namespace MTropolis {

struct ElementLoaderContext;

class GraphicElement : public VisualElement {
public:
	GraphicElement();
	~GraphicElement();

	bool load(ElementLoaderContext &context, const Data::GraphicElement &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Graphic Element"; }
#endif

private:
	bool _directToScreen;
	bool _cacheBitmap;
};

class MovieElement : public VisualElement {
public:
	MovieElement();
	~MovieElement();

	bool load(ElementLoaderContext &context, const Data::MovieElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib);
	bool writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Movie Element"; }
#endif

private:
	bool scriptSetDirect(const DynamicValue &dest);
	bool scriptSetPaused(const DynamicValue &dest);

	bool _directToScreen;
	bool _cacheBitmap;
	bool _paused;
	bool _loop;
	bool _alternate;
	bool _playEveryFrame;
	uint32 _assetID;
};

} // End of namespace MTropolis

#endif
