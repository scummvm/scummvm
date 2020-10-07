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

#include "engines/grim/grim.h"
#include "engines/grim/lua_v1.h"
#include "engines/grim/resource.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/primitives.h"
#include "engines/grim/iris.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/set.h"
#include "actor.h"

#include "engines/grim/movie/movie.h"

#include "engines/grim/lua/lua.h"

namespace Grim {

void Lua_V1::GetImage() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj)) {
		lua_pushnil();
		return;
	}
	const char *bitmapName = lua_getstring(nameObj);
	Bitmap *b = Bitmap::create(bitmapName);
	lua_pushusertag(b->getId(), MKTAG('V','B','U','F'));
}

void Lua_V1::FreeImage() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKTAG('V','B','U','F'))
		return;
	Bitmap *bitmap = getbitmap(param);
	delete bitmap;
}

void Lua_V1::BlastImage() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKTAG('V','B','U','F'))
		return;
	Bitmap *bitmap = getbitmap(param);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	if (!lua_isnumber(xObj) || !lua_isnumber(yObj))
		return;

	int x = (int)lua_getnumber(xObj);
	int y = (int)lua_getnumber(yObj);
//	bool transparent = getbool(4); // TODO transparent/masked copy into display
	bitmap->draw(x, y);
}

void Lua_V1::CleanBuffer() {
	g_driver->copyStoredToDisplay();
}

void Lua_V1::StartFullscreenMovie() {
	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	Lua_V1::CleanBuffer();

	GrimEngine::EngineMode prevEngineMode = g_grim->getMode();
	g_grim->setMode(GrimEngine::SmushMode);
	g_grim->setMovieSubtitle(nullptr);
	bool looping = getbool(2);
	bool result = g_movie->play(lua_getstring(name), looping, 0, 0);
	if (!result)
		g_grim->setMode(prevEngineMode);
	pushbool(result);
}

void Lua_V1::StartMovie() {
	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	int x = 0, y = 0;
	if (!lua_isnil(lua_getparam(3)))
		x = (int)lua_getnumber(lua_getparam(3));
	if (!lua_isnil(lua_getparam(4)))
		y = (int)lua_getnumber(lua_getparam(4));

	GrimEngine::EngineMode prevEngineMode = g_grim->getMode();
	g_grim->setMode(GrimEngine::NormalMode);

	bool looping = getbool(2);
	bool result = g_movie->play(lua_getstring(name), looping, x, y);
	g_grim->setMovieSetup();
	if (!result)
		g_grim->setMode(prevEngineMode);
	pushbool(result);
}

/* Fullscreen movie playing query and normal movie
 * query should actually detect correctly and not
 * just return true whenever ANY movie is playing
 */
void Lua_V1::IsFullscreenMoviePlaying() {
	pushbool(g_movie->isPlaying());
}

void Lua_V1::IsMoviePlaying() {
	// Previously, if the game was *not* the demo, this checked also if the mode
	// was GrimEngine::NormalMode. This doesn't seem to be what original does, and causes
	// bug #301 because the movie eldepot.snm is played before legslide.snm ends.
	pushbool(g_movie->isPlaying());
}

void Lua_V1::StopMovie() {
	g_movie->stop();
	// Delete subtitles that may have not expired.
	g_grim->setMovieSubtitle(nullptr);
}

void Lua_V1::PauseMovie() {
	g_movie->pause(lua_isnil(lua_getparam(1)) == 0);
}

void Lua_V1::PurgePrimitiveQueue() {
	PrimitiveObject::getPool().deleteObjects();
}

void Lua_V1::DrawPolygon() {
	lua_Object tableObj1 = lua_getparam(1);
	if (!lua_istable(tableObj1)) {
		lua_pushnil();
		return;
	}

	//int layer = 2;
	Color color;
	lua_Object tableObj2 = lua_getparam(2);
	if (lua_istable(tableObj2)) {
		lua_pushobject(tableObj2);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
			color = getcolor(colorObj);
		}
		lua_pushobject(tableObj2);
		lua_pushstring("layer");
		lua_Object layerObj = lua_gettable();
		if (lua_isnumber(layerObj))
			/*layer = (int)*/lua_getnumber(layerObj);
	}

	// This code only supports 4 point polygons because the game doesn't
	// use other than that. However, the original engine can support 
	// many points per polygon
	lua_Object pointObj;
	Common::Point p[4];
	for (int i = 0; i < 4; i++) {
		// Get X
		lua_pushobject(tableObj1);
		lua_pushnumber(i * 2 + 1);
		pointObj = lua_gettable();
		if (!lua_isnumber(pointObj)) {
			warning("Lua_V1::DrawPolygon: %i Point Parameter X isn't a number!", i * 2 + 1);
			return;
		}
		if (g_grim->getGameType() == GType_GRIM)
			p[i].x = (int)lua_getnumber(pointObj);
		else
			p[i].x = (int)((lua_getnumber(pointObj) + 1) * 320);

		// Get Y
		lua_pushobject(tableObj1);
		lua_pushnumber(i * 2 + 2);
		pointObj = lua_gettable();
		if (!lua_isnumber(pointObj)) {
			warning("Lua_V1::DrawPolygon: %i Point Parameter Y isn't a number!", i * 2 + 2);
			return;
		}
		if (g_grim->getGameType() == GType_GRIM)
			p[i].y = (int)lua_getnumber(pointObj);
		else
			p[i].y = (int)((1 - lua_getnumber(pointObj)) * 240);
	}

	PrimitiveObject *prim = new PrimitiveObject();
	prim->createPolygon(p[0], p[1], p[2], p[3], color);
	lua_pushusertag(prim->getId(), MKTAG('P','R','I','M'));
}

