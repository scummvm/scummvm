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

#ifndef QDENGINE_MINIGAMES_3MICE2_TESTO_H
#define QDENGINE_MINIGAMES_3MICE2_TESTO_H

#include "common/debug.h"
#include "common/rect.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qd3mice2TestoMiniGame : public qdMiniGameInterface {
public:
	qd3mice2TestoMiniGame() {}
	~qd3mice2TestoMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mice2Testo::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		_figures[0] = _scene->object_interface("figure1");
		_figures[1] = _scene->object_interface("figure2");
		_figures[2] = _scene->object_interface("figure3");
		_figures[3] = _scene->object_interface("figure4");
		_figures[4] = _scene->object_interface("figure5");
		_figures[5] = _scene->object_interface("figure6");
		_figures[6] = _scene->object_interface("figure7");
		_figures[7] = _scene->object_interface("figure8");
		_figures[8]  = _scene->object_interface("figure1fake");
		_figures[9]  = _scene->object_interface("figure2fake");
		_figures[10] = _scene->object_interface("figure3fake");
		_figures[11] = _scene->object_interface("figure4fake");
		_figures[12] = _scene->object_interface("figure5fake");
		_figures[13] = _scene->object_interface("figure6fake");
		_figures[14] = _scene->object_interface("figure7fake");
		_figures[15] = _scene->object_interface("figure8fake");
		_figures[16] = _scene->object_interface("figure1inv");
		_figures[17] = _scene->object_interface("figure2inv");
		_figures[18] = _scene->object_interface("figure3inv");
		_figures[19] = _scene->object_interface("figure4inv");
		_figures[20] = _scene->object_interface("figure5inv");
		_figures[21] = _scene->object_interface("figure6inv");
		_figures[22] = _scene->object_interface("figure7inv");
		_figures[23] = _scene->object_interface("figure8inv");

		_objNoDough = _scene->object_interface("notesto");
		_objNoDoughFake = _scene->object_interface("notesto_fake");
		_objDone = _scene->object_interface("$done");
		_objLoadPassed = _scene->object_interface("$\xE7\xE0\xE3\xF0\xF3\xE7\xEA\xE0 \xE1\xFB\xEB\xE0"); // "$загрузка была"

#if 0
		for (int i = 0; i < 1088; i++) {
			debugN("{ %3d, %3d }, ", _figureVerts[i].x, _figureVerts[i].y);

			if ((i + 1) % 8 == 0)
				debugN("// %d\n", i - 7);
		}
