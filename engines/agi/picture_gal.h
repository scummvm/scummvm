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

#ifndef AGI_PICTURE_GAL_H
#define AGI_PICTURE_GAL_H

namespace Agi {

class PictureMgr_GAL : public PictureMgr {
public:
	PictureMgr_GAL(AgiBase *agi, GfxMgr *gfx);

protected:
	byte getInitialPriorityColor() const override { return 1; }

	void drawPicture() override;

	void draw_Line(int16 x1, int16 y1, int16 x2, int16 y2) override;

	bool getNextXCoordinate(byte &x) override;
	bool getNextYCoordinate(byte &y) override;

private:
	void draw_SetScreens(byte scrOn, byte priOn);
	void draw_SetScreens(byte scrOn, byte priOn, byte &prevScrOn, byte &prevPriOn);

	void drawBlackFrame();
};

} // End of namespace Agi

#endif