void Lua_V1::DrawLine() {
	Common::Point p1, p2;
	Color color;
	lua_Object x1Obj = lua_getparam(1);
	lua_Object y1Obj = lua_getparam(2);
	lua_Object x2Obj = lua_getparam(3);
	lua_Object y2Obj = lua_getparam(4);
	lua_Object tableObj = lua_getparam(5);

	if (!lua_isnumber(x1Obj) || !lua_isnumber(y1Obj) || !lua_isnumber(x2Obj) || !lua_isnumber(y2Obj)) {
		lua_pushnil();
		return;
	}

	if (g_grim->getGameType() == GType_GRIM) {
		p1.x = (int)lua_getnumber(x1Obj);
		p1.y = (int)lua_getnumber(y1Obj);
		p2.x = (int)lua_getnumber(x2Obj);
		p2.y = (int)lua_getnumber(y2Obj);
	} else {
		p1.x = (int)((lua_getnumber(x1Obj) + 1) * 320);
		p1.y = (int)((1 - lua_getnumber(y1Obj)) * 240);
		p2.x = (int)((lua_getnumber(x2Obj) + 1) * 320);
		p2.y = (int)((1 - lua_getnumber(y2Obj)) * 240);
	}

	//int layer = 2;
	if (lua_istable(tableObj)) {
		lua_pushobject(tableObj);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
			color = getcolor(colorObj);
		}
		lua_pushobject(tableObj);
		lua_pushstring("layer");
		lua_Object layerObj = lua_gettable();
		if (lua_isnumber(layerObj))
			/*layer = (int)*/lua_getnumber(layerObj);
	}

	PrimitiveObject *p = new PrimitiveObject();
	p->createLine(p1, p2, color); // TODO Add layer support
	lua_pushusertag(p->getId(), MKTAG('P','R','I','M'));
}

