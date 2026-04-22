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

#ifndef WAYNESWORLD_WWINTRO_H
#define WAYNESWORLD_WWINTRO_H

#include "common/scummsys.h"

#define MAX_SOUNDS 20

namespace WaynesWorld {

class WaynesWorldEngine;
class GxlArchive;
class WWSurface;

struct Frame {
	const char *filename;
	int x;
	int y;
	int delay;
};

class WWIntro {
protected:
	WaynesWorldEngine *_vm;

	WWSurface *_outlineSurface = nullptr;
	WWSurface *_logoSurface = nullptr;
	WWSurface *_backg2Surface = nullptr;
	WWSurface *_introBackg1Image = nullptr;
	WWSurface *_introWhead1[8] = {nullptr};
	WWSurface *_introWbodyImage[5] = {nullptr};
	WWSurface *_introGhead1[11] = {nullptr};
	WWSurface *_introGbodyImage = nullptr;
	
	int _oldRefreshBackgFl = -1;
	int _oldWBodyIndex = -1;
	int _oldGBodyIndex = -1;
	int _oldWHead1Index = -1;
	int _oldGHead1Index = -1;

public:
	WWIntro(WaynesWorldEngine *vm);
	virtual ~WWIntro() {}

	virtual void runIntro() = 0;
	WWSurface *_demoPt2Surface = nullptr;

protected:
	GxlArchive *_oanGxl = nullptr;
	int _startOawPos = 0;
	int _startOagPos = 0;
	int _startOaoPos = 0;


	bool initOanGxl();
	void cleanOanGxl();
	void wwEffect(int arg0, int arg1, bool flag = false);
	void setColor236(int index);
	void sub2FEFB(int arg_refreshBackgFl, int arg_wBodyIndex, int arg_gBodyIndex, int arg_wHead1Index, int arg_gHead1Index, int arg_TextId);
	void introPt3_init();
	void introPt3_clean();
	bool introPt4();
	virtual void introPt4_init() = 0;
	virtual bool introPt4_intro() = 0;
	bool introPt4_displayCallInTime();
	virtual bool introPt4_caller1() = 0;
	virtual bool introPt4_caller2() = 0;
	virtual bool introPt4_caller3() = 0;
	virtual bool introPt4_caller4() = 0;
	void introPt4_cleanup();
	bool introPt4_playGuitar();


private:
	void sub3009A(int textId);
};

class WWIntro_full : public WWIntro {
public:
	WWIntro_full(WaynesWorldEngine *vm);
	~WWIntro_full() override;

	void runIntro() override;

private:
	bool introPt1();
	bool introPt3(bool flag);
	void introDisplaySign();
	void introPt7();

protected:
	bool introPt4_intro() override;
	void introPt4_init() override;
	bool introPt4_caller1() override;
	bool introPt4_caller2() override;
	bool introPt4_caller3() override;
	bool introPt4_caller4() override;
};

class WWIntro_demo1 : public WWIntro {
public:
	WWIntro_demo1(WaynesWorldEngine *vm);
	~WWIntro_demo1() override;

	void runIntro() override;

private:
	bool introPt1();
	bool introPt1_selectware();
	bool introPt3();
	bool introPt3Bis();
	bool introDisplaySign();
	bool introPreviewRoom00();
	bool introMapStonebridge();
	bool introPreviewRoom08and22();
	bool introMapButterfield();
	bool introPreviewRoom07and15and16();
	bool introMapDowntown();
	bool introPreviewRoom10();
	bool introMapNorthAurora();
	bool introPreviewRoom03and23();
	bool introMapSouthEastArea();
	bool introPreviewRoom13and18();
	bool introMapWestAurora();
	bool introPreviewRoom01();

protected:
	bool introPt4_intro() override;
	void introPt4_init() override;
	bool introPt4_caller1() override;
	bool introPt4_caller2() override;
	bool introPt4_caller3() override;
	bool introPt4_caller4() override;
};
} // End of namespace WaynesWorld

#endif /* WAYNESWORLD_WWINTRO_H*/