#endif

		_noDoughX = 1000;

		_numVerts[0] = 66;
		_numVerts[1] = 94;
		_numVerts[2] = 136;
		_numVerts[3] = 136;
		_numVerts[4] = 68;
		_numVerts[5] = 68;
		_numVerts[6] = 112;
		_numVerts[7] = 112;

		for (int i = 0; i < 8; i++) {
			int v = i;
			int maxX = 0;
			int minX = 0;
			int minY = 0;
			int maxY = 0;

			for (int j = 0; j < _numVerts[i]; j++) {
				if (_figureVerts[v].x < minX)
					minX = _figureVerts[v].x;
				if (_figureVerts[v].x > maxX)
					maxX = _figureVerts[v].x;
				if (_figureVerts[v].y < minY)
					minY = _figureVerts[v].y;
				if (_figureVerts[v].y > maxY)
					maxY = _figureVerts[v].y;

				v += 8;	// Data is arranged in vertical columns
			}

			_figureBboxes[i].top = maxX;
			_figureBboxes[i].left = minX;
			_figureBboxes[i].bottom = maxY;
			_figureBboxes[i].top = minY;

			debugC(2, kDebugMinigames, "bbox for: %d: [%d, %d, %d, %d]", i, _figureBboxes[i].left, _figureBboxes[i].top, _figureBboxes[i].right, _figureBboxes[i].bottom);
		}

		mgVect2i pos = _objNoDough->screen_R();

		pos.x += _noDoughX;

		_objNoDoughFake->set_R(_scene->screen2world_coords(pos, -100.0));

		_initialCoords[0].x = 75;
		_initialCoords[0].y = 80;
		_initialCoords[1].x = 695;
		_initialCoords[1].y = 100;
		_initialCoords[2].x = 82;
		_initialCoords[2].y = 234;
		_initialCoords[3].x = 738;
		_initialCoords[3].y = 468;
		_initialCoords[4].x = 84;
		_initialCoords[4].y = 392;
		_initialCoords[5].x = 302;
		_initialCoords[5].y = 465;
		_initialCoords[6].x = 687;
		_initialCoords[6].y = 293;
		_initialCoords[7].x = 611;
		_initialCoords[7].y = 472;

		if (_objLoadPassed->is_state_active("\xed\xe5\xf2")) { // "нет"
			for (int i = 0; i < 8; i++)
				_figures[i]->set_R(_scene->screen2world_coords(_initialCoords[i], 0.0));

			_objLoadPassed->set_state("\xe4\xe0");	// "да"

			return true;
		}

		return false;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "3mice2Testo::quant(%f)", dt);

		if (checkSolution())
			_objDone->set_state("\xe4\xe0");	// "да"
		else
			_objDone->set_state("\xed\xe5\xf2"); // "нет"

		for (int i = 0; i < 8; i++) {
			mgVect2i pos = _figures[i]->screen_R();

			pos.x += _noDoughX;

			_figures[i + 8]->set_R(_scene->screen2world_coords(pos, 1000.0));
		}

		qdMinigameObjectInterface *mouseObj = _scene->mouse_object_interface();
		qdMinigameObjectInterface *clickObj = _scene->mouse_click_object_interface();

		const char *name = nullptr;

		if (mouseObj) {
			name = mouseObj->name();
			if (strstr(name, "figure") && strstr(name, "inv")) {
				int num = atol(name + 6);

				if (checkSnapPiece(num - 1))
					mouseObj->set_state("inv");
				else
					mouseObj->set_state("inv_glow");
			}
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_DOWN) && !mouseObj) {
			int hit = hitTest();

			debugC(2, kDebugMinigames, "hit: %d", hit);

			if (hit > -1) {
				_figures[hit]->set_state("hide");
				_figures[hit + 16]->set_state("to_inv");
			}
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_DOWN) && mouseObj) {
			int num = -1;

			if (strstr(name, "figure") && strstr(name, "inv")) {
				num = atol(name + 6);

				debugC(2, kDebugMinigames, "drop: %d", num);

				if (num > 0 && num <= 8 && !checkSnapPiece(num - 1)) {
					_figures[num - 1]->set_state("testo");
					_figures[num + 15]->set_state("del");
					_figures[num - 1]->set_R(_scene->screen2world_coords(_engine->mouse_cursor_position(), 0.0));
				}
			}

			if (clickObj) { // && mouseObj
				name = clickObj->name();

				if (strstr(name, "notesto") && num > 0 && num <= 8) {
					_figures[num - 1]->set_state("base");
					_figures[num + 15]->set_state("del");

					_figures[num - 1]->set_R(_scene->screen2world_coords(_initialCoords[num - 1], 0.0));
				}
			}
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_RIGHT_DOWN) && mouseObj) {
			name = mouseObj->name();
			if (strstr(name, "figure") && strstr(name, "inv")) {
				int num = atol(name + 6);

				if (num > 0 && num <= 8) {
					_figures[num - 1]->set_state("base");
					_figures[num + 15]->set_state("del");

					_figures[num - 1]->set_R(_scene->screen2world_coords(_initialCoords[num - 1], 0.0));
				}
			}
		}

  		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "3mice2Testo::finit()");

		if (_scene)  {
			_engine->release_scene_interface(_scene);
			_scene = 0;
		}

 		return true;
	}

	bool new_game(const qdEngineInterface *engine_interface) {
		return true;
	}

	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		return 0;
	}

	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		return 0;
	}

	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}