void Lua_V1::ChangePrimitive() {
	lua_Object param1 = lua_getparam(1);
	if (!lua_isuserdata(param1) || lua_tag(param1) != MKTAG('P','R','I','M'))
		return;

	lua_Object tableObj = lua_getparam(2);
	if (!lua_istable(tableObj))
		return;

	PrimitiveObject *pmodify = getprimitive(param1);
	assert(pmodify);

	Color color;
	lua_pushobject(tableObj);
	lua_pushstring("color");
	lua_Object colorObj = lua_gettable();
	if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
		color = getcolor(colorObj);
		pmodify->setColor(color);
	}

	lua_pushobject(tableObj);
	lua_pushstring("layer");
	lua_Object layer = lua_gettable();
	if (lua_isnumber(layer)) {
		// TODO pmodify->setLayer(lua_getnumber(layer));
		warning("Not implemented: PrimitiveObject::setLayer. Layer: %d", (int)lua_getnumber(layer));
	}

	lua_pushobject(tableObj);
	lua_pushstring("xoffset");
	lua_Object xObj = lua_gettable();
	lua_pushobject(tableObj);
	lua_pushstring("yoffset");
	lua_Object yObj = lua_gettable();
	if (lua_isnumber(xObj) || lua_isnumber(yObj)) {
		//int x = 0;
		//int y = 0;
		if (lua_isnumber(xObj))
			/*x = (int)*/lua_getnumber(xObj);
		if (lua_isnumber(yObj))
			/*y = (int)*/lua_getnumber(yObj);
		// TODO pmodify->setOffets(x, y);
		assert(0);
	}

	lua_pushobject(tableObj);
	lua_pushstring("x");
	xObj = lua_gettable();
	lua_pushobject(tableObj);
	lua_pushstring("y");
	yObj = lua_gettable();
	if (lua_isnumber(xObj) || lua_isnumber(yObj)) {
		int x = -1;
		int y = -1;
		if (lua_isnumber(xObj)) {
			if (g_grim->getGameType() == GType_GRIM)
				x = (int)lua_getnumber(xObj);
			else
				x = (int)((lua_getnumber(xObj) + 1) * 320);
		}
		if (lua_isnumber(yObj)) {
			if (g_grim->getGameType() == GType_GRIM)
				y = (int)lua_getnumber(yObj);
			else
				y = (int)((1 - lua_getnumber(yObj)) * 240);
		}
		pmodify->setPos(x, y);
	}

	lua_pushobject(tableObj);
	lua_pushstring("x2");
	xObj = lua_gettable();
	lua_pushobject(tableObj);
	lua_pushstring("y2");
	yObj = lua_gettable();
	if (lua_isnumber(xObj) || lua_isnumber(yObj)) {
		int x = -1;
		int y = -1;
		if (lua_isnumber(xObj)) {
			if (g_grim->getGameType() == GType_GRIM)
				x = (int)lua_getnumber(xObj);
			else
				x = (int)((lua_getnumber(xObj) + 1) * 320);
		}
		if (lua_isnumber(yObj)) {
			if (g_grim->getGameType() == GType_GRIM)
				y = (int)lua_getnumber(yObj);
			else
				y = (int)((1 - lua_getnumber(yObj)) * 240);
		}
		pmodify->setEndpoint(x, y);
	}

	lua_pushobject(tableObj);
	lua_pushstring("width");
	lua_Object width = lua_gettable();
	lua_pushobject(tableObj);
	lua_pushstring("height");
	lua_Object height = lua_gettable();
	if (lua_isnumber(width) || lua_isnumber(height)) {
		//int x = -1;
		//int y = -1;
		if (lua_isnumber(width))
			/*x = (int)*/lua_getnumber(width);
		if (lua_isnumber(height))
			/*y = (int)*/lua_getnumber(height);
		// TODO pmodify->setSize(x, y);
	}
}

void Lua_V1::DrawRectangle() {
	Common::Point p1, p2;
	Color color;
	lua_Object x1Obj = lua_getparam(1);
	lua_Object y1Obj = lua_getparam(2);
	lua_Object x2Obj = lua_getparam(3);
	lua_Object y2Obj = lua_getparam(4);
	lua_Object tableObj = lua_getparam(5);

	if (!lua_isnumber(x1Obj) || !lua_isnumber(y1Obj) || !lua_isnumber(x2Obj) || !lua_isnumber(y2Obj)) {
		lua_pushnil();
		return;
	}

	if (g_grim->getGameType() == GType_GRIM) {
		p1.x = (int)lua_getnumber(x1Obj);
		p1.y = (int)lua_getnumber(y1Obj);
		p2.x = (int)lua_getnumber(x2Obj);
		p2.y = (int)lua_getnumber(y2Obj);
	} else {
		p1.x = (int)((lua_getnumber(x1Obj) + 1) * 320);
		p1.y = (int)((1 - lua_getnumber(y1Obj)) * 240);
		p2.x = (int)((lua_getnumber(x2Obj) + 1) * 320);
		p2.y = (int)((1 - lua_getnumber(y2Obj)) * 240);
	}
	bool filled = false;

	if (lua_istable(tableObj)) {
		lua_pushobject(tableObj);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
			color = getcolor(colorObj);
		}

		lua_pushobject(tableObj);
		lua_pushstring("filled");
		lua_Object objFilled = lua_gettable();
		if (!lua_isnil(objFilled))
			filled = true;
	}

	PrimitiveObject *p = new PrimitiveObject();
	p->createRectangle(p1, p2, color, filled);
	lua_pushusertag(p->getId(), MKTAG('P','R','I','M')); // FIXME: we use PRIM usetag here
}

