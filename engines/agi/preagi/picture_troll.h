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

#ifndef AGI_PREAGI_PICTURE_TROLL_H
#define AGI_PREAGI_PICTURE_TROLL_H

namespace Agi {

class PictureMgr_Troll : public PictureMgr {
public:
	PictureMgr_Troll(AgiBase *agi, GfxMgr *gfx);

	void drawPicture() override;
	
	void draw_Fill() override;
	bool draw_FillCheck(int16 x, int16 y, bool horizontalCheck) override;

	void setStopOnF3(bool stopOnF3) { _stopOnF3 = stopOnF3; }
	void setTrollMode(bool trollMode) { _trollMode = trollMode; }

private:
	bool _stopOnF3;
	bool _trollMode;
};

} // End of namespace Agi

#endif
