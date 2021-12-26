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

#ifndef DC_DCUTILS_H
#define DC_DCUTILS_H

extern int getCdState();

class TextureStack {
private:
	void *mark;
public:
	TextureStack() {
		ta_sync();
		mark = ta_txmark();
	}
	~TextureStack() {
		ta_sync();
		ta_txrelease(mark);
	}
};

class DiscLabel {
private:
	char buf[32];
public:
	DiscLabel();
	bool operator==(const DiscLabel &other) const;
	void get(char *p) const;
};

class DiscSwap : TextureStack {
private:
	unsigned int argb;
	float x;
	Label lab;
protected:
	virtual void background() {}
	virtual void interact() {}
public:
	DiscSwap(const char *label, unsigned int argb);
	virtual ~DiscSwap() {}
	void run();
};

extern void draw_solid_quad(float x1, float y1, float x2, float y2,
			    int c0, int c1, int c2, int c3);
extern void draw_trans_quad(float x1, float y1, float x2, float y2,
			    int c0, int c1, int c2, int c3);

namespace DC_Flash {
  int flash_read_sector(int partition, int sec, unsigned char *dst);
  int get_locale_setting();
} // End of namespace DC_Flash


#endif /* DC_DCUTILS_H */
