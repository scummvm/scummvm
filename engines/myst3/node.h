/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef MYST3_ROOM_H
#define MYST3_ROOM_H

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "engines/myst3/archive.h"

#include "graphics/surface.h"
#include "graphics/jpeg.h"
#include "graphics/conversion.h"

namespace Myst3 {

class Myst3Engine;

class Face {
	public:
		Graphics::Surface *_bitmap;
		GLuint _textureId;

		Face();
		~Face();

		void setTextureFromJPEG(Graphics::JPEG *jpeg);
		void uploadTexture();
};

class SpotItemFace {
	public:
		SpotItemFace(Face *face, uint16 posX, uint16 posY);
		~SpotItemFace();

		void loadData(Graphics::JPEG *jpeg);

		void draw();
		void undraw();
		void fadeDraw();

		bool isDrawn() { return _drawn; }
		void setDrawn(bool drawn) { _drawn = drawn; }
		uint16 getFadeValue() { return _fadeValue; }
		void setFadeValue(uint16 value) { _fadeValue = value; }

	private:
		Face *_face;
		bool _drawn;
		uint16 _fadeValue;
		uint16 _posX;
		uint16 _posY;

		Graphics::Surface *_bitmap;
		Graphics::Surface *_notDrawnBitmap;
};

class SpotItem {
	public:
		SpotItem(Myst3Engine *vm);
		~SpotItem();

		void setCondition(uint16 condition) { _condition = condition; }
		void setFade(bool fade) { _enableFade = fade; }
		void setFadeVar(uint16 var) { _fadeVar = var; }
		void addFace(SpotItemFace *face) { _faces.push_back(face); }

		void update();
	private:
		Myst3Engine *_vm;

		uint16 _condition;
		uint16 _fadeVar;
		bool _enableFade;

		Common::Array<SpotItemFace *> _faces;
};

class Node {
	protected:
		Myst3Engine *_vm;
		Face *_faces[6];
		Common::Array<SpotItem *> _spotItems;

	public:
		Node(Myst3Engine *vm, Archive *archive, uint16 id);
		virtual ~Node();

		void update();
		virtual void draw() = 0;
		void loadSpotItem(Archive &archive, uint16 id, uint16 condition, bool fade);
		void dumpFaceMask(Archive &archive, uint16 index, int face);

		static const int _cubeTextureSize = 1024;
};

} // end of namespace Myst3

#endif