void Lua_V1::BlastRect() {
	Common::Point p1, p2;
	Color color;
	lua_Object x1Obj = lua_getparam(1);
	lua_Object y1Obj = lua_getparam(2);
	lua_Object x2Obj = lua_getparam(3);
	lua_Object y2Obj = lua_getparam(4);
	lua_Object tableObj = lua_getparam(5);

	if (!lua_isnumber(x1Obj) || !lua_isnumber(y1Obj) || !lua_isnumber(x2Obj) || !lua_isnumber(y2Obj)) {
		lua_pushnil();
		return;
	}
	if (g_grim->getGameType() == GType_GRIM) {
		p1.x = (int)lua_getnumber(x1Obj);
		p1.y = (int)lua_getnumber(y1Obj);
		p2.x = (int)lua_getnumber(x2Obj);
		p2.y = (int)lua_getnumber(y2Obj);
	} else {
		p1.x = (int)((lua_getnumber(x1Obj) + 1) * 320);
		p1.y = (int)((1 - lua_getnumber(y1Obj)) * 240);
		p2.x = (int)((lua_getnumber(x2Obj) + 1) * 320);
		p2.y = (int)((1 - lua_getnumber(y2Obj)) * 240);
	}
	bool filled = false;

	if (lua_istable(tableObj)) {
		lua_pushobject(tableObj);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
			color = getcolor(colorObj);
		}

		lua_pushobject(tableObj);
		lua_pushstring("filled");
		lua_Object objFilled = lua_gettable();
		if (!lua_isnil(objFilled))
			filled = true;
	}

	PrimitiveObject *p = new PrimitiveObject();
	p->createRectangle(p1, p2, color, filled);
	p->draw();
	delete p;
}

void Lua_V1::KillPrimitive() {
	lua_Object primObj = lua_getparam(1);

	if (!lua_isuserdata(primObj) || lua_tag(primObj) != MKTAG('P','R','I','M'))
		return;

	PrimitiveObject *prim = getprimitive(primObj);
	delete prim;
}

void Lua_V1::DimScreen() {
	g_driver->storeDisplay();
	g_driver->dimScreen();
}

void Lua_V1::DimRegion() {
	int x = (int)lua_getnumber(lua_getparam(1));
	int y = (int)lua_getnumber(lua_getparam(2));
	int w = (int)lua_getnumber(lua_getparam(3));
	int h = (int)lua_getnumber(lua_getparam(4));
	float level = lua_getnumber(lua_getparam(5));
	g_driver->dimRegion(x, y, w, h, level);
}

void Lua_V1::ScreenShot() {
	int width = (int)lua_getnumber(lua_getparam(1));
	int height = (int)lua_getnumber(lua_getparam(2));
	GrimEngine::EngineMode mode = g_grim->getMode();
	g_grim->setMode(GrimEngine::NormalMode);
	g_grim->updateDisplayScene();
	Bitmap *screenshot = g_driver->getScreenshot(width, height, false);
	g_grim->setMode(mode);
	if (screenshot) {
		lua_pushusertag(screenshot->getId(), MKTAG('V','B','U','F'));
	} else {
		lua_pushnil();
	}
}

void Lua_V1::SetGamma() {
	lua_Object levelObj = lua_getparam(1);

	if (!lua_isnumber(levelObj))
		return;
	double level = lua_getnumber(levelObj);

	// FIXME: func(level)
	warning("Lua_V1::SetGamma, implement opcode, level: %f", level);
}

void Lua_V1::Display() {
	if (g_grim->getFlipEnable()) {
		g_driver->flipBuffer();
	}
}

void Lua_V1::EngineDisplay() {
	// it enable/disable updating display
	g_grim->setFlipEnable((bool)lua_getnumber(lua_getparam(1)));
}

void Lua_V1::ForceRefresh() {
	// Nothing to do, no off-screen buffers
}

void Lua_V1::RenderModeUser() {
	lua_Object param1 = lua_getparam(1);
	if (!lua_isnil(param1) && g_grim->getMode() != GrimEngine::DrawMode) {
		g_grim->setPreviousMode(g_grim->getMode());
		g_movie->pause(true);
		g_grim->setMode(GrimEngine::DrawMode);
	} else if (lua_isnil(param1) && g_grim->getMode() == GrimEngine::DrawMode) {
		g_movie->pause(false);
		g_grim->setMode(g_grim->getPreviousMode());
	}
}

void Lua_V1::IrisUp() {
	lua_Object xObj = lua_getparam(1);
	lua_Object yObj = lua_getparam(2);
	lua_Object timeObj = lua_getparam(3);

	g_grim->playIrisAnimation(Iris::Open, (int)lua_getnumber(xObj), (int)lua_getnumber(yObj), (int)lua_getnumber(timeObj));
}

void Lua_V1::IrisDown() {
	lua_Object xObj = lua_getparam(1);
	lua_Object yObj = lua_getparam(2);
	lua_Object timeObj = lua_getparam(3);

	g_grim->playIrisAnimation(Iris::Close, (int)lua_getnumber(xObj), (int)lua_getnumber(yObj), (int)lua_getnumber(timeObj));
}

void Lua_V1::PreRender() {
	g_driver->renderBitmaps(getbool(1));
	g_driver->renderZBitmaps(getbool(2));
}

} // end of namespace Grim