private:
	bool checkSolution() {
		return _figures[1]->is_state_active("testo")
			&& _figures[3]->is_state_active("testo")
			&& _figures[4]->is_state_active("testo")
			&& _figures[5]->is_state_active("testo")
			&& _figures[6]->is_state_active("testo");
	}

	int hitTest() {
		mgVect2i pos = _engine->mouse_cursor_position();
		pos.x += _noDoughX;

		for (int i = 0; i < 8; i++) {
			if (_figures[i + 8]->hit_test(pos))
				return i;
		}

		return -1;
	}

	bool checkSnapPiece(int num) {
		mgVect2i pos = _engine->mouse_cursor_position();
		pos.x += _noDoughX;

		int v = num;

		for (int i = 0; i < _numVerts[num]; i++) {
			mgVect2i npos = _figureVerts[v];
			npos.x += pos.x;
			npos.y += pos.y;

			for (int j = 0; j < 8; j++) {
				if (j != num) {
					if (_figures[8 + j]->hit_test(npos))
						return true;

					if (_objNoDoughFake->hit_test(npos))
						return true;
				}
			}

			v += 8;
		}

		for (int i = 0; i < 8; i++) {
			if (i != num) {
				if (_figureBboxes[i].right  + _figures[i]->screen_R().x < pos.x + _figureBboxes[num].right &&
					_figureBboxes[i].left   + _figures[i]->screen_R().x > pos.x + _figureBboxes[num].left &&
					_figureBboxes[i].top    + _figures[i]->screen_R().y < pos.y + _figureBboxes[num].top &&
					_figureBboxes[i].bottom + _figures[i]->screen_R().y > pos.y + _figureBboxes[num].bottom)
						return true;
			}
		}

		return false;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_figures[24] = { nullptr };

	int _numVerts[8] = { 0 };

	qdMinigameObjectInterface *_objNoDough = nullptr;
	qdMinigameObjectInterface *_objNoDoughFake = nullptr;
	qdMinigameObjectInterface *_objLoadPassed = nullptr;
	qdMinigameObjectInterface *_objDone = nullptr;

	mgVect2i _initialCoords[8];
	int _noDoughX = 0;
	Common::Rect _figureBboxes[8];


	const mgVect2i _figureVerts[1088] = {
		{ -66,   0 }, { -86,   0 }, { -73,   0 }, { -54,   0 }, { -65,   0 }, { -22,   0 }, { -83,   0 }, { -62,   0 }, // 0
		{ -66, -10 }, { -86, -11 }, { -68,  -3 }, { -50,  -2 }, { -65, -10 }, { -22,  -3 }, { -83,  -6 }, { -62,  -5 }, // 8
		{ -65, -20 }, { -85, -20 }, { -63,  -6 }, { -47,  -4 }, { -65, -17 }, { -22,  -6 }, { -80,  -9 }, { -60,  -7 }, // 16
		{ -65, -30 }, { -85, -30 }, { -58, -11 }, { -43,  -8 }, { -62, -27 }, { -21,  -9 }, { -77, -12 }, { -58,  -9 }, // 24
		{ -65, -40 }, { -85, -41 }, { -53, -14 }, { -39, -10 }, { -58, -34 }, { -20, -12 }, { -74, -15 }, { -56, -11 }, // 32
		{ -64, -50 }, { -84, -50 }, { -50, -16 }, { -37, -12 }, { -53, -41 }, { -18, -14 }, { -71, -20 }, { -53, -15 }, // 40
		{ -63, -60 }, { -84, -57 }, { -50, -17 }, { -37, -13 }, { -48, -45 }, { -17, -15 }, { -70, -23 }, { -53, -17 }, // 48
		{ -62, -63 }, { -84, -63 }, { -53, -22 }, { -39, -16 }, { -43, -50 }, { -15, -17 }, { -71, -27 }, { -53, -20 }, // 56
		{ -59, -64 }, { -83, -74 }, { -53, -26 }, { -39, -19 }, { -38, -53 }, { -13, -18 }, { -72, -31 }, { -54, -23 }, // 64
		{ -49, -65 }, { -82, -80 }, { -57, -35 }, { -42, -26 }, { -33, -56 }, { -11, -19 }, { -73, -36 }, { -55, -27 }, // 72
		{ -39, -65 }, { -81, -81 }, { -61, -41 }, { -45, -30 }, { -28, -58 }, { -10, -20 }, { -71, -40 }, { -53, -30 }, // 80
		{ -29, -65 }, { -80, -82 }, { -61, -44 }, { -45, -33 }, { -23, -60 }, {  -8, -21 }, { -69, -44 }, { -52, -33 }, // 88
		{ -19, -65 }, { -77, -82 }, { -56, -44 }, { -41, -33 }, { -18, -61 }, {  -6, -21 }, { -68, -46 }, { -51, -35 }, // 96
		{  -9, -65 }, { -74, -83 }, { -51, -44 }, { -38, -33 }, { -13, -63 }, {  -4, -22 }, { -64, -47 }, { -48, -35 }, // 104
		{   1, -65 }, { -57, -84 }, { -45, -42 }, { -33, -31 }, {  -8, -63 }, {  -3, -22 }, { -59, -47 }, { -44, -35 }, // 112
		{  11, -65 }, { -47, -84 }, { -40, -41 }, { -30, -30 }, {  -3, -63 }, {  -1, -22 }, { -57, -48 }, { -43, -36 }, // 120
		{  21, -65 }, { -37, -85 }, { -34, -40 }, { -25, -30 }, {   2, -63 }, {   1, -22 }, { -53, -52 }, { -40, -39 }, // 128
		{  31, -65 }, { -30, -86 }, { -33, -40 }, { -24, -30 }, {   7, -63 }, {   2, -22 }, { -52, -55 }, { -39, -41 }, // 136
		{  39, -65 }, { -20, -86 }, { -33, -45 }, { -24, -33 }, {  12, -63 }, {   4, -22 }, { -52, -61 }, { -39, -46 }, // 144
		{  47, -64 }, { -10, -86 }, { -33, -50 }, { -24, -37 }, {  17, -62 }, {   6, -21 }, { -49, -67 }, { -37, -50 }, // 152
		{  52, -64 }, {   0, -86 }, { -33, -55 }, { -24, -41 }, {  22, -60 }, {   8, -21 }, { -45, -71 }, { -34, -53 }, // 160
		{  57, -63 }, {  10, -86 }, { -33, -60 }, { -24, -44 }, {  27, -58 }, {   9, -20 }, { -40, -74 }, { -30, -56 }, // 168
		{  61, -63 }, {  20, -86 }, { -34, -61 }, { -25, -45 }, {  32, -56 }, {  11, -19 }, { -35, -76 }, { -26, -57 }, // 176
		{  64, -62 }, {  30, -86 }, { -34, -68 }, { -25, -50 }, {  37, -53 }, {  13, -18 }, { -30, -75 }, { -23, -56 }, // 184
		{  65, -61 }, {  41, -86 }, { -30, -68 }, { -22, -50 }, {  42, -50 }, {  14, -17 }, { -25, -72 }, { -19, -54 }, // 192
		{  65, -60 }, {  51, -85 }, { -29, -67 }, { -21, -50 }, {  47, -45 }, {  16, -15 }, { -20, -71 }, { -15, -53 }, // 200
		{  65, -50 }, {  59, -84 }, { -24, -66 }, { -18, -49 }, {  53, -39 }, {  18, -13 }, { -15, -72 }, { -11, -54 }, // 208
		{  65, -40 }, {  74, -83 }, { -20, -64 }, { -15, -47 }, {  57, -34 }, {  20, -12 }, { -10, -77 }, {  -8, -58 }, // 216
		{  65, -30 }, {  79, -82 }, { -15, -61 }, { -11, -45 }, {  60, -29 }, {  21, -10 }, {  -5, -80 }, {  -4, -60 }, // 224
		{  65, -20 }, {  83, -81 }, { -10, -58 }, {  -7, -43 }, {  63, -22 }, {  22,  -8 }, {   0, -80 }, {   0, -60 }, // 232
		{  65, -10 }, {  84, -80 }, {  -6, -56 }, {  -4, -41 }, {  65, -15 }, {  22,  -5 }, {   5, -80 }, {   4, -60 }, // 240
		{  65,   0 }, {  85, -79 }, {   0, -65 }, {   0, -48 }, {  65, -10 }, {  22,  -3 }, {  10, -76 }, {   8, -57 }, // 248
		{  65,   6 }, {  86, -78 }, {   5, -72 }, {   4, -53 }, {  65,  -5 }, {  22,  -2 }, {  15, -71 }, {  11, -53 }, // 256
		{  64,   7 }, {  87, -75 }, {   9, -76 }, {   7, -56 }, {  65,   0 }, {  22,   0 }, {  20, -70 }, {  15, -53 }, // 264
		{  64,  17 }, {  87, -68 }, {  12, -76 }, {   9, -56 }, {  65,   5 }, {  22,   2 }, {  25, -71 }, {  19, -53 }, // 272
		{  64,  20 }, {  87, -58 }, {  15, -69 }, {  11, -51 }, {  65,  10 }, {  22,   3 }, {  30, -72 }, {  23, -54 }, // 280
		{  63,  22 }, {  86, -58 }, {  18, -60 }, {  13, -44 }, {  63,  15 }, {  22,   5 }, {  35, -71 }, {  26, -53 }, // 288
		{  63,  32 }, {  86, -48 }, {  19, -53 }, {  14, -39 }, {  61,  20 }, {  21,   7 }, {  40, -68 }, {  30, -51 }, // 296
		{  63,  42 }, {  86, -38 }, {  20, -52 }, {  15, -38 }, {  59,  25 }, {  20,   9 }, {  43, -64 }, {  32, -48 }, // 304
		{  63,  44 }, {  86, -28 }, {  25, -53 }, {  18, -39 }, {  56,  30 }, {  19,  10 }, {  44, -60 }, {  33, -45 }, // 312
		{  62,  45 }, {  86, -18 }, {  30, -54 }, {  22, -40 }, {  53,  35 }, {  18,  12 }, {  46, -54 }, {  35, -41 }, // 320
		{  62,  55 }, {  86, -10 }, {  35, -55 }, {  26, -41 }, {  49,  40 }, {  17,  14 }, {  50, -51 }, {  38, -38 }, // 328
		{  62,  61 }, {  85, -10 }, {  40, -57 }, {  30, -42 }, {  44,  45 }, {  15,  15 }, {  55, -49 }, {  41, -37 }, // 336
		{  61,  62 }, {  85,   9 }, {  43, -58 }, {  32, -43 }, {  37,  50 }, {  13,  17 }, {  59, -50 }, {  44, -38 }, // 344
		{  61,  63 }, {  84,   9 }, {  45, -57 }, {  33, -42 }, {  33,  53 }, {  11,  18 }, {  65, -48 }, {  49, -36 }, // 352
		{  60,  64 }, {  84,  26 }, {  45, -52 }, {  33, -38 }, {  28,  56 }, {  10,  19 }, {  70, -43 }, {  53, -32 }, // 360
		{  50,  64 }, {  83,  26 }, {  43, -46 }, {  32, -34 }, {  21,  59 }, {   7,  20 }, {  71, -41 }, {  53, -31 }, // 368
		{  40,  65 }, {  83,  42 }, {  41, -37 }, {  30, -27 }, {  16,  61 }, {   6,  21 }, {  71, -36 }, {  53, -27 }, // 376
		{  30,  65 }, {  82,  42 }, {  39, -32 }, {  29, -24 }, {  10,  63 }, {   3,  22 }, {  69, -32 }, {  52, -24 }, // 384
		{  20,  65 }, {  82,  50 }, {  40, -31 }, {  30, -23 }, {   5,  63 }, {   2,  22 }, {  68, -27 }, {  51, -20 }, // 392
		{  10,  65 }, {  83,  50 }, {  45, -30 }, {  33, -22 }, {   0,  63 }, {   0,  22 }, {  71, -22 }, {  53, -17 }, // 400
		{   0,  65 }, {  83,  55 }, {  50, -30 }, {  37, -22 }, {  -5,  63 }, {  -2,  22 }, {  76, -18 }, {  57, -14 }, // 408
		{ -10,  65 }, {  82,  55 }, {  55, -31 }, {  41, -23 }, { -10,  63 }, {  -3,  22 }, {  80, -15 }, {  60, -11 }, // 416
		{ -20,  65 }, {  82,  68 }, {  60, -31 }, {  44, -23 }, { -15,  63 }, {  -5,  22 }, {  84, -10 }, {  63,  -8 }, // 424
		{ -30,  65 }, {  81,  68 }, {  67, -31 }, {  50, -23 }, { -20,  61 }, {  -7,  21 }, {  84,  -5 }, {  63,  -4 }, // 432
		{ -40,  65 }, {  81,  81 }, {  67, -29 }, {  50, -21 }, { -25,  59 }, {  -9,  20 }, {  84,  -2 }, {  63,  -2 }, // 440
		{ -45,  64 }, {  80,  81 }, {  64, -25 }, {  47, -18 }, { -30,  57 }, { -10,  20 }, {  82,   0 }, {  62,   0 }, // 448
		{ -50,  63 }, {  80,  83 }, {  61, -20 }, {  45, -15 }, { -35,  54 }, { -12,  19 }, {  78,   3 }, {  59,   2 }, // 456
		{ -60,  63 }, {  79,  83 }, {  57, -16 }, {  42, -12 }, { -40,  51 }, { -14,  18 }, {  75,   7 }, {  56,   5 }, // 464
		{ -66,  63 }, {  79,  84 }, {  54, -12 }, {  40,  -9 }, { -45,  46 }, { -15,  16 }, {  73,  10 }, {  55,   8 }, // 472
		{ -66,  53 }, {  71,  84 }, {  52,  -9 }, {  38,  -7 }, { -49,  42 }, { -17,  14 }, {  73,  14 }, {  55,  11 }, // 480
		{ -66,  43 }, {  71,  85 }, {  55,  -6 }, {  41,  -4 }, { -53,  37 }, { -18,  13 }, {  77,  20 }, {  58,  15 }, // 488
		{ -66,  33 }, {  61,  85 }, {  60,  -3 }, {  44,  -2 }, { -57,  32 }, { -20,  11 }, {  79,  24 }, {  59,  18 }, // 496
		{ -66,  23 }, {  53,  85 }, {  65,   0 }, {  48,   0 }, { -60,  26 }, { -21,   9 }, {  78,  29 }, {  59,  22 }, // 504
		{ -66,  13 }, {  49,  85 }, {  69,   3 }, {  51,   2 }, { -63,  19 }, { -22,   7 }, {  75,  32 }, {  56,  24 }, // 512
		{ -67,   9 }, {  49,  86 }, {  72,   5 }, {  53,   4 }, { -64,  16 }, { -22,   6 }, {  73,  34 }, {  55,  26 }, // 520
		{   0,   0 }, {  39,  86 }, {  72,   6 }, {  53,   4 }, { -65,  11 }, { -22,   4 }, {  73,  34 }, {  55,  26 }, // 528
		{   0,   0 }, {  29,  86 }, {  67,   9 }, {  50,   7 }, { -65,   5 }, { -22,   2 }, {  68,  34 }, {  51,  26 }, // 536
		{   0,   0 }, {  19,  86 }, {  62,  11 }, {  46,   8 }, {   0,   0 }, {   0,   0 }, {  63,  35 }, {  47,  26 }, // 544
		{   0,   0 }, {   9,  86 }, {  57,  13 }, {  42,  10 }, {   0,   0 }, {   0,   0 }, {  58,  35 }, {  44,  26 }, // 552
		{   0,   0 }, {   0,  86 }, {  52,  16 }, {  38,  12 }, {   0,   0 }, {   0,   0 }, {  56,  38 }, {  42,  29 }, // 560
		{   0,   0 }, {  -3,  86 }, {  50,  18 }, {  37,  13 }, {   0,   0 }, {   0,   0 }, {  55,  41 }, {  41,  31 }, // 568
		{   0,   0 }, {  -3,  85 }, {  52,  23 }, {  38,  17 }, {   0,   0 }, {   0,   0 }, {  55,  46 }, {  41,  35 }, // 576
		{   0,   0 }, { -13,  85 }, {  54,  28 }, {  40,  21 }, {   0,   0 }, {   0,   0 }, {  50,  52 }, {  38,  39 }, // 584
		{   0,   0 }, { -23,  85 }, {  56,  35 }, {  41,  26 }, {   0,   0 }, {   0,   0 }, {  45,  57 }, {  34,  43 }, // 592
		{   0,   0 }, { -33,  85 }, {  57,  38 }, {  42,  28 }, {   0,   0 }, {   0,   0 }, {  39,  60 }, {  29,  45 }, // 600
		{   0,   0 }, { -43,  85 }, {  57,  42 }, {  42,  31 }, {   0,   0 }, {   0,   0 }, {  33,  59 }, {  25,  44 }, // 608
		{   0,   0 }, { -50,  85 }, {  54,  42 }, {  40,  31 }, {   0,   0 }, {   0,   0 }, {  27,  65 }, {  20,  49 }, // 616
		{   0,   0 }, { -57,  85 }, {  49,  42 }, {  36,  31 }, {   0,   0 }, {   0,   0 }, {  26,  69 }, {  20,  52 }, // 624
		{   0,   0 }, { -57,  84 }, {  45,  41 }, {  33,  30 }, {   0,   0 }, {   0,   0 }, {  25,  74 }, {  19,  56 }, // 632
		{   0,   0 }, { -62,  84 }, {  40,  41 }, {  30,  30 }, {   0,   0 }, {   0,   0 }, {  22,  76 }, {  17,  57 }, // 640
		{   0,   0 }, { -62,  83 }, {  35,  41 }, {  26,  30 }, {   0,   0 }, {   0,   0 }, {  17,  77 }, {  13,  58 }, // 648
		{   0,   0 }, { -72,  83 }, {  30,  41 }, {  22,  30 }, {   0,   0 }, {   0,   0 }, {  12,  76 }, {   9,  57 }, // 656
		{   0,   0 }, { -83,  83 }, {  28,  41 }, {  21,  30 }, {   0,   0 }, {   0,   0 }, {   7,  74 }, {   5,  56 }, // 664
		{   0,   0 }, { -84,  82 }, {  27,  41 }, {  20,  30 }, {   0,   0 }, {   0,   0 }, {   3,  70 }, {   2,  53 }, // 672
		{   0,   0 }, { -84,  71 }, {  27,  45 }, {  20,  33 }, {   0,   0 }, {   0,   0 }, {   0,  68 }, {   0,  51 }, // 680
		{   0,   0 }, { -84,  61 }, {  27,  50 }, {  20,  37 }, {   0,   0 }, {   0,   0 }, {  -5,  71 }, {  -4,  53 }, // 688
		{   0,   0 }, { -85,  61 }, {  27,  55 }, {  20,  41 }, {   0,   0 }, {   0,   0 }, { -10,  76 }, {  -8,  57 }, // 696
		{   0,   0 }, { -85,  51 }, {  27,  60 }, {  20,  44 }, {   0,   0 }, {   0,   0 }, { -16,  78 }, { -12,  59 }, // 704
		{   0,   0 }, { -85,  41 }, {  27,  65 }, {  20,  48 }, {   0,   0 }, {   0,   0 }, { -22,  77 }, { -17,  58 }, // 712
		{   0,   0 }, { -85,  31 }, {  25,  66 }, {  18,  49 }, {   0,   0 }, {   0,   0 }, { -28,  73 }, { -21,  55 }, // 720
		{   0,   0 }, { -85,  21 }, {  20,  63 }, {  15,  47 }, {   0,   0 }, {   0,   0 }, { -33,  64 }, { -25,  48 }, // 728
		{   0,   0 }, { -85,  12 }, {  15,  59 }, {  11,  44 }, {   0,   0 }, {   0,   0 }, { -40,  64 }, { -30,  48 }, // 736
		{   0,   0 }, { -86,   1 }, {  10,  57 }, {   7,  42 }, {   0,   0 }, {   0,   0 }, { -45,  66 }, { -34,  50 }, // 744
		{   0,   0 }, {   0,   0 }, {   5,  54 }, {   4,  40 }, {   0,   0 }, {   0,   0 }, { -50,  66 }, { -38,  50 }, // 752
		{   0,   0 }, {   0,   0 }, {   2,  60 }, {   1,  44 }, {   0,   0 }, {   0,   0 }, { -55,  64 }, { -41,  48 }, // 760
		{   0,   0 }, {   0,   0 }, {  -1,  65 }, {  -1,  48 }, {   0,   0 }, {   0,   0 }, { -59,  60 }, { -44,  45 }, // 768
		{   0,   0 }, {   0,   0 }, {  -4,  71 }, {  -3,  53 }, {   0,   0 }, {   0,   0 }, { -60,  56 }, { -45,  42 }, // 776
		{   0,   0 }, {   0,   0 }, {  -6,  73 }, {  -4,  54 }, {   0,   0 }, {   0,   0 }, { -60,  51 }, { -45,  38 }, // 784
		{   0,   0 }, {   0,   0 }, {  -9,  73 }, {  -7,  54 }, {   0,   0 }, {   0,   0 }, { -60,  47 }, { -45,  35 }, // 792
		{   0,   0 }, {   0,   0 }, { -11,  71 }, {  -8,  53 }, {   0,   0 }, {   0,   0 }, { -64,  41 }, { -48,  31 }, // 800
		{   0,   0 }, {   0,   0 }, { -13,  65 }, { -10,  48 }, {   0,   0 }, {   0,   0 }, { -69,  39 }, { -52,  29 }, // 808
		{   0,   0 }, {   0,   0 }, { -15,  60 }, { -11,  44 }, {   0,   0 }, {   0,   0 }, { -74,  38 }, { -56,  29 }, // 816
		{   0,   0 }, {   0,   0 }, { -17,  56 }, { -13,  41 }, {   0,   0 }, {   0,   0 }, { -78,  35 }, { -59,  26 }, // 824
		{   0,   0 }, {   0,   0 }, { -19,  51 }, { -14,  38 }, {   0,   0 }, {   0,   0 }, { -79,  33 }, { -59,  25 }, // 832
		{   0,   0 }, {   0,   0 }, { -22,  53 }, { -16,  39 }, {   0,   0 }, {   0,   0 }, { -80,  30 }, { -60,  23 }, // 840
		{   0,   0 }, {   0,   0 }, { -27,  57 }, { -20,  42 }, {   0,   0 }, {   0,   0 }, { -77,  25 }, { -58,  19 }, // 848
		{   0,   0 }, {   0,   0 }, { -32,  60 }, { -24,  44 }, {   0,   0 }, {   0,   0 }, { -74,  19 }, { -56,  14 }, // 856
		{   0,   0 }, {   0,   0 }, { -38,  63 }, { -28,  47 }, {   0,   0 }, {   0,   0 }, { -73,  14 }, { -55,  11 }, // 864
		{   0,   0 }, {   0,   0 }, { -39,  62 }, { -29,  46 }, {   0,   0 }, {   0,   0 }, { -76,   9 }, { -57,   7 }, // 872
		{   0,   0 }, {   0,   0 }, { -41,  62 }, { -30,  46 }, {   0,   0 }, {   0,   0 }, { -80,   7 }, { -60,   5 }, // 880
		{   0,   0 }, {   0,   0 }, { -41,  57 }, { -30,  42 }, {   0,   0 }, {   0,   0 }, { -82,   4 }, { -62,   3 }, // 888
		{   0,   0 }, {   0,   0 }, { -41,  52 }, { -30,  38 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 896
		{   0,   0 }, {   0,   0 }, { -42,  48 }, { -31,  36 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 904
		{   0,   0 }, {   0,   0 }, { -42,  43 }, { -31,  32 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 912
		{   0,   0 }, {   0,   0 }, { -39,  39 }, { -29,  29 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 920
		{   0,   0 }, {   0,   0 }, { -43,  38 }, { -32,  28 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 928
		{   0,   0 }, {   0,   0 }, { -49,  38 }, { -36,  28 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 936
		{   0,   0 }, {   0,   0 }, { -54,  37 }, { -40,  27 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 944
		{   0,   0 }, {   0,   0 }, { -59,  36 }, { -44,  27 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 952
		{   0,   0 }, {   0,   0 }, { -62,  34 }, { -46,  25 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 960
		{   0,   0 }, {   0,   0 }, { -60,  29 }, { -44,  21 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 968
		{   0,   0 }, {   0,   0 }, { -58,  26 }, { -43,  19 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 976
		{   0,   0 }, {   0,   0 }, { -55,  21 }, { -41,  16 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 984
		{   0,   0 }, {   0,   0 }, { -52,  17 }, { -38,  13 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 992
		{   0,   0 }, {   0,   0 }, { -51,  15 }, { -38,  11 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1000
		{   0,   0 }, {   0,   0 }, { -54,  13 }, { -40,  10 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1008
		{   0,   0 }, {   0,   0 }, { -57,  10 }, { -42,   7 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1016
		{   0,   0 }, {   0,   0 }, { -60,   9 }, { -44,   7 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1024
		{   0,   0 }, {   0,   0 }, { -65,   6 }, { -48,   4 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1032
		{   0,   0 }, {   0,   0 }, { -68,   4 }, { -50,   3 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1040
		{   0,   0 }, {   0,   0 }, { -71,   3 }, { -53,   2 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1048
		{   0,   0 }, {   0,   0 }, { -73,   2 }, { -54,   1 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1056
		{   0,   0 }, {   0,   0 }, { -74,   1 }, { -55,   1 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1064
		{   0,   0 }, {   0,   0 }, { -75,   0 }, { -55,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1072
		{   0,   0 }, {   0,   0 }, { -75,  -1 }, { -55,  -1 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, // 1080
		};

};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_TESTO_H
