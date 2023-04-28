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

#ifndef TETRAEDGE_TE_TE_SPRITE_LAYOUT_H
#define TETRAEDGE_TE_TE_SPRITE_LAYOUT_H

#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_tiled_surface.h"

namespace Tetraedge {

class TeSpriteLayout : public TeLayout {
public:
	TeSpriteLayout();

	int bufferSize();
	void cont();
	void draw() override;

	virtual bool onParentWorldColorChanged() override;

	bool load(const Common::String &path);
	bool load(const Common::FSNode &node, const Common::String *forcePath = nullptr);
	bool load(TeImage &img);
	bool load(TeIntrusivePtr<Te3DTexture> &texture);

	void unload();
	void pause();
	void play();

	void setBufferSize(int bufsize);
	void setColor(const TeColor &col) override;

	void setColorKey(const TeColor &col);
	void setColorKeyActivated(bool activated);
	void setColorKeyTolerence(float val);

	bool setName(const Common::String &newName) override;
	void setSize(const TeVector3f32 &newSize) override;

	void stop();

	void updateMesh() override;
	void updateSize() override;

	TeIntrusivePtr<TeTiledSurface> _tiledSurfacePtr;

private:
	bool _sizeSet;
	//bool _allowFloatTranslate;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_SPRITE_LAYOUT_H
