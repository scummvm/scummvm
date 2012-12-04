/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/textconsole.h"
#include "hopkins/lines.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

LinesManager::LinesManager() {
	for (int i = 0; i < 400; ++i) {
		Common::fill((byte *)&LigneZone[i], (byte *)&LigneZone[i] + sizeof(LigneZoneItem), 0);
		Common::fill((byte *)&Ligne[i], (byte *)&Ligne[i] + sizeof(LigneItem), 0);
	}
	for (int i = 0; i < 4000; ++i) {
		Common::fill((byte *)&SMOOTH[i], (byte *)&SMOOTH[i] + sizeof(SmoothItem), 0);
	}

	next_ligne = 0;
	TOTAL_LIGNES = 0;
	NV_LIGNEDEP = 0;
	NV_LIGNEOFS = 0;
	NV_POSI = 0;
	NVPX = 0;
	NVPY = 0;
	SMOOTH_SENS = 0;
	SMOOTH_X = SMOOTH_Y = 0;
}

void LinesManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void LinesManager::CLEAR_ZONE() {
	for (int idx = 0; idx < 400; ++idx)
		RETIRE_LIGNE_ZONE(idx);

	next_ligne = 0;
}

// Object Zone
int LinesManager::ZONE_OBJET(int a1, int a2) {
	int v2;

	v2 = 0;
	if ((uint)(a2 - 120) <= 0x21u)
		v2 = OPTI_ZONE(a1, 1, 0);
	if ((uint)(a2 - 154) <= 0x25u)
		v2 = OPTI_ZONE(a1, 7, 0);
	if ((uint)(a2 - 192) <= 0x25u)
		v2 = OPTI_ZONE(a1, 13, 0);
	if ((uint)(a2 - 230) <= 0x25u)
		v2 = OPTI_ZONE(a1, 19, 0);
	if ((uint)(a2 - 268) <= 0x25u)
		v2 = OPTI_ZONE(a1, 25, 1);
	if ((uint)(a2 - 268) <= 0x14u && a1 >= _vm->_graphicsManager.ofscroll + 424 && a1 <= _vm->_graphicsManager.ofscroll + 478)
		v2 = 30;
	if ((uint)(a2 - 290) <= 0x10u && a1 >= _vm->_graphicsManager.ofscroll + 424 && a1 <= _vm->_graphicsManager.ofscroll + 478)
		v2 = 31;
	if (a1 < _vm->_graphicsManager.ofscroll + 152 || a1 > _vm->_graphicsManager.ofscroll + 484)
		v2 = 32;
	if ((uint)(a2 - 114) > 0xC0u)
		v2 = 32;
	return v2;
}

int LinesManager::OPTI_ZONE(int a1, int a2, int a3) {
	int v3; 
	signed int v4;

	v3 = a2;
	v4 = 0;
	if (a1 >= _vm->_graphicsManager.ofscroll + 158 && a1 <= _vm->_graphicsManager.ofscroll + 208)
		v4 = 1;
	if (!v4) {
		if (a1 >= _vm->_graphicsManager.ofscroll + 208 && a1 <= _vm->_graphicsManager.ofscroll + 266) {
			v3 = a2 + 1;
			v4 = 1;
		}
		if (!v4) {
			if (a1 >= _vm->_graphicsManager.ofscroll + 266 && a1 <= _vm->_graphicsManager.ofscroll + 320) {
				v3 += 2;
				v4 = 1;
			}
			if (!v4) {
				if (a1 >= _vm->_graphicsManager.ofscroll + 320 && a1 <= _vm->_graphicsManager.ofscroll + 370) {
					v3 += 3;
					v4 = 1;
				}
				if (!v4) {
					if (a1 >= _vm->_graphicsManager.ofscroll + 370 && a1 <= _vm->_graphicsManager.ofscroll + 424) {
						v3 += 4;
						v4 = 1;
					}
					if (!v4) {
						if (!a3 && a1 >= _vm->_graphicsManager.ofscroll + 424 && a1 <= _vm->_graphicsManager.ofscroll + 478) {
							v3 += 5;
							v4 = 1;
						}
						if (!v4)
							v3 = 0;
					}
				}
			}
		}
	}
	return v3;
}

// Remove Line Zone
void LinesManager::RETIRE_LIGNE_ZONE(int idx) {
	if (idx > 400)
		error("Attempting to add a line obstacle > MAX_LIGNE.");
	if (_vm->_linesManager.LigneZone[idx].zoneData != (int16 *)g_PTRNUL)
		_vm->_globals.dos_free2((byte *)_vm->_linesManager.LigneZone[idx].zoneData);

	_vm->_linesManager.LigneZone[idx].zoneData = (int16 *)g_PTRNUL;
}

// Add Line Zone
void LinesManager::AJOUTE_LIGNE_ZONE(int idx, int a2, int a3, int a4, int a5, int a6) {
	int16 *zoneData;

	if (a2 != a3 || a3 != a4 || a3 != a5) {
		if (idx > 400)
			error("Attempting to add a line obstacle > MAX_LIGNE.");

		if (LigneZone[idx].zoneData != (int16 *)g_PTRNUL)
			LigneZone[idx].zoneData = (int16 *)_vm->_globals.dos_free2((byte *)LigneZone[idx].zoneData);
		int v8 = a2 - a4;
		if (a2 - a4 < 0)
			v8 = -v8;
		int v19 = v8;
		int v9 = a3 - a5;
		if (a3 - a5 < 0)
			v9 = -v9;
		int v18 = v9;
		int v20 = 1;
		if (v19 <= v9)
			v20 += v9;
		else
			v20 += v19;

		zoneData = (int16 *)_vm->_globals.dos_malloc2(2 * sizeof(int16) * v20 + (4 * sizeof(int16)));
		int v11 = idx;
		LigneZone[v11].zoneData = zoneData;
		if (zoneData == (int16 *)g_PTRNUL)
			error("AJOUTE LIGNE ZONE");

		int16 *dataP = zoneData;
		int v23 = 1000 * v19 / v20;
		int v22 = 1000 * v18 / v20;
		if (a4 < a2)
			v23 = -v23;
		if (a5 < a3)
			v22 = -v22;
		int v13 = 1000 * a2;
		int v16 = 1000 * a3;
		int v17 = 1000 * a2 / 1000;
		int v21 = 1000 * a3 / 1000;
		if (v20 > 0) {
			for (int i = 0; i < v20; i++) {
				*dataP++ = v17;
				*dataP++ = v21;

				v13 += v23;
				v16 += v22;
				v17 = v13 / 1000;
				v21 = v16 / 1000;
			}
		}
		*dataP++ = -1;
		*dataP++ = -1;
		
		LigneZone[idx].count = v20;
		LigneZone[idx].field2 = a6;
	} else {
		_vm->_globals.BOBZONE_FLAG[a6] = true;
		_vm->_globals.BOBZONE[a6] = a3;
	}
}


void LinesManager::RESET_OBSTACLE() {
	for (int idx =0; idx < 400; ++idx) {
		RETIRE_LIGNE(idx);
		Ligne[idx].field0 = 0;
		Ligne[idx].lineData = (int16 *)g_PTRNUL;
	}
}

// Remove Line
void LinesManager::RETIRE_LIGNE(int idx) {
	if (idx > 400)
		error("Attempting to add a line obstacle > MAX_LIGNE.");
	if (Ligne[idx].lineData != (int16 *)g_PTRNUL)
		_vm->_globals.dos_free2((byte *)Ligne[idx].lineData);
	Ligne[idx].lineData = (int16 *)g_PTRNUL;
}

// Add Line
void LinesManager::AJOUTE_LIGNE(int idx, int a2, int a3, int a4, int a5, int a6, int a7) {
	int v7; 
	int v8; 
	int v9; 
	byte *v10; 
	int v11; 
	int v12;
	int v13;
	int v24;
	int v25;
	int v26;
	int16 *v27;
	int v30;
	int v31;
	int16 *v32;
	int v33;
	int v34;
	int v35;
	int v36;
	int v37;
	int v38;
	int v39;
	int v40;

	if (idx > 400)
		error("Attempting to add a line obstacle > MAX_LIGNE.");
	if (TOTAL_LIGNES < idx)
		TOTAL_LIGNES = idx;

	if (Ligne[idx].lineData != (int16 *)g_PTRNUL)
		_vm->_globals.dos_free2((byte *)Ligne[idx].lineData);
	v7 = a3 - a5;
	if (a3 - a5 < 0)
		v7 = -v7;
	v8 = v7 + 1;
	v9 = a4 - a6;
	if (a4 - a6 < 0)
		v9 = -v9;
	v34 = v9 + 1;
	v33 = v9 + 1;
	if (v8 > (int)(v9 + 1))
		v34 = v8;
	
	v10 = _vm->_globals.dos_malloc2(4 * v34 + 8);
	if (v10 == g_PTRNUL)
		error("AJOUTE LIGNE OBSTACLE");

	Common::fill(v10, v10 + 4 * v34 + 8, 0);
	Ligne[idx].lineData = (int16 *)v10;

	v32 = Ligne[idx].lineData;
	v36 = 1000 * v8;
	v39 = 1000 * v8 / (v34 - 1);
	v37 = 1000 * v33 / (v34 - 1);
	if (a5 < a3)
		v39 = -v39;
	if (a6 < a4)
		v37 = -v37;
	v11 = (int)v39 / 1000;
	v12 = (int)v37 / 1000;
	v13 = (int)v37 / 1000;
	if (!((int)v39 / 1000)) {
		if (v12 == -1) {
			Ligne[idx].field6 = 1;
			Ligne[idx].field8 = 5;
		}
		if (v12 == 1) {
			Ligne[idx].field6 = 5;
			Ligne[idx].field8 = 1;
		}
	}
	if (v11 == 1) {
		if (v12 == -1) {
			Ligne[idx].field6 = 2;
			Ligne[idx].field8 = 6;
		}
		if (!v13) {
			Ligne[idx].field6 = 3;
			Ligne[idx].field8 = 7;
		}
		if (v13 == 1) {
			Ligne[idx].field6 = 4;
			Ligne[idx].field8 = 8;
		}
	}
	if (v11 == -1) {
		if (v13 == 1) {
			Ligne[idx].field6 = 6;
			Ligne[idx].field8 = 2;
		}
		if (!v13) {
			Ligne[idx].field6 = 7;
			Ligne[idx].field8 = 3;
		}
		if (v13 == -1) {
			Ligne[idx].field6 = 8;
			Ligne[idx].field8 = 4;
		}
	}
	if (v11 == 1 && (unsigned int)(v37 - 251) <= 0x2EC) {
		Ligne[idx].field6 = 4;
		Ligne[idx].field8 = 8;
	}
	if (v11 == -1 && (unsigned int)(v37 - 251) <= 0x2EC) {
		Ligne[idx].field6 = 6;
		Ligne[idx].field8 = 2;
	}
	if (v11 == 1 && (unsigned int)(v37 + 999) <= 0x2EC) {
		Ligne[idx].field6 = 2;
		Ligne[idx].field8 = 6;
	}
	// CHECKME: v37 conditions are impossible to meet!
	if (v11 == -1 && v37 <= 249 && v37 > 1000) {
		Ligne[idx].field6 = 8;
		Ligne[idx].field8 = 4;
	}
	v40 = v36 / v34;
	v38 = 1000 * v33 / v34;
	if (a5 < a3)
		v40 = -v40;
	if (a6 < a4)
		v38 = -v38;
	v24 = 1000 * a3;
	v25 = 1000 * a4;
	v31 = 1000 * a3 / 1000;
	v30 = 1000 * a4 / 1000;
	v35 = v34 - 1;
	v26 = 0;
	if (v35 > 0) {
		do {
			v32[0] = v31;
			v32[1] = v30;
			v32 += 2;

			v24 += v40;
			v25 += v38;
			v31 = v24 / 1000;
			v30 = v25 / 1000;
			++v26;
		} while (v35 > v26);
	}
	v32[0] = a5;
	v32[1] = a6;

	v27 = v32 + 2;
	v27[0] = -1;
	v27[1] = -1;
	
	Ligne[idx].field0 = v35 + 1;
	Ligne[idx].field2 = a7;
	Ligne[idx].field4 = a2;
}

// Line Collision 2
// TODO: Should return a bool
int LinesManager::colision2_ligne(int a1, int a2, int *a3, int *a4, int a5, int a6) {
	int16 *v7; 
	int16 *v13; 
	int result; 

	int v24 = a5;
	int v6 = a5;
	if (a5 >= a6 + 1) {
LABEL_29:
		result = 0;
	} else {
		int v11;
		int v22 = a1 + 4;
		int v21 = a1 - 4;
		int v20 = a2 + 4;
		int v19 = a2 - 4;
		int v17;
		for (;;) {
			v7 = Ligne[v6].lineData;

			if (v7 != (int16 *)g_PTRNUL) {
				int v23 = 1;
				int v8 = 2 * Ligne[v6].field0;
				int v9 = v7[0];
				int v16 = v7[1];
				int v10 = v7[v8 - 2];
				int v18 = v7[v8 - 1];
				if (v7[0] >= v10)
					goto LABEL_32;
				if (v22 < v9 || v21 > v10)
					v23 = 0;
				if (v9 >= v10) {
LABEL_32:
					if (v21 > v9 || v22 < v10)
						v23 = 0;
				}
				if (v16 >= v18)
					goto LABEL_33;
				if (v20 < v16 || v19 > v18)
					v23 = 0;
				if (v16 >= v18) {
LABEL_33:
					if (v19 > v16 || v20 < v18)
						v23 = 0;
				}
				if (v23 == 1) {
					v11 = 0;
					v17 = Ligne[v24].field0;
					if (v17 > 0)
						break;
				}
			}
LABEL_28:
			++v24;
			v6 = v24;
			if (v24 >= a6 + 1)
				goto LABEL_29;
		}
		for (;;) {
			int v12 = v7[0];
			
			v13 = v7 + 1;
			int v14 = v13[0];
			v7 = v13 + 1;

			if (a1 == v12 || a1 + 1 == v12) {
				if (a2 == v14 || a2 + 1 == v14)
					break;
			}
			++v11;
			if (v17 <= v11)
				goto LABEL_28;
		}
		*a3 = v11;
		*a4 = v24;
		result = 1;
	}
	return result;
}

int LinesManager::Scolision2_ligne(int a1, int a2, int *a3, int *a4, int a5, int a6) {
	int16 *v7; 
	int16 *v13;
	int result;

	int v24 = a5;
	int v6 = a5;

	if (a5 >= a6 + 1) {
LABEL_47:
		result = 0;
	} else {
		int v22 = a1 + 10;
		int v21 = a1 - 10;
		int v20 = a2 + 10;
		int v19 = a2 - 10;
		int v11, v18;
		for (;;) {
			v7 = Ligne[v6].lineData;

			if (v7 != (int16 *)g_PTRNUL) {
				int v23 = 1;
				int v8 = 2 * Ligne[v6].field0;
				int v9 = v7[0];
				int v15 = v7[1];
				int v10 = v7[v8 - 2];
				int v17 = v7[v8 - 1];
				if (v7[0] >= v10)
					goto LABEL_50;
				if (v22 < v9 || v21 > v10)
					v23 = 0;
				if (v9 >= v10) {
LABEL_50:
					if (v21 > v9 || v22 < v10)
						v23 = 0;
				}
				if (v15 >= v17)
					goto LABEL_51;
				if (v20 < v15 || v19 > v17)
					v23 = 0;
				if (v15 >= v17) {
LABEL_51:
					if (v19 > v15 || v20 < v17)
						v23 = 0;
				}
				if (v23 == 1) {
					v11 = 0;
					v18 = Ligne[v24].field0;
					if (v18 > 0)
						break;
				}
			}
LABEL_46:
			++v24;
			v6 = v24;
			if (v24 >= a6 + 1)
				goto LABEL_47;
		}
		for (;;) {
			int v12 = v7[0];
			v13 = v7 + 1;
			int v16 = v13[0];
			v7 = v13 + 1;
			if (a1 == v12
			        || v12 == a1 + 1
			        || v12 == a1 + 2
			        || v12 == a1 + 3
			        || v12 == a1 + 4
			        || v12 == a1 + 5
			        || v12 == a1 + 6
			        || v12 == a1 + 7
			        || v12 == a1 + 8
			        || v12 == a1 + 9
			        || a1 + 10 == v12) {
				if (v16 == a2
				        || v16 == a2 + 1
				        || v16 == a2 + 2
				        || v16 == a2 + 3
				        || v16 == a2 + 4
				        || v16 == a2 + 5
				        || v16 == a2 + 6
				        || v16 == a2 + 7
				        || v16 == a2 + 8
				        || v16 == a2 + 9
				        || a2 + 10 == v16)
					break;
			}
			++v11;
			if (v18 <= v11)
				goto LABEL_46;
		}
		*a3 = v11;
		*a4 = v24;
		result = 1;
	}
	return result;
}

// Init route
void LinesManager::INIPARCOURS() {
	int v0; 
	int v1; 
	int v2; 
	int16 *v3; 
	int v4; 
	int v5; 
	int v6; 
	int16 *v7; 
	int v8; 
	int v9; 
	int v11;
	int v12;
	int v13;
	int v14;
	int v15;

	v15 = Ligne[0].lineData[0];
	v14 = Ligne[0].lineData[1];

	v0 = 1;
	v1 = 0;
	do {
		v2 = Ligne[v0].field0;
		v3 = Ligne[v0].lineData;

		v4 = v3[2 * v2 - 2];
		v5 = v3[2 * v2 - 1];
		if (_vm->_graphicsManager.max_x == v4 || _vm->_graphicsManager.max_y == v5 || _vm->_graphicsManager.min_x == v4 
				|| _vm->_graphicsManager.min_y == v5)
			v1 = 1;
		if (v15 == v4 && v14 == v5)
			v1 = 1;
		if (v0 == 400) {
			v6 = v1;
			error("ERROR - LAST LINE NOT FOUND");
			v1 = v6;
		}
		v7 = Ligne[v0 + 1].lineData;
		if (v7[0] != v4 && v7[1] != v5)
			v1 = 1;
		if (!v1)
			++v0;
	} while (v1 != 1);

	_vm->_objectsManager.DERLIGNE = v0;
	v8 = 1;
	v13 = _vm->_globals.Max_Ligne_Long;
	v9 = _vm->_objectsManager.DERLIGNE + 1;
	do {
		v11 = v8;
		if (Ligne[v11].field0 < v13) {
			if (v8 != v9) {
				v12 = v8 - 1;
				Ligne[v11].field6 = Ligne[v12].field6;
				Ligne[v11].field8 = Ligne[v12].field8;
			}
		}
		++v8;
	} while (v8 != 400);
}

// Avoid
int LinesManager::CONTOURNE(int a1, int a2, int a3, int a4, int a5, int16 *a6, int a7) {
	int v36 = a1;
	int v7 = a2;
	int v8 = a3;
	int v50;
	if (a1 < a4) {
		for (int i = a2; Ligne[a1].field0 > i; ++i) {
			int16 *v10 = Ligne[a1].lineData;
			int v11 = v10[2 * i];
			int v50 = v10[2 * i + 1];
			
			int v12 = v8;
			a6[v12] = v11;
			a6[v12 + 1] = v50;
			a6[v12 + 2] = Ligne[a1].field6;
			v8 += a7;
		}

		if ((int)(a1 + 1) < a4) {
			for (int v34 = a1 + 1; v34 < a4; v34++) {
				if (Ligne[v34].field0 > 0) {
					for (int i = 0; i < Ligne[v34].field0; i++) {
						int16 *v14 = Ligne[v34].lineData;
						int v15 = v14[2 * i];
						v50 = v14[2 * i + 1];
						int v16 = v8;

						a6[v16] = v15;
						a6[v16 + 1] = v50;
						a6[v16 + 2] = Ligne[v34].field6;
						v8 += a7;
					}
				}
			}
		}
		v7 = 0;
		v36 = a4;
	}
	if (v36 > a4) {
		for (int j = v7; j > 0; --j) {
			int16 *v18 = Ligne[v36].lineData;
			int v19 = v18[2 * j];
			v50 = v18[2 * j + 1];
			
			int v20 = v8;
			a6[v20] = v19;
			a6[v20 + 1] = v50;
			a6[v20 + 2] = Ligne[v36].field8;
			v8 += a7;
		}
		if ((int)(v36 - 1) > a4) {
			for (int v35 = v36 - 1; v35 > a4; v35--) {
				for (int k = Ligne[v35].field0 - 1; k > 0; --k) {
					int16 *v22 = Ligne[v35].lineData;
					int v23 = v22[2 * k];
					v50 = v22[2 * k + 1];

					int v24 = v8;
					a6[v24] = v23;
					a6[v24 + 1] = v50;
					a6[v24 + 2] = Ligne[v35].field8;
					v8 += a7;
				}

			}
		}
		v7 = Ligne[a4].field0 - 1;
		v36 = a4;
	}
	if (v36 == a4) {
		if (a5 >= v7) {
			if (a5 > v7) {
				for (int i = v7; i > a5; i++) {
					int16 *v30 = Ligne[a4].lineData;
					int v31 = v30[2 * i];
					v50 = v30[2 * i + 1];

					int v32 = v8;
					a6[v32] = v31;
					a6[v32 + 1] = v50;
					a6[v32 + 2] = Ligne[a4].field6;
					v8 += a7;
				}
			}
		} else {
			for (int i = v7; i > a5; --i) {
				int16 *v26 = Ligne[a4].lineData;
				int v27 = v26[2 * i];
				v50 = v26[2 * i+ 1];

				int v28 = v8;
				a6[v28] = v27;
				a6[v28 + 1] = v50;
				a6[v28 + 2] = Ligne[a4].field8;
				v8 += a7;
			}
		}
	}
	return v8;
}

// Avoid 1
int LinesManager::CONTOURNE1(int a1, int a2, int a3, int a4, int a5, int16 *a6, int a7, int a8, int a9) {
	int v9 = a1;
	int v10 = a2;
	int v40 = a3;
	int v50;
	if (a4 < a1) {
		for (int i = a2; Ligne[a1].field0 > i; ++i) {
			int16 *v12 = Ligne[a1].lineData;
			int v13 = v12[2 * i];
			v50 = v12[2 * i + 1];

			int v14 = v40;
			a6[v14] = v13;
			a6[v14 + 1] = v50;
			a6[v14 + 2] = Ligne[a1].field6;
			v40 += a7;
		}
		int v15 = a1 + 1;
		if ((int)(a1 + 1) == a9 + 1)
			v15 = a8;
		while (a4 != v15) {
			if (Ligne[v15].field0 > 0) {
				for (int v16 = 0; v16 < Ligne[v15].field0; v16++) {
					int16 *v17 = Ligne[v15].lineData;
					int v18 = v17[2 * v16];
					v50 = v17[2 * v16 + 1];

					int v19 = v40;
					a6[v19] = v18;
					a6[v19 + 1] = v50;
					a6[v19 + 2] = Ligne[v15].field6;
					v40 += a7;
	
				}
			}
			++v15;
			if (a9 + 1 == v15)
				v15 = a8;
		}
		v10 = 0;
		v9 = a4;
	}
	if (a4 > v9) {
		for (int j = v10; j > 0; --j) {
			int16 *v21 = Ligne[v9].lineData;
			int v22 = v21[2 * j];
			v50 = v21[2 * j + 1];

			int v23 = v40;
			a6[v23] = v22;
			a6[v23 + 1] = v50;
			a6[v23 + 2] = Ligne[v9].field8;
			v40 += a7;
		}
		int v24 = v9 - 1;
		if (v24 == a8 - 1)
			v24 = a9;
		while (a4 != v24) {
			for (int k = Ligne[v24].field0 - 1; k > 0; --k) {
				int16 *v26 = Ligne[v24].lineData;
				int v27 = v26[2 * k];
				v50 = v26[2 * k + 1];

				int v28 = v40;
				a6[v28] = v27;
				a6[v28 + 1] = v50;
				a6[v28 + 2] = Ligne[v24].field8;
				v40 += a7;
			}
			--v24;
			if (a8 - 1 == v24)
				v24 = a9;
		}
		v10 = Ligne[a4].field0 - 1;
		v9 = a4;
	}
	if (a4 == v9) {
		if (a5 >= v10) {
			if (a5 > v10) {
				int v39 = a4;
				for (int v33 = v10; v33 < a5; v33++) {
					int16 *v34 = Ligne[v39].lineData;
					int v35 = v34[2 * v33];
					v50 = v34[2 * v33 + 1];

					int v36 = v40;
					a6[v36] = v35;
					a6[v36 + 1] = v50;
					a6[v36 + 2] = Ligne[v39].field6;
					v40 += a7;
				}
			}
		} else {
			int v38 = a4;
			for (int v29 = v10; v29 > a5; v29--) {
				int16 *v30 = Ligne[v38].lineData;
				int v31 = v30[2 * v29];
				v50 = v30[2 * v29 + 1];

				int v32 = v40;
				a6[v32] = v31;
				a6[v32 + 1] = v50;
				a6[v32 + 2] = Ligne[v38].field8;
				v40 += a7;
			}
		}
	}
	return v40;
}

int LinesManager::MIRACLE(int a1, int a2, int a3, int a4, int a5) {
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10;
	int i; 
	int v12;
	int v13;
	int j; 
	int v15;
	int v16;
	int k; 
	int v18;
	int v19;
	int l; 
	int v21;
	int v22; 
	int v23; 
	int v24; 
	int v25;
	int v26;
	int v27; 
	int v28;
	int v29;
	int v30; 
	int v31;
	int v32;
	int v33; 
	int v35 = 0; 
	int v36 = 0; 
	int v37; 
	int v38; 
	int v39; 
	int v40; 
	int v41; 
	int v42 = 0; 
	int v43 = 0; 
	int v44 = 0; 
	int v45 = 0; 
	int v46 = 0; 
	int v47 = 0; 
	int v48 = 0; 
	int v49 = 0; 
	int v50; 
	int v51; 

	v5 = a1;
	v6 = a2;
	v50 = a3;
	v7 = a5;
	if (colision2_ligne(a1, a2, &v51, &v50, 0, TOTAL_LIGNES) == 1) {
		v8 = Ligne[v50].field4;
		if (v8 == 1)
			v6 = a2 - 2;
		if (v8 == 2) {
			v6 -= 2;
			v5 = a1 + 2;
		}
		if (v8 == 3)
			v5 += 2;
		if (v8 == 4) {
			v6 += 2;
			v5 += 2;
		}
		if (v8 == 5)
			v6 += 2;
		if (v8 == 6) {
			v6 += 2;
			v5 -= 2;
		}
		if (v8 == 7)
			v5 -= 2;
		if (v8 == 8) {
			v6 -= 2;
			v5 -= 2;
		}
	}
	v41 = v5;
	v40 = v6;
	v9 = 0;
	v10 = v40;
	for (i = v40; v40 + 200 > v10; i = v10) {
		if (colision2_ligne(v41, i, &v49, &v48, 0, _vm->_objectsManager.DERLIGNE) == 1 && v48 <= _vm->_objectsManager.DERLIGNE)
			break;
		v49 = 0;
		v48 = -1;
		++v9;
		++v10;
	}
	v37 = v9;
	v12 = 0;
	v13 = v40;
	for (j = v40; v40 - 200 < v13; j = v13) {
		if (colision2_ligne(v41, j, &v47, &v46, 0, _vm->_objectsManager.DERLIGNE) == 1 && v46 <= _vm->_objectsManager.DERLIGNE)
			break;
		v47 = 0;
		v46 = -1;
		++v12;
		--v13;
	}
	v39 = v12;
	v15 = 0;
	v16 = v41;
	for (k = v41; v41 + 200 > v16; k = v16) {
		if (colision2_ligne(k, v40, &v45, &v44, 0, _vm->_objectsManager.DERLIGNE) == 1 && v44 <= _vm->_objectsManager.DERLIGNE)
			break;
		v45 = 0;
		v44 = -1;
		++v15;
		++v16;
	}
	v38 = v15;
	v18 = 0;
	v19 = v41;
	for (l = v41; v41 - 200 < v19; l = v19) {
		if (colision2_ligne(l, v40, &v43, &v42, 0, _vm->_objectsManager.DERLIGNE) == 1 && v42 <= _vm->_objectsManager.DERLIGNE)
			break;
		v43 = 0;
		v42 = -1;
		++v18;
		--v19;
	}
	if (a4 > v50) {
		if (v46 != -1 && v46 <= v50)
			v46 = -1;
		if (v44 != -1 && v50 >= v44)
			v44 = -1;
		if (v48 != -1 && v50 >= v48)
			v48 = -1;
		if (v42 != -1 && v50 >= v42)
			v42 = -1;
		if (v46 != -1 && a4 < v46)
			v46 = -1;
		if (v44 != -1 && a4 < v44)
			v44 = -1;
		if (v48 != -1 && a4 < v48)
			v48 = -1;
		if (v42 != -1 && a4 < v42)
			v42 = -1;
	}
	if (a4 < v50) {
		if (v46 != -1 && v46 >= v50)
			v46 = -1;
		if (v44 != -1 && v50 <= v44)
			v44 = -1;
		if (v48 != -1 && v50 <= v48)
			v48 = -1;
		if (v42 != -1 && v50 <= v42)
			v42 = -1;
		if (v46 != -1 && a4 > v46)
			v46 = -1;
		if (v44 != -1 && a4 > v44)
			v44 = -1;
		if (v48 != -1 && a4 > v48)
			v48 = -1;
		if (v42 != -1 && a4 > v42)
			v42 = -1;
	}
	if (v46 != -1 || v44 != -1 || v48 != -1 || v42 != -1) {
		v21 = 0;
		if (a4 > v50) {
			if (v48 <= v46 && v44 <= v46 && v42 <= v46 && v46 > v50)
				v21 = 1;
			if (v48 <= v44 && v46 <= v44 && v42 <= v44 && v50 < v44)
				v21 = 3;
			if (v46 <= v48 && v44 <= v48 && v42 <= v48 && v50 < v48)
				v21 = 5;
			if (v48 <= v42 && v44 <= v42 && v46 <= v42 && v50 < v42)
				v21 = 7;
		}
		if (a4 < v50) {
			if (v46 == -1)
				v46 = 1300;
			if (v44 == -1)
				v44 = 1300;
			if (v48 == -1)
				v48 = 1300;
			if (v42 == -1)
				v42 = 1300;
			if (v46 != 1300 && v48 >= v46 && v44 >= v46 && v42 >= v46 && v46 < v50)
				v21 = 1;
			if (v44 != 1300 && v48 >= v44 && v46 >= v44 && v42 >= v44 && v50 > v44)
				v21 = 3;
			if (v48 != 1300 && v46 >= v48 && v44 >= v48 && v42 >= v48 && v50 > v48)
				v21 = 5;
			if (v42 != 1300 && v48 >= v42 && v44 >= v42 && v46 >= v42 && v50 > v42)
				v21 = 7;
		}
		if (v21) {
			if (v21 == 1) {
				v36 = v46;
				v35 = v47;
			}
			if (v21 == 3) {
				v36 = v44;
				v35 = v45;
			}
			if (v21 == 5) {
				v36 = v48;
				v35 = v49;
			}
			if (v21 == 7) {
				v36 = v42;
				v35 = v43;
			}
			if (v21 == 1) {
				v22 = 0;
				if (v39 > 0) {
					do {
						if (colision2_ligne(v41, v40 - v22, &v47, &v46, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
						        && _vm->_objectsManager.DERLIGNE < v46) {
									v23 = GENIAL(v46, v47, v41, v40 - v22, v41, v40 - v39, v7, &_vm->_globals.super_parcours[0], 4);
							if (v23 == -1)
								return 0;
							v7 = v23;
							if (NVPY != -1)
								v22 = NVPY - v40;
						}
						v24 = v7;
						_vm->_globals.super_parcours[v24] = v41;
						_vm->_globals.super_parcours[v24 + 1] = v40 - v22;
						_vm->_globals.super_parcours[v24 + 2] = 1;
						_vm->_globals.super_parcours[v24 + 3] = 0;
						v7 += 4;
						++v22;
					} while (v39 > v22);
				}
LABEL_186:
				NV_LIGNEDEP = v36;
				NV_LIGNEOFS = v35;
				NV_POSI = v7;
				return 1;
			}
			if (v21 == 5) {
				v25 = 0;
				if (v37 > 0) {
					do {
						if (colision2_ligne(v41, v25 + v40, &v47, &v46, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
						        && _vm->_objectsManager.DERLIGNE < v46) {
							v26 = GENIAL(v46, v47, v41, v25 + v40, v41, v37 + v40, v7, &_vm->_globals.super_parcours[0], 4);
							if (v26 == -1)
								return 0;
							v7 = v26;
							if (NVPY != -1)
								v25 = v40 - NVPY;
						}
						v27 = v7;
						_vm->_globals.super_parcours[v27] = v41;
						_vm->_globals.super_parcours[v27 + 1] = v25 + v40;
						_vm->_globals.super_parcours[v27 + 2] = 5;
						_vm->_globals.super_parcours[v27 + 3] = 0;
						v7 += 4;
						++v25;
					} while (v37 > v25);
				}
				goto LABEL_186;
			}
			if (v21 == 7) {
				v28 = 0;
				if (v18 > 0) {
					do {
						if (colision2_ligne(v41 - v28, v40, &v47, &v46, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
						        && _vm->_objectsManager.DERLIGNE < v46) {
							v29 = GENIAL(v46, v47, v41 - v28, v40, v41 - v18, v40, v7, &_vm->_globals.super_parcours[0], 4);
							if (v29 == -1)
								return 0;
							v7 = v29;
							if (NVPX != -1)
								v28 = v41 - NVPX;
						}
						v30 = v7;
						_vm->_globals.super_parcours[v30] = v41 - v28;
						_vm->_globals.super_parcours[v30 + 1] = v40;
						_vm->_globals.super_parcours[v30 + 2] = 7;
						_vm->_globals.super_parcours[v30 + 3] = 0;
						v7 += 4;
						++v28;
					} while (v18 > v28);
				}
				goto LABEL_186;
			}
			if (v21 == 3) {
				v31 = 0;
				if (v38 > 0) {
					do {
						if (colision2_ligne(v31 + v41, v40, &v47, &v46, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
						        && _vm->_objectsManager.DERLIGNE < v46) {
							v32 = GENIAL(v46, v47, v31 + v41, v40, v38 + v41, v40, v7, &_vm->_globals.super_parcours[0], 4);
							if (v32 == -1)
								return 0;
							v7 = v32;
							if (NVPX != -1)
								v31 = NVPX - v41;
						}
						v33 = v7;
						_vm->_globals.super_parcours[v33] = v31 + v41;
						_vm->_globals.super_parcours[v33 + 1] = v40;
						_vm->_globals.super_parcours[v33 + 2] = 3;
						_vm->_globals.super_parcours[v33 + 3] = 0;
						v7 += 4;
						++v31;
					} while (v38 > v31);
				}
				goto LABEL_186;
			}
		}
	}
	return 0;
}

int LinesManager::GENIAL(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int16 *a8, int a9) {
	int v99 = a7;
	int v80 = -1;
	int v9;
	++_vm->_globals.STOP_BUG;
	if (_vm->_globals.STOP_BUG > 10) {
		v9 = a7;
LABEL_112:
		a8[v9] = -1;
		a8[v9 + 1] = -1;
		a8[v9 + 2] = -1;
		return -1;
	}
	int16 *v10 = Ligne[a1].lineData;
	int v98 = v10[0];
	int v97 = v10[1];
	int v92 = a1;

	int v65;
	for (;;) {
		int v86 = v92 - 1;
		int v11 = 2 * Ligne[v92 - 1].field0;

		int16 *v12 = Ligne[v92 - 1].lineData;
		if (v12 == (int16 *)g_PTRNUL)
			break;
		while (v12[v11 - 2] != v98 || v97 != v12[v11 - 1]) {
			--v86;
			if (_vm->_objectsManager.DERLIGNE - 1 != v86) {
				v11 = 2 * Ligne[v86].field0;
				v12 = Ligne[v86].lineData;
				if (v12 != (int16 *)g_PTRNUL)
					continue;
			}
			goto LABEL_11;
		}

		v92 = v86;
		v98 = v12[0];
		v97 = v12[1];
	}
LABEL_11:
	int16 *v13 = Ligne[a1].lineData;
	int v14 = 2 * Ligne[a1].field0;
	int v95 = v13[v14 - 2];
	int v93 = v13[v14 - 1];
	int v91 = a1;
	int v100, v101;
	for (;;) {
		int v87 = v91 + 1;
		int v15 = 2 * Ligne[v91 + 1].field0;
		int16 *v16 = Ligne[v91 + 1].lineData;
		if (v16 == (int16 *)g_PTRNUL)
			break;
		int v17;
		for (;;) {
			v65 = v15;
			v17 = v16[v15 - 2];
			if (v16[0] == v95) {
				if (v93 == v16[1])
					break;
			}
			++v87;
			if (v87 != TOTAL_LIGNES + 1) {
				v15 = 2 * Ligne[v87].field0;
				v16 = Ligne[v87].lineData;
				if (v16 != (int16 *)g_PTRNUL)
					continue;
			}
			goto LABEL_17;
		}
		v91 = v87;
		v95 = v17;
		v93 = v16[v65 - 1];
	}
LABEL_17:
	int v18 = a3 - a5;
	if (a3 - a5 < 0)
		v18 = -v18;
	int v58 = v18 + 1;
	int v19 = a4 - a6;
	if (a4 - a6 < 0)
		v19 = -v19;
	int v85 = v19 + 1;
	int v20 = v19 + 1;
	if (v58 > v20)
		v85 = v18 + 1;
	int v84 = 1000 * v58 / v85;
	int v83 = 1000 * v20 / v85;
	int v21 = 1000 * a3;
	int v22 = 1000 * a4;
	int v82 = v21 / 1000;
	int v81 = v22 / 1000;
	if (a5 < a3)
		v84 = -v84;
	if (a6 < a4)
		v83 = -v83;
	if (v85 > 800)
		v85 = 800;

	Common::fill(&_vm->_globals.BufLig[0], &_vm->_globals.BufLig[1000], 0);
	int v23 = 0;
	if (v85 + 1 > 0) {
		int16 *v51 = _vm->_globals.BufLig;
		for (int v88 = 0; v88 < v85 + 1; v88++) {
			int v24 = v23;
			v51[v24] = v82;
			v51[v24 + 1] = v81;
			v21 += v84;
			v22 += v83;
			v82 = v21 / 1000;
			v81 = v22 / 1000;
			v23 += 2;
		}
	}
	int v25 = v23 - 2;
	bool loopCond = false;
	int v77 = 0;
	int v78 = 0;
	int v79 = 0;
	if (v85 + 1 > 0) {
		for (int v89 = v85 + 1; v89 > 0; v89--) {
			int v96 = _vm->_globals.BufLig[v25];
			int v94 = _vm->_globals.BufLig[v25 + 1];
			if (colision2_ligne(v96, v94, &v101, &v100, v92, v91) == 1 && _vm->_objectsManager.DERLIGNE < v100) {
				v80 = v100;
				v77 = v101;
				v78 = v96;
				v79 = v94;
				loopCond = true;
			}
			if (loopCond)
				break;
			v25 -= 2;
		}
	}
	int v66 = 0;
	int v68 = 0;
	int v70 = 0;
	int v72 = 0;
	for (int i = v92; i < v91 + 1; ++i) {
		int v27 = 2 * Ligne[i].field0;
		int16 *v28 = Ligne[i].lineData;
		if (v28 == (int16 *)g_PTRNUL)
			error("error in genial routine");
		int v29 = v28[0];
		int v30 = v28[1];
		int v59 = v28[1];
		int v31 = v27;
		int v32 = v28[v27 - 2];
		int v33 = v28[v31 - 1];
		if (i == v92) {
			v72 = v33;
			if (v30 <= v33)
				v72 = v30;
			v70 = v33;
			if (v59 >= v33)
				v70 = v59;
			v68 = v32;
			if (v29 <= v32)
				v68 = v29;
			v66 = v32;
			if (v29 >= v32)
				v66 = v29;
		} else {
			if (v59 < v33 && v59 < v72)
				v72 = v59;
			if (v33 < v59 && v33 < v72)
				v72 = v33;
			if (v59 > v33 && v59 > v70)
				v70 = v59;
			if (v33 > v59 && v33 > v70)
				v70 = v33;
			if (v29 < v32 && v68 > v29)
				v68 = v29;
			if (v32 < v29 && v68 > v32)
				v68 = v32;
			if (v29 > v32 && v66 < v29)
				v66 = v29;
			if (v32 > v29 && v66 < v32)
				v66 = v32;
		}
	}
	int v69 = v68 - 2;
	int v73 = v72 - 2;
	int v67 = v66 + 2;
	int v71 = v70 + 2;
	if (a5 >= v69 && a5 <= v67 && a6 >= v73 && a6 <= v71) {
		int v34 = a6;
		int v76 = -1;
		int v60 = 0;
		do {
			--v34;
			v60 = colision2_ligne(a5, v34, &v101, &v100, v92, v91);
			if (v60 == 1)
				v76 = v100;
			if (!v34 || v73 > v34)
				v60 = 1;
		} while (v60 != 1);
		int v35 = a6;
		int v75 = -1;
		int v61 = 0;
		do {
			++v35;
			v61 = colision2_ligne(a5, v35, &v101, &v100, v92, v91);
			if (v61 == 1)
				v75 = v100;
			if (_vm->_globals.Max_Perso_Y <= v35 || v71 <= v35)
				v61 = 1;
		} while (v61 != 1);
		int v36 = a5;
		int v74 = -1;
		int v62 = 0;
		do {
			++v36;
			v62 = colision2_ligne(v36, a6, &v101, &v100, v92, v91);
			if (v62 == 1)
				v74 = v100;
			if (_vm->_graphicsManager.max_x <= v36 || v67 <= v36)
				v62 = 1;
		} while (v62 != 1);
		int v37 = a5;
		int v38 = -1;
		int v63 = 0;
		do {
			--v37;
			v63 = colision2_ligne(v37, a6, &v101, &v100, v92, v91);
			if (v63 == 1)
				v38 = v100;
			if (v37 <= 0 || v69 >= v37)
				v63 = 1;
		} while (v63 != 1);
		if (v74 != -1 && v38 != -1 && v76 != -1 && v75 != -1) {
			v9 = a7;
			goto LABEL_112;
		}
	}
	if (v78 < a3 - 1 || v78 > a3 + 1 || v79 < a4 - 1 || v79 > a4 + 1) {
		NVPX = v78;
		NVPY = v79;
		if (a1 < v80) {
			int v40 = v80 - a1;
			if (v40 < 0)
				v40 = -v40;
			int v41 = v40;
			int v42 = a1;
			int v43 = 0;
			int v52 = v92 - 1;
			do {
				if (v52 == v42)
					v42 = v91;
				++v43;
				--v42;
				if (v52 == v42)
					v42 = v91;
			} while (v80 != v42);
			if (v41 == v43) {
				int v44 = Ligne[a1].field0 / 2;
				int v54 = Ligne[a1].field0 / 2;
				if (v44 < 0)
					v54 = -v44;
				if (a2 > v54) {
					int v55 = Ligne[a1].field0 / 2;
					if (v44 < 0)
						v55 = -v44;
					if (a2 >= v55)
						v99 = CONTOURNE(a1, a2, a7, v80, v77, a8, a9);
				} else {
					v99 = CONTOURNE1(a1, a2, a7, v80, v77, a8, a9, v92, v91);
				}
			}
			if (v41 < v43)
				v99 = CONTOURNE(a1, a2, v99, v80, v77, a8, a9);
			if (v43 < v41)
				v99 = CONTOURNE1(a1, a2, v99, v80, v77, a8, a9, v92, v91);
		}
		if (a1 > v80) {
			int v45 = a1 - v80;
			if (v45 < 0)
				v45 = -v45;
			int v46 = v45;
			int v47 = a1;
			int v48 = 0;
			int v53 = v91 + 1;
			do {
				if (v53 == v47)
					v47 = v92;
				++v48;
				++v47;
				if (v53 == v47)
					v47 = v92;
			} while (v80 != v47);
			if (v46 == v48) {
				int v49 = Ligne[a1].field0 / 2;
				int v56 = Ligne[a1].field0 / 2;
				if (v49 < 0)
					v56 = -v49;
				if (a2 > v56) {
					int v57 = Ligne[a1].field0 / 2;
					if (v49 < 0)
						v57 = -v49;
					if (a2 >= v57)
						v99 = CONTOURNE1(a1, a2, v99, v80, v77, a8, a9, v92, v91);
				} else {
					v99 = CONTOURNE(a1, a2, v99, v80, v77, a8, a9);
				}
			}
			if (v46 < v48)
				v99 = CONTOURNE(a1, a2, v99, v80, v77, a8, a9);
			if (v48 < v46)
				v99 = CONTOURNE1(a1, a2, v99, v80, v77, a8, a9, v92, v91);
		}
		if (a1 == v80)
			v99 = CONTOURNE(a1, a2, v99, a1, v77, a8, a9);
		int v64 = 0;
		do {
			v64 = colision2_ligne(NVPX, NVPY, &v101, &v100, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES);
			if (v64 == 1) {
				int v50 = v100;
				if (Ligne[v50].field4 == 1)
					--NVPY;
				if (Ligne[v50].field4 == 2) {
					--NVPY;
					++NVPX;
				}
				if (Ligne[v50].field4 == 3)
					++NVPX;
				if (Ligne[v50].field4 == 4) {
					++NVPY;
					++NVPX;
				}
				if (Ligne[v50].field4 == 5)
					++NVPY;
				if (Ligne[v50].field4 == 6) {
					++NVPY;
					--NVPX;
				}
				if (Ligne[v50].field4 == 7)
					--NVPX;
				if (Ligne[v50].field4 == 8) {
					--NVPY;
					--NVPX;
				}
			}
		} while (v64);
	} else {
		NVPX = -1;
		NVPY = -1;
	}
	return v99;
}

// Avoid 2
int16 *LinesManager::PARCOURS2(int srcX, int srcY, int destX, int destY) {
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10;
	int v11;
	int v12;
	int v14;
	int v15;
	int v16;
	int v17;
	int v18;
	int v19;
	int v20;
	int v21;
	int v22;
	int v23;
	int v24;
	int v25;
	int v26;
	int v27;
	int v28;
	int v29;
	int v31;
	int v34; 
	int v35;
	int v36;
	int v37;
	int v38;
	int v39;
	int v40;
	int v41;
	int v42;
	int v43;
	int v44;
	int v45;
	int v46;
	int v47;
	int v48;
	int v49;
	int v50;
	int v51;
	int v52;
	int v53;
	int v54;
	int v55;
	int v56;
	int v57;
	int v58;
	int v59;
	int v60;
	int v61;
	int v62;
	int v63;
	int v64;
	int v65;
	int v66;
	int v67;
	int v68;
	int i;
	int16 *v70;
	int v71;
	int v72;
	int j;
	int16 *v74;
	int v75; 
	int v76; 
	int v77; 
	int v78; 
	int v79; 
	int v80; 
	int16 *v81;
	int v82; 
	int v83; 
	int16 *v84; 
	int v85; 
	int v86; 
	int v87; 
	int v88; 
	int v89; 
	int v90;
	int16 *v91; 
	int v92;
	int v93; 
	int v94;
	int16 *v95; 
	int v96;
	int v97; 
	int v98; 
	int v99; 
	int v100; 
	int v101;
	int v102;
	int v103;
	int v104;
	int v105;
	int v106;
	int v107;
	int v108;
	int v109; 
	int v110; 
	int v111; 
	int v112; 
	int v113;
	int v114; 
	int v115; 
	int v116; 
	int v117; 
	int v118; 
	int v119; 
	int v120; 
	int v121; 
	int v122; 
	int v123; 
	int v124; 
	int v125; 
	int v126[9];
	int v131[9];
	int v136[9];
	int v141[9];

	v123 = destX;
	v122 = destY;
	v121 = 0;
	v120 = 0;
	v115 = 0;
	v114 = 0;
	v113 = 0;
	v111 = 0;
	if (destY <= 24)
		v122 = 25;
	if (!_vm->_globals.NOT_VERIF) {
		v4 = srcX - _vm->_globals.old_x1_65;
		if (v4 < 0)
			v4 = -v4;
		if (v4 <= 4) {
			v5 = srcY - _vm->_globals.old_y1_66;
			if (v5 < 0)
				v5 = -v5;
			if (v5 <= 4) {
				v6 = _vm->_globals.old_x2_67 - destX;
				if (v6 < 0)
					v6 = -v6;
				if (v6 <= 4) {
					v7 = _vm->_globals.old_y2_68 - v122;
					if (v7 < 0)
						v7 = -v7;
					if (v7 <= 4)
						return (int16 *)g_PTRNUL;
				}
			}
		}
		v8 = srcX - destX;
		if (v8 < 0)
			v8 = -v8;
		if (v8 <= 4) {
			v9 = srcY - v122;
			if (v9 < 0)
				v9 = -v9;
			if (v9 <= 4)
				return (int16 *)g_PTRNUL;
		}
		if (_vm->_globals.old_z_69 > 0 && _vm->_objectsManager.NUMZONE > 0 && _vm->_globals.old_z_69 == _vm->_objectsManager.NUMZONE)
			return (int16 *)g_PTRNUL;
	}
	_vm->_globals.NOT_VERIF = 0;
	_vm->_globals.old_z_69 = _vm->_objectsManager.NUMZONE;
	_vm->_globals.old_x1_65 = srcX;
	_vm->_globals.old_x2_67 = destX;
	_vm->_globals.old_y1_66 = srcY;
	_vm->_globals.old_y2_68 = v122;
	_vm->_globals.STOP_BUG = 0;
	v112 = 0;
	if (destX <= 19)
		v123 = 20;
	if (v122 <= 19)
		v122 = 20;
	if (v123 > _vm->_graphicsManager.max_x - 10)
		v123 = _vm->_graphicsManager.max_x - 10;
	if (v122 > _vm->_globals.Max_Perso_Y)
		v122 = _vm->_globals.Max_Perso_Y;
	v10 = srcX - v123;
	if (v10 < 0)
		v10 = -v10;
	if (v10 <= 3) {
		v11 = srcY - v122;
		if (v11 < 0)
			v11 = -v11;
		if (v11 <= 3)
			return (int16 *)g_PTRNUL;
	}
	
	for (v12 = 0; v12 <= 8; ++v12) {
		v141[v12] = -1;
		v136[v12] = 0;
		v131[v12] = 1300;
		v126[v12] = 1300;
		++v12;
	}

	if (PARC_PERS(srcX, srcY, v123, v122, -1, -1, 0) != 1) {
		v14 = 0;
		v15 = v122;
		if (_vm->_graphicsManager.max_y > v122) {
			v16 = 5;
			do {
				v101 = v16;
				v17 = colision2_ligne(v123, v15, &v136[5], &v141[5], 0, _vm->_objectsManager.DERLIGNE);
				v16 = v101;
				if (v17 == 1 && v141[v101] <= _vm->_objectsManager.DERLIGNE)
					break;
				v136[v101] = 0;
				v141[v101] = -1;
				++v14;
				++v15;
			} while (_vm->_graphicsManager.max_y > v15);
		}
		v131[5] = v14;
		v18 = 0;
		v19 = v122;
		if (_vm->_graphicsManager.min_y < v122) {
			v20 = 1;
			do {
				v102 = v20;
				v21 = colision2_ligne(v123, v19, &v136[1], &v141[1], 0, _vm->_objectsManager.DERLIGNE);
				v20 = v102;
				if (v21 == 1 && v141[v102] <= _vm->_objectsManager.DERLIGNE)
					break;
				v136[v102] = 0;
				v141[v102] = -1;
				if (v131[5] < v18) {
					if (v141[5] != -1)
						break;
				}
				++v18;
				--v19;
			} while (_vm->_graphicsManager.min_y < v19);
		}
		v131[1] = v18;
		v22 = 0;
		v23 = v123;
		if (_vm->_graphicsManager.max_x > v123) {
			v24 = 3;
			do {
				v103 = v24;
				v25 = colision2_ligne(v23, v122, &v136[3], &v141[3], 0, _vm->_objectsManager.DERLIGNE);
				v24 = v103;
				if (v25 == 1 && v141[v103] <= _vm->_objectsManager.DERLIGNE)
					break;
				v136[v103] = 0;
				v141[v103] = -1;
				++v22;
				if (v131[1] < v22) {
					if (v141[1] != -1)
						break;
				}
				if (v131[5] < v22 && v141[5] != -1)
					break;
				++v23;
			} while (_vm->_graphicsManager.max_x > v23);
		}
		v131[3] = v22;
		v26 = 0;
		v27 = v123;
		if (_vm->_graphicsManager.min_x < v123) {
			v28 = 7;
			do {
				v104 = v28;
				v29 = colision2_ligne(v27, v122, &v136[7], &v141[7], 0, _vm->_objectsManager.DERLIGNE);
				v28 = v104;
				if (v29 == 1 && v141[v104] <= _vm->_objectsManager.DERLIGNE)
					break;
				v136[v104] = 0;
				v141[v104] = -1;
				++v26;
				if (v131[1] < v26) {
					if (v141[1] != -1)
						break;
				}
				if (v131[5] < v26 && v141[5] != -1)
					break;
				if (v131[3] < v26 && v141[3] != -1)
					break;
				--v27;
			} while (_vm->_graphicsManager.min_x < v27);
		}
		v131[7] = v26;
		if (v141[1] < 0 || _vm->_objectsManager.DERLIGNE < v141[1])
			v141[1] = -1;
		if (v141[3] < 0 || _vm->_objectsManager.DERLIGNE < v141[3])
			v141[3] = -1;
		if (v141[5] < 0 || _vm->_objectsManager.DERLIGNE < v141[5])
			v141[5] = -1;
		if (v141[7] < 0 || _vm->_objectsManager.DERLIGNE < v141[7])
			v141[7] = -1;
		if (v141[1] < 0)
			v131[1] = 1300;
		if (v141[3] < 0)
			v131[3] = 1300;
		if (v141[5] < 0)
			v131[5] = 1300;
		if (v141[7] < 0)
			v131[7] = 1300;
		if (v141[1] == -1 && v141[3] == -1 && v141[5] == -1 && v141[7] == -1)
			return (int16 *)g_PTRNUL;
		v31 = 0;
		if (v141[5] != -1 && v131[1] >= v131[5] && v131[3] >= v131[5] && v131[7] >= v131[5]) {
			v121 = v141[5];
			v120 = v136[5];
			v31 = 1;
		}
		if (v141[1] != -1 && !v31 && v131[5] >= v131[1] && v131[3] >= v131[1] && v131[7] >= v131[1]) {
			v121 = v141[1];
			v120 = v136[1];
			v31 = 1;
		}
		if (v141[3] != -1 && !v31 && v131[1] >= v131[3] && v131[5] >= v131[3] && v131[7] >= v131[3]) {
			v121 = v141[3];
			v120 = v136[3];
			v31 = 1;
		}
		if (v141[7] != -1 && !v31 && v131[5] >= v131[7] && v131[3] >= v131[7] && v131[1] >= v131[7]) {
			v121 = v141[7];
			v120 = v136[7];
		}
		
		for (v12 = 0; v12 <= 8; ++v12) {
			v141[v12] = -1;
			v136[v12] = 0;
			v131[v12] = 1300;
			v126[v12] = 1300;
		} 

		v34 = 0;
		v35 = srcY;
		if (_vm->_graphicsManager.max_y > srcY) {
			v36 = 5;
			do {
				v105 = v36;
				v37 = colision2_ligne(srcX, v35, &v136[5], &v141[5], 0, _vm->_objectsManager.DERLIGNE);
				v36 = v105;
				if (v37 == 1 && v141[v105] <= _vm->_objectsManager.DERLIGNE)
					break;
				v136[v105] = 0;
				v141[v105] = -1;
				++v34;
				++v35;
			} while (_vm->_graphicsManager.max_y > v35);
		}
		v131[5] = v34 + 1;
		v38 = 0;
		v39 = srcY;
		if (_vm->_graphicsManager.min_y < srcY) {
			v40 = 1;
			do {
				v106 = v40;
				v41 = colision2_ligne(srcX, v39, &v136[1], &v141[1], 0, _vm->_objectsManager.DERLIGNE);
				v40 = v106;
				if (v41 == 1 && v141[v106] <= _vm->_objectsManager.DERLIGNE)
					break;
				v136[v106] = 0;
				v141[v106] = -1;
				++v38;
				if (v141[5] != -1) {
					if (v38 > 80)
						break;
				}
				--v39;
			} while (_vm->_graphicsManager.min_y < v39);
		}
		v131[1] = v38 + 1;
		v42 = 0;
		v43 = srcX;
		if (_vm->_graphicsManager.max_x > srcX) {
			v44 = 3;
			do {
				v107 = v44;
				v45 = colision2_ligne(v43, srcY, &v136[3], &v141[3], 0, _vm->_objectsManager.DERLIGNE);
				v44 = v107;
				if (v45 == 1 && v141[v107] <= _vm->_objectsManager.DERLIGNE)
					break;
				v136[v107] = 0;
				v141[v107] = -1;
				++v42;
				if (v141[5] != -1 || v141[1] != -1) {
					if (v42 > 100)
						break;
				}
				++v43;
			} while (_vm->_graphicsManager.max_x > v43);
		}
		v131[3] = v42 + 1;
		v46 = 0;
		v47 = srcX;
		if (_vm->_graphicsManager.min_x < srcX) {
			v48 = 7;
			do {
				v108 = v48;
				v49 = colision2_ligne(v47, srcY, &v136[7], &v141[7], 0, _vm->_objectsManager.DERLIGNE);
				v48 = v108;
				if (v49 == 1 && v141[v108] <= _vm->_objectsManager.DERLIGNE)
					break;
				v136[v108] = 0;
				v141[v108] = -1;
				++v46;
				if (v141[5] != -1 || v141[1] != -1 || v141[3] != -1) {
					if (v46 > 100)
						break;
				}
				--v47;
			} while (_vm->_graphicsManager.min_x < v47);
		}
		v131[7] = v46 + 1;
		if (v141[1] != -1) {
			v50 = v141[1] - v121;
			if (v50 < 0)
				v50 = -v50;
			v126[1] = v50;
		}
		if (v141[3] != -1) {
			v51 = v141[3] - v121;
			if (v51 < 0)
				v51 = -v51;
			v126[3] = v51;
		}
		if (v141[5] != -1) {
			v52 = v141[5] - v121;
			if (v52 < 0)
				v52 = -v52;
			v126[5] = v52;
		}
		if (v141[7] != -1) {
			v53 = v141[7] - v121;
			if (v53 < 0)
				v53 = -v53;
			v126[7] = v53;
		}
		if (v141[1] == -1 && v141[3] == -1 && v141[5] == -1 && v141[7] == -1)
			error("Nearest point not found error");
		v54 = 0;
		if (v141[1] != -1 && v126[3] >= v126[1] && v126[5] >= v126[1] && v126[7] >= v126[1]) {
			v54 = 1;
			v115 = v141[1];
			v111 = v131[1];
			v113 = 1;
			v114 = v136[1];
		}
		if (!v54) {
			if (v141[5] != -1 && v126[3] >= v126[5] && v126[1] >= v126[5] && v126[7] >= v126[5]) {
				v54 = 1;
				v115 = v141[5];
				v111 = v131[5];
				v113 = 5;
				v114 = v136[5];
			}
			if (!v54) {
				if (v141[3] != -1 && v126[1] >= v126[3] && v126[5] >= v126[3] && v126[7] >= v126[3]) {
					v54 = 1;
					v115 = v141[3];
					v111 = v131[3];
					v113 = 3;
					v114 = v136[3];
				}
				if (!v54 && v141[7] != -1 && v126[1] >= v126[7] && v126[5] >= v126[7] && v126[3] >= v126[7]) {
					v115 = v141[7];
					v111 = v131[7];
					v113 = 7;
					v114 = v136[7];
				}
			}
		}
		v55 = PARC_PERS(srcX, srcY, v123, v122, v115, v121, 0);
		if (v55 != 1) {
			if (v55 == 2) {
LABEL_201:
				v115 = NV_LIGNEDEP;
				v114 = NV_LIGNEOFS;
				v112 = NV_POSI;
			} else {
				if (v113 == 1) {
					v56 = 0;
					if (v111 > 0) {
						do {
							if (colision2_ligne(srcX, srcY - v56, &v125, &v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v58 = GENIAL(v124, v125, srcX, srcY - v56, srcX, srcY - v111, v112, &_vm->_globals.super_parcours[0], 4);
								if (v58 == -1)
									goto LABEL_282;
								v112 = v58;
								if (NVPY != -1)
									v56 = srcY - NVPY;
							}
							v59 = v112;
							_vm->_globals.super_parcours[v59] = srcX;
							_vm->_globals.super_parcours[v59 + 1] = srcY - v56;
							_vm->_globals.super_parcours[v59 + 2] = 1;
							_vm->_globals.super_parcours[v59 + 3] = 0;
							v112 += 4;
							++v56;
						} while (v111 > v56);
					}
				}
				if (v113 == 5) {
					v60 = 0;
					if (v111 > 0) {
						do {
							if (colision2_ligne(srcX, v60 + srcY, &v125, &v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v61 = GENIAL(v124, v125, srcX, v60 + srcY, srcX, v111 + srcY, v112, &_vm->_globals.super_parcours[0], 4);
								if (v61 == -1)
									goto LABEL_282;
								v112 = v61;
								if (NVPY != -1)
									v60 = NVPY - srcY;
							}
							v62 = v112;
							_vm->_globals.super_parcours[v62] = srcX;
							_vm->_globals.super_parcours[v62 + 1] = v60 + srcY;
							_vm->_globals.super_parcours[v62 + 2] = 5;
							_vm->_globals.super_parcours[v62 + 3] = 0;
							v112 += 4;
							++v60;
						} while (v111 > v60);
					}
				}
				if (v113 == 7) {
					v63 = 0;
					if (v111 > 0) {
						do {
							if (colision2_ligne(srcX - v63, srcY, &v125, &v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v64 = GENIAL(v124, v125, srcX - v63, srcY, srcX - v111, srcY, v112, &_vm->_globals.super_parcours[0], 4);
								if (v64 == -1)
									goto LABEL_282;
								v112 = v64;
								if (NVPX != -1)
									v63 = srcX - NVPX;
							}
							v65 = v112;
							_vm->_globals.super_parcours[v65] = srcX - v63;
							_vm->_globals.super_parcours[v65 + 1] = srcY;
							_vm->_globals.super_parcours[v65 + 2] = 7;
							_vm->_globals.super_parcours[v65 + 3] = 0;
							v112 += 4;
							++v63;
						} while (v111 > v63);
					}
				}
				if (v113 == 3) {
					v66 = 0;
					if (v111 > 0) {
						do {
							if (colision2_ligne(v66 + srcX, srcY, &v125, &v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v67 = GENIAL(v124, v125, v66 + srcX, srcY, v111 + srcX, srcY, v112, &_vm->_globals.super_parcours[0], 4);
								if (v67 == -1)
									goto LABEL_282;
								v112 = v67;
								if (NVPX != -1)
									v66 = NVPX - srcX;
							}
							v68 = v112;
							_vm->_globals.super_parcours[v68] = v66 + srcX;
							_vm->_globals.super_parcours[v68 + 1] = srcY;
							_vm->_globals.super_parcours[v68 + 2] = 3;
							_vm->_globals.super_parcours[v68 + 3] = 0;
							v112 += 4;
							++v66;
						} while (v111 > v66);
					}
				}
			}
LABEL_234:
			if (v115 < v121) {
				for (i = v114; Ligne[v115].field0 > i; ++i) {
					v70 = Ligne[v115].lineData;
					v119 = v70[2 * i];
					v118 = v70[2 * i + 1];
					v71 = v112;
					_vm->_globals.super_parcours[v71] = v119;
					_vm->_globals.super_parcours[v71 + 1] = v118;
					_vm->_globals.super_parcours[v71 + 2] = Ligne[v115].field6;
					_vm->_globals.super_parcours[v71 + 3] = 0;
					v112 += 4;
				}
				v116 = v115 + 1;
				if ((v115 + 1) < v121) {
					do {
						v72 = 0;
						v110 = v116;
						for (j = v116; Ligne[j].field0 > v72; j = v116) {
							v74 = Ligne[v110].lineData;
							v119 = v74[2 * v72];
							v118 = v74[2 * v72 + 1];
							v75 = v112;
							_vm->_globals.super_parcours[v75] = v119;
							_vm->_globals.super_parcours[v75 + 1] = v118;
							_vm->_globals.super_parcours[v75 + 2] = Ligne[v110].field6;
							_vm->_globals.super_parcours[v75 + 3] = 0;
							v112 += 4;
							v76 = Ligne[v110].field0;
							if (v76 > 30) {
								v77 = v76 / 2;
								if (v77 < 0)
									v77 = -v77;
								if (v72 == v77) {
									v78 = PARC_PERS(v119, v118, v123, v122, v110, v121, v112);
									if (v78 == 1)
										return &_vm->_globals.super_parcours[0];
									if (v78 == 2)
										goto LABEL_200;
									if (MIRACLE(v119, v118, v110, v121, v112) == 1)
										goto LABEL_201;
								}
							}
							++v72;
							v110 = v116;
						}
						v79 = PARC_PERS(v119, v118, v123, v122, v116, v121, v112);
						if (v79 == 1)
							return &_vm->_globals.super_parcours[0];
						if (v79 == 2) {
LABEL_200:
							v115 = NV_LIGNEDEP;
							v114 = NV_LIGNEOFS;
							v112 = NV_POSI;
							goto LABEL_234;
						}
						if (MIRACLE(v119, v118, v116, v121, v112) == 1)
							goto LABEL_201;
						++v116;
					} while (v116 < v121);
				}
				v114 = 0;
				v115 = v121;
			}
			if (v115 > v121) {
				v80 = v114;
				if (v114 > 0) {
					v98 = v115;
					do {
						v81 = Ligne[v98].lineData;
						v119 = v81[2 * v80];
						v118 = v81[2 * v80 + 1];

						v82 = v112;
						_vm->_globals.super_parcours[v82] = v119;
						_vm->_globals.super_parcours[v82 + 1] = v118;
						_vm->_globals.super_parcours[v82 + 2] = Ligne[v98].field8;
						_vm->_globals.super_parcours[v82 + 3] = 0;
						v112 += 4;
						--v80;
					} while (v80 > 0);
				}
				v117 = v115 - 1;
				if ((v115 - 1) > v121) {
					do {
						v83 = Ligne[v117].field0 - 1;
						if (v83 > -1) {
							v109 = v117;
							do {
								v84 = Ligne[v109].lineData;
								v119 = v84[2 * v83];
								v118 = v84[2 * v83 + 1];
								v85 = v112;
								_vm->_globals.super_parcours[v85] = v119;
								_vm->_globals.super_parcours[v85 + 1] = v118;
								_vm->_globals.super_parcours[v85 + 2] = Ligne[v109].field8;
								_vm->_globals.super_parcours[v85 + 3] = 0;
								v112 += 4;
								v86 = Ligne[v109].field0;
								if (v86 > 30) {
									v87 = v86 / 2;
									if (v87 < 0)
										v87 = -v87;
									if (v83 == v87) {
										v88 = PARC_PERS(v119, v118, v123, v122, v117, v121, v112);
										if (v88 == 1)
											return &_vm->_globals.super_parcours[0];
										if (v88 == 2)
											goto LABEL_200;
										if (MIRACLE(v119, v118, v117, v121, v112) == 1)
											goto LABEL_201;
									}
								}
								--v83;
							} while (v83 > -1);
						}
						v89 = PARC_PERS(v119, v118, v123, v122, v117, v121, v112);
						if (v89 == 1)
							return &_vm->_globals.super_parcours[0];
						if (v89 == 2)
							goto LABEL_200;
						if (MIRACLE(v119, v118, v117, v121, v112) == 1)
							goto LABEL_201;
						--v117;
					} while (v117 > v121);
				}
				v114 = Ligne[v121].field0 - 1;
				v115 = v121;
			}
			if (v115 == v121) {
				if (v114 <= v120) {
					if (v114 < v120) {
						v94 = v114;
						v100 = v121;
						do {
							v95 = Ligne[v100].lineData;
							v96 = v95[2 * v94 + 1];
							v97 = v112;
							_vm->_globals.super_parcours[v97] = v95[2 * v94];
							_vm->_globals.super_parcours[v97 + 1] = v96;
							_vm->_globals.super_parcours[v97 + 2] = Ligne[v100].field6;
							_vm->_globals.super_parcours[v97 + 3] = 0;
							v112 += 4;
							++v94;
						} while (v120 > v94);
					}
				} else {
					v90 = v114;
					v99 = v121;
					do {
						v91 = Ligne[v99].lineData;
						v92 = v91[2 * v90 + 1];
						v93 = v112;
						_vm->_globals.super_parcours[v93] = v91[2 * v90];
						_vm->_globals.super_parcours[v93 + 1] = v92;
						_vm->_globals.super_parcours[v93 + 2] = Ligne[v99].field8;
						_vm->_globals.super_parcours[v93 + 3] = 0;
						v112 += 4;
						--v90;
					} while (v120 < v90);
				}
			}
			v57 = v112;
			if (PARC_PERS(
			            _vm->_globals.super_parcours[v112 - 4],
			            _vm->_globals.super_parcours[v112 - 3],
			            v123,
			            v122,
			            -1,
			            -1,
			            v112) != 1) {
LABEL_282:
				_vm->_globals.super_parcours[v57] = -1;
				_vm->_globals.super_parcours[v57 + 1] = -1;
				_vm->_globals.super_parcours[v57 + 2] = -1;
				_vm->_globals.super_parcours[v57 + 3] = -1;
			}
			return &_vm->_globals.super_parcours[0];
		}
	}
	return &_vm->_globals.super_parcours[0];
}

int LinesManager::PARC_PERS(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
	int16 *v17;
	int v18;
	int v19;
	int v20;
	int v21;
	int v22;
	int v23;
	int v24;
	int v25;
	int16 *v26;
	int v27;
	int16 *v28;
	int16 *v29;
	int v30;
	int v31;
	int16 *v32;
	int v33;
	int v34;
	int16 *v35;
	int v36;
	int v37;
	int16 *v38;
	int v39;
	int v40;
	int v41;
	int16 *v42;
	int v43;
	int v44;
	int v45;
	int v46;
	int16 *v47;
	int v48;
	int16 *v49;
	int16 *v50;
	int v51;
	int v52;
	int16 *v53;
	int v54;
	int v55;
	int v56;
	int16 *v57;
	int v58;
	int v59;
	int16 *v60;
	int v61;
	int v62;
	int v63;
	int16 *v64;
	int v65;
	int v66;
	int v67;
	int v68;
	int16 *v69;
	int v70;
	int16 *v71;
	int16 *v72;
	int v73;
	int v74;
	int v76;
	int16 *v77; 
	int16 *v78; 
	int v79; 
	int16 *v80; 
	int v81; 
	int16 *v82; 
	int v83; 
	int16 *v84; 
	int v85; 
	int16 *v86; 
	int v87; 
	int16 *v88; 
	int v89; 
	int v90; 
	int v91; 
	int v92; 
	int v93; 
	int v94; 
	int v95; 
	int v96; 
	int v97; 
	int v98; 
	int v99; 
	int v100;
	int v101;
	int v102;
	int v103;
	int v104;
	int v105;
	int v106;
	int v107;
	int v108;
	int v109;
	int v110;
	int v111;
	int v112;
	int v113;
	int v114;
	int v115;
	int v116;
	int v117;
	int v118;
	int v119;
	int v120;
	int v121;
	int v122;
	int v123;
	int v124;
	int v125;
	int v126 = 0;
	int v127;
	int v128;
	int v129;
	int v130;
	int v131;
	int v132;
	int v133;
	int v134;
	int v135;
	int v136;
	int v137;
	int v138;
	int v139;
	int v140; 
	int v141; 
	int v142; 
	int v143; 
	int v144; 
	int v145;
	int colResult = 0;

	int v7 = a1;
	v90 = a2;
	v137 = a7;
	v136 = 0;
	if (a5 == -1 && a6 == -1)
		v136 = 1;
	v144 = a5;
	if (colision2_ligne(a1, a2, &v145, &v144, 0, TOTAL_LIGNES) == 1) {
		int v8 = Ligne[v144].field4;
		if (v8 == 1)
			v90 = a2 - 2;
		if (v8 == 2) {
			v90 -= 2;
			v7 = a1 + 2;
		}
		if (v8 == 3)
			v7 += 2;
		if (v8 == 4) {
			v90 += 2;
			v7 += 2;
		}
		if (v8 == 5)
			v90 += 2;
		if (v8 == 6) {
			v90 += 2;
			v7 -= 2;
		}
		if (v8 == 7)
			v7 -= 2;
		if (v8 == 8) {
			v90 -= 2;
			v7 -= 2;
		}
	}
	v98 = v7;
	v97 = v90;
	v115 = 0;
	v142 = -1;
	v140 = -1;
	v138 = -1;

	int v9, v10, v11, v12, v13, v14;
	for (;;) {
		v111 = v7;
		v109 = v90;
		if (a3 >= v7 - 2 && a3 <= v7 + 2 && a4 >= v90 - 2 && a4 <= v90 + 2) {
LABEL_149:
			v27 = v115;
			v28 = _vm->_globals.essai0;
			v28[v27] = -1;
			v28[v27 + 1] = -1;
			v28[v27 + 2] = -1;

LABEL_150:
			if (v115) {
				v127 = 0;
				v116 = 0;
				v29 = _vm->_globals.essai0;
				do {
					v30 = v137;
					_vm->_globals.super_parcours[v30] = v29[v116];
					_vm->_globals.super_parcours[v30 + 1] = v29[v116 + 1];
					_vm->_globals.super_parcours[v30 + 2] = v29[v116 + 2];
					_vm->_globals.super_parcours[v30 + 3] = 0;
					v116 += 3;
					v137 += 4;

					if (v29[v116] == -1) {
						if (v29[v116 + 1] == -1)
							v127 = 1;
					}
				} while (v127 != 1);
			}
			goto LABEL_248;
		}
		v9 = v7 - a3;
		if (v7 - a3 < 0)
			v9 = -v9;
		v10 = v9 + 1;
		v11 = v90 - a4;
		if (v90 - a4 < 0)
			v11 = -v11;
		v107 = v11 + 1;
		if (v10 > (int16)(v11 + 1))
			v107 = v10;
		v12 = v107 - 1;
		v101 = 1000 * v10 / v12;
		v99 = 1000 * (int16)(v11 + 1) / v12;
		if (a3 < v7)
			v101 = -v101;
		if (a4 < v90)
			v99 = -v99;
		v13 = (int16)v101 / 1000;
		v94 = (int16)v99 / 1000;
		v91 = -1;
		if ((int16)v99 / 1000 == -1 && (unsigned int)v101 <= 0x96)
			v91 = 1;
		if (v13 == 1) {
			if ((unsigned int)(v99 + 1) <= 0x97)
				v91 = 3;
			if ((unsigned int)v99 <= 0x96)
				v91 = 3;
		}
		if (v94 == 1) {
			if ((unsigned int)v101 <= 0x96)
				v91 = 5;
			if ((unsigned int)(v101 + 150) <= 0x96)
				v91 = 5;
		}
		if (v13 == -1) {
			if ((unsigned int)v99 <= 0x96)
				v91 = 7;
			if ((unsigned int)(v99 + 150) <= 0x96)
				v91 = 7;
		}
		if (v94 == -1 && (unsigned int)(v101 + 150) <= 0x96)
			v91 = 1;
		if (v91 == -1 && !VERIF_SMOOTH(v7, v109, a3, a4) && SMOOTH_MOVE(v7, v109, a3, a4) != -1)
			break;
LABEL_72:
		v19 = v111 - a3;
		if (v111 - a3 < 0)
			v19 = -v19;
		v20 = v19 + 1;
		v95 = v109 - a4;
		if (v95 < 0)
			v95 = -(v109 - a4);
		v108 = v95 + 1;
		if (v20 > (int16)(v95 + 1))
			v108 = v20;
		if (v108 <= 10)
			goto LABEL_149;
		v21 = v108 - 1;
		v102 = 1000 * v20 / v21;
		v100 = 1000 * (int16)(v95 + 1) / v21;
		if (a3 < v111)
			v102 = -v102;
		if (a4 < v109)
			v100 = -v100;
		v22 = (int16)v102 / 1000;
		v96 = (int16)v100 / 1000;
		v106 = 1000 * v111;
		v105 = 1000 * v109;
		v104 = 1000 * v111 / 1000;
		v103 = v105 / 1000;
		if (!((int16)v102 / 1000) && v96 == -1)
			v91 = 1;
		if (v22 == 1) {
			if (v96 == -1)
				v91 = 2;
			if (!v96)
				v91 = 3;
			if (v96 == 1)
				v91 = 4;
		}
		if (!v22 && v96 == 1)
			v91 = 5;
		if (v22 != -1)
			goto LABEL_103;
		if (v96 == 1)
			v91 = 6;
		if (!v96)
			v91 = 7;
		if (v96 == -1) {
			v91 = 8;
LABEL_103:
			if (v96 == -1) {
				if ((unsigned int)v102 <= 0x1FE)
					v91 = 1;
				if ((unsigned int)(v102 - 510) <= 0x1EA)
					v91 = 2;
			}
		}
		if (v22 == 1) {
			if ((unsigned int)(v100 + 1) <= 0x1FF)
				v91 = 2;
			if ((unsigned int)(v100 + 510) <= 0x1FE)
				v91 = 3;
			if ((unsigned int)v100 <= 0x1FE)
				v91 = 3;
			if ((unsigned int)(v100 - 510) <= 0x1EA)
				v91 = 4;
		}
		if (v96 == 1) {
			if ((unsigned int)(v102 - 510) <= 0x1EA)
				v91 = 4;
			if ((unsigned int)v102 <= 0x1FE)
				v91 = 5;
			// CHECKME: The two conditions on v102 are not compatible!
			if (v102 >= -1 && v102 <= -510)
				v91 = 6;
			if ((unsigned int)(v102 + 510) <= 0x1FE)
				v91 = 5;
		}
		if (v22 == -1) {
			if ((unsigned int)(v100 - 510) <= 0x1EA)
				v91 = 6;
			if ((unsigned int)v100 <= 0x1FE)
				v91 = 7;
			if ((unsigned int)(v100 + 1000) <= 0x1EA)
				v91 = 8;
			if ((unsigned int)(v100 + 510) <= 0x1FE)
				v91 = 7;
		}
		if (v96 == -1) {
			if ((unsigned int)(v102 + 1000) <= 0x1EA)
				v91 = 8;
			if ((unsigned int)(v102 + 510) <= 0x1FE)
				v91 = 1;
		}
		v23 = 0;
		if (v108 + 1 <= 0)
			goto LABEL_149;
		while (colision2_ligne(v104, v103, &v143, &v142, 0, TOTAL_LIGNES) != 1) {
			v25 = v115;
			v26 = _vm->_globals.essai0;
			v26[v25] = v104;
			v26[v25 + 1] = v103;
			v26[v25 + 2] = v91;
			v106 += v102;
			v105 += v100;
			v104 = v106 / 1000;
			v103 = v105 / 1000;
			v115 += 3;
			++v23;
			if (v23 >= v108 + 1)
				goto LABEL_149;
		}
		if (_vm->_objectsManager.DERLIGNE >= v142)
			goto LABEL_157;
		v24 = GENIAL(v142, v143, v104, v103, a3, a4, v115, _vm->_globals.essai0, 3);
		if (v24 == -1)
			goto LABEL_150;
		v115 = v24;
		if (NVPX != -1 || NVPY != -1) {
LABEL_67:
			v142 = -1;
			goto LABEL_157;
		}
		v7 = -1;
		v90 = -1;
	}
	v91 = SMOOTH_SENS;
	v14 = 0;
	int v16;
	for (;;) {
		int v15 = SMOOTH[v14].field0;
		v112 = v15;
		v110 = SMOOTH[v14].field2;
		if (v15 == -1 || SMOOTH[v14].field2 == -1) {
			v126 = 1;
			goto LABEL_70;
		}
		if (colision2_ligne(v15, v110, &v143, &v142, 0, TOTAL_LIGNES) == 1)
			break;
		v16 = v115;

		v17 = _vm->_globals.essai0;
		v17[v16] = v112;
		v17[v16 + 1] = v110;
		v17[v16 + 2] = v91;
		v115 += 3;
		++v14;
LABEL_70:
		if (v126 == 1) {
			v18 = v14 - 1;
			v111 = SMOOTH[v18].field0;
			v109 = SMOOTH[v18].field2;
			goto LABEL_72;
		}
	}
	if (v142 > _vm->_objectsManager.DERLIGNE)
		goto LABEL_67;
LABEL_157:
	v31 = v115;
	v32 = _vm->_globals.essai0;
	v32[v31] = -1;
	v32[v31 + 1] = -1;
	v32[v31 + 2] = -1;
	
	v117 = 0;
	v33 = v98;
	v92 = v97;
LABEL_158:
	v113 = v33;
	if (a3 >= v33 - 2 && a3 <= v33 + 2 && a4 >= v92 - 2 && a4 <= v92 + 2)
		goto LABEL_194;
	if (v33 >= a3) {
LABEL_165:
		if (v113 > a3) {
			v36 = v113;
			while (colision2_ligne(v36, v92, &v141, &v140, 0, TOTAL_LIGNES) != 1) {
				v37 = v117;
				v38 = _vm->_globals.essai1;
				v38[v37] = v36;
				v38[v37 + 1] = v92;
				v38[v37 + 2] = 7;
				v117 += 3;
				--v36;
				if (a3 >= v36)
					goto LABEL_171;
			}
			goto LABEL_168;
		}
LABEL_171:
		if (v92 >= a4) {
LABEL_181:
			if (v92 > a4) {
				v43 = v92;
				do {
					if (colision2_ligne(a3, v43, &v141, &v140, 0, TOTAL_LIGNES) == 1) {
						if (_vm->_objectsManager.DERLIGNE < v140) {
							v44 = GENIAL(v140, v141, a3, v43, a3, a4, v117, _vm->_globals.essai1, 3);
							if (v44 == -1)
								goto LABEL_195;
							v117 = v44;
							if (NVPX != -1) {
								if (NVPY != -1) {
									v33 = NVPX;
									v92 = NVPY;
									v45 = colision2_ligne(NVPX, NVPY, &v141, &v140, 0, _vm->_objectsManager.DERLIGNE);
									goto LABEL_189;
								}
							}
						}
						if (v140 <= _vm->_objectsManager.DERLIGNE)
							goto LABEL_202;
					}
					v46 = v117;
					v47 = _vm->_globals.essai1;
					v47[v46] = a3;
					v47[v46 + 1] = v43;
					v47[v46 + 2] = 1;
					v117 += 3;
					--v43;
				} while (a4 < v43);
			}
LABEL_194:
			v48 = v117;
			v49 = _vm->_globals.essai1;
			v49[v48] = -1;
			v49[v48 + 1] = -1;
			v49[v48 + 2] = -1;
LABEL_195:
			if (v117) {
				v128 = 0;
				v118 = 0;
				v50 = _vm->_globals.essai1;
				do {
					v51 = v137;
					_vm->_globals.super_parcours[v51] = v50[v118];
					_vm->_globals.super_parcours[v51 + 1] = v50[v118 + 1];
					_vm->_globals.super_parcours[v51 + 2] = v50[v118 + 2];
					_vm->_globals.super_parcours[v51 + 3] = 0;
					v118 += 3;
					v137 += 4;
					if (v50[v118] == -1 && v50[v118 + 1] == -1)
						v128 = 1;
				} while (v128 != 1);
			}
			goto LABEL_248;
		}
		v39 = v92;
		for (;;) {
			if (colision2_ligne(a3, v39, &v141, &v140, 0, TOTAL_LIGNES) == 1) {
				if (_vm->_objectsManager.DERLIGNE < v140) {
					v40 = GENIAL(v140, v141, a3, v39, a3, a4, v117, _vm->_globals.essai1, 3);
					if (v40 == -1)
						goto LABEL_195;
					v117 = v40;
					if (NVPX != -1) {
						if (NVPY != -1) {
							v33 = NVPX;
							v92 = NVPY;
							v45 = colision2_ligne(NVPX, NVPY, &v141, &v140, 0, _vm->_objectsManager.DERLIGNE);
LABEL_189:
							if (v45 == 1 && v140 <= _vm->_objectsManager.DERLIGNE)
								goto LABEL_202;
							goto LABEL_158;
						}
					}
				}
				if (v140 <= _vm->_objectsManager.DERLIGNE)
					goto LABEL_202;
			}
			v41 = v117;

			v42 = _vm->_globals.essai1;
			v42[v41] = a3;
			v42[v41 + 1] = v39;
			v42[v41 + 2] = 5;
			v117 += 3;
			++v39;
			if (a4 <= v39)
				goto LABEL_181;
		}
	}
	while (colision2_ligne(v33, v92, &v141, &v140, 0, TOTAL_LIGNES) != 1) {
		v34 = v117;
		v35 = _vm->_globals.essai1;
		v35[v34] = v33;
		v35[v34 + 1] = v92;
		v35[v34 + 2] = 3;
		v117 += 3;
		++v33;
		if (a3 <= v33)
			goto LABEL_165;
	}
LABEL_168:
	if (v140 > _vm->_objectsManager.DERLIGNE)
		v140 = -1;
LABEL_202:
	v52 = v117;
	v53 = _vm->_globals.essai1;
	v53[v52] = -1;
	v53[v52 + 1] = -1;
	v53[v52 + 2] = -1;
	v117 = 0;
	v54 = v98;
	v93 = v97;
LABEL_203:
	v114 = v54;
	if (a3 >= v54 - 2 && a3 <= v54 + 2 && a4 >= v93 - 2 && a4 <= v93 + 2)
		goto LABEL_241;
	if (v93 < a4) {
		v55 = v93;
		while (colision2_ligne(v114, v55, &v139, &v138, 0, TOTAL_LIGNES) != 1) {
			v56 = v117;
			v57 = _vm->_globals.essai2;
			v57[v56] = v114;
			v57[v56 + 1] = v55;
			v57[v56 + 2] = 5;
			v117 += 3;
			++v55;
			if (a4 <= v55)
				goto LABEL_211;
		}
		goto LABEL_214;
	}
LABEL_211:
	if (v93 > a4) {
		v58 = v93;
		while (colision2_ligne(v114, v58, &v139, &v138, 0, TOTAL_LIGNES) != 1) {
			v59 = v117;
			v60 = _vm->_globals.essai2;
			v60[v59] = v114;
			v60[v59 + 1] = v58;
			v60[v59 + 2] = 1;
			v117 += 3;
			--v58;
			if (a4 >= v58)
				goto LABEL_217;
		}
LABEL_214:
		if (v138 > _vm->_objectsManager.DERLIGNE)
			v138 = -1;
LABEL_249:
		v76 = v117;
		v77 = _vm->_globals.essai2;
		v77[v76] = -1;
		v77[v76 + 1] = -1;
		v77[v76 + 2] = -1;

		if (v136 != 1) {
			if (a6 > v144) {
				if (_vm->_globals.essai0[0] != -1 && v142 > v144 && v140 <= v142 && v138 <= v142 && a6 >= v142) {
					NV_LIGNEDEP = v142;
					NV_LIGNEOFS = v143;
					v130 = 0;
					v120 = 0;
					v78 = _vm->_globals.essai0;
					do {
						v79 = v137;
						_vm->_globals.super_parcours[v79] = v78[v120];
						_vm->_globals.super_parcours[v79 + 1] = v78[v120 + 1];
						_vm->_globals.super_parcours[v79 + 2] = v78[v120 + 2];
						_vm->_globals.super_parcours[v79 + 3] = 0;
						v120 += 3;
						v137 += 4;
						if (v78[v120] == -1 && v78[v120 + 1] == -1)
							v130 = 1;
					} while (v130 != 1);
LABEL_323:
					NV_POSI = v137;
					return 2;
				}
				v80 = _vm->_globals.essai1;
				if (_vm->_globals.essai1[0] != -1 && v144 < v140 && v138 <= v140 && v142 <= v140 && a6 >= v140) {
					NV_LIGNEDEP = v140;
					NV_LIGNEOFS = v141;
					v131 = 0;
					v121 = 0;
					do {
						assert(137 <= 32000);
						v81 = v137;
						_vm->_globals.super_parcours[v81] = v80[v121];
						_vm->_globals.super_parcours[v81 + 1] = v80[v121 + 1];
						_vm->_globals.super_parcours[v81 + 2] = v80[v121 + 2];
						_vm->_globals.super_parcours[v81 + 3] = 0;
						v121 += 3;
						v137 += 4;
						if (v80[v121] == -1 && v80[v121 + 1] == -1)
							v131 = 1;
					} while (v131 != 1);
LABEL_301:
					NV_POSI = v137;
					return 2;
				}
				v82 = _vm->_globals.essai2;
				if (_vm->_globals.essai2[0] != -1) {
					if (v144 < v138 && v140 < v138 && v142 < v138 && a6 >= v138) {
						NV_LIGNEDEP = v138;
						NV_LIGNEOFS = v139;
						v132 = 0;
						v122 = 0;
						do {
							assert(v137 <= 32000);
							v83 = v137;
							_vm->_globals.super_parcours[v83] = v82[v122];
							_vm->_globals.super_parcours[v83 + 1] = v82[v122 + 1];
							_vm->_globals.super_parcours[v83 + 2] = v82[v122 + 2];
							_vm->_globals.super_parcours[v83 + 3] = 0;
							v122 += 3;
							v137 += 4;
							if (v82[v122] == -1 && v82[v122 + 1] == -1)
								v132 = 1;
						} while (v132 != 1);
LABEL_312:
						NV_POSI = v137;
						return 2;
					}
				}
			}
			if (a6 < v144) {
				if (v142 == -1)
					v142 = 1300;
				if (v140 == -1)
					v142 = 1300;
				if (v138 == -1)
					v142 = 1300;
				if (_vm->_globals.essai1[0] != -1 && v140 < v144 && v138 >= v140 && v142 >= v140 && a6 <= v140) {
					NV_LIGNEDEP = v140;
					NV_LIGNEOFS = v141;
					v133 = 0;
					v123 = 0;
					v84 = _vm->_globals.essai1;
					do {
						assert(137 <= 32000);
						v85 = v137;
						_vm->_globals.super_parcours[v85] = v84[v123];
						_vm->_globals.super_parcours[v85 + 1] = v84[v123 + 1];
						_vm->_globals.super_parcours[v85 + 2] = v84[v123 + 2];
						_vm->_globals.super_parcours[v85 + 3] = 0;
						v123 += 3;
						v137 += 4;
						if (v84[v123] == -1 && v84[v123 + 1] == -1)
							v133 = 1;
					} while (v133 != 1);
					goto LABEL_301;
				}
				v86 = _vm->_globals.essai2;
				if (_vm->_globals.essai2[0] != -1 && v144 > v138 && v140 >= v138 && v142 >= v138 && a6 <= v138) {
					NV_LIGNEDEP = v138;
					NV_LIGNEOFS = v139;
					v134 = 0;
					v124 = 0;
					do {
						assert(137 <= 32000);
						v87 = v137;
						_vm->_globals.super_parcours[v87] = v86[v124];
						_vm->_globals.super_parcours[v87 + 1] = v86[v124 + 1];
						_vm->_globals.super_parcours[v87 + 2] = v86[v124 + 2];
						_vm->_globals.super_parcours[v87 + 3] = 0;
						v124 += 3;
						v137 += 4;
						if (v86[v124] == -1 && v86[v124 + 1] == -1)
							v134 = 1;
					} while (v134 != 1);
					goto LABEL_312;
				}
				if (_vm->_globals.essai1[0] != -1 && v144 > v142 && v140 >= v142 && v138 >= v142 && a6 <= v142) {
					NV_LIGNEDEP = v142;
					NV_LIGNEOFS = v143;
					v135 = 0;
					v125 = 0;
					v88 = _vm->_globals.essai0;
					do {
						assert(137 <= 32000);
						v89 = v137;
						_vm->_globals.super_parcours[v89] = v88[v125];
						_vm->_globals.super_parcours[v89 + 1] = v88[v125 + 1];
						_vm->_globals.super_parcours[v89 + 2] = v88[v125 + 2];
						_vm->_globals.super_parcours[v89 + 3] = 0;
						v125 += 3;
						v137 += 4;
						if (v88[v125] == -1 && v88[v125 + 1] == -1)
							v135 = 1;
					} while (v135 != 1);
					goto LABEL_323;
				}
			}
		}
		return 0;
	}
LABEL_217:
	if (v114 < a3) {
		v61 = v114;
		do {
			if (colision2_ligne(v61, a4, &v139, &v138, 0, TOTAL_LIGNES) == 1) {
				if (_vm->_objectsManager.DERLIGNE < v138) {
					v62 = GENIAL(v138, v139, v61, a4, a3, a4, v117, _vm->_globals.essai2, 3);
					if (v62 == -1)
						goto LABEL_195;
					v117 = v62;
					if (NVPX != -1) {
						if (NVPY != -1) {
							v54 = NVPX;
							v93 = NVPY;
							colResult = colision2_ligne(NVPX, NVPY, &v139, &v138, 0, _vm->_objectsManager.DERLIGNE);
LABEL_235:
							if (colResult == 1 && v138 <= _vm->_objectsManager.DERLIGNE)
								goto LABEL_249;
							goto LABEL_203;
						}
					}
				}
				if (v138 <= _vm->_objectsManager.DERLIGNE)
					goto LABEL_249;
			}
			v63 = v117;

			v64 = _vm->_globals.essai2;
			v64[v63] = v61;
			v64[v63 + 1] = a4;
			v64[v63 + 2] = 3;
			v117 += 3;
			++v61;
		} while (a3 > v61);
	}
	if (v114 > a3) {
		v65 = v114;
		do {
			if (colision2_ligne(v65, a4, &v139, &v138, 0, TOTAL_LIGNES) == 1) {
				if (_vm->_objectsManager.DERLIGNE < v138) {
					v66 = GENIAL(v138, v139, v65, a4, a3, a4, v117, _vm->_globals.essai2, 3);
					if (v66 == -1)
						goto LABEL_242;
					v117 = v66;
					if (NVPX != -1) {
						if (NVPY != -1) {
							v54 = NVPX;
							v93 = NVPY;
							v67 = colision2_ligne(NVPX, NVPY, &v139, &v138, 0, _vm->_objectsManager.DERLIGNE);
							goto LABEL_235;
						}
					}
				}
				if (v138 <= _vm->_objectsManager.DERLIGNE)
					goto LABEL_249;
			}
			v68 = v117;
			v69 = _vm->_globals.essai2;
			v69[v68] = v65;
			v69[v68 + 1] = a4;
			v69[v68 + 2] = 7;
			v117 += 3;
			--v65;
		} while (a3 < v65);
	}
	v138 = -1;
LABEL_241:
	v70 = v117;
	v71 = _vm->_globals.essai2;
	v71[v70] = -1;
	v71[v70 + 1] = -1;
	v71[v70 + 2] = -1;
LABEL_242:
	if (v117) {
		v129 = 0;
		v119 = 0;
		v72 = _vm->_globals.essai2;
		do {
			v73 = v137;
			_vm->_globals.super_parcours[v73] = v72[v119];
			_vm->_globals.super_parcours[v73 + 1] = v72[v119 + 1];
			_vm->_globals.super_parcours[v73 + 2] = v72[v119 + 2];
			_vm->_globals.super_parcours[v73 + 3] = 0;
			v119 += 3;
			v137 += 4;
			if (v72[v119] == -1 && v72[v119 + 1] == -1)
				v129 = 1;
		} while (v129 != 1);
	}
LABEL_248:
	v74 = v137;
	_vm->_globals.super_parcours[v74] = -1;
	_vm->_globals.super_parcours[v74 + 1] = -1;
	_vm->_globals.super_parcours[v74 + 2] = -1;
	_vm->_globals.super_parcours[v74 + 3] = -1;
	return 1;
}

int LinesManager::VERIF_SMOOTH(int a1, int a2, int a3, int a4) {
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v11; 
	int v12; 
	int v13; 
	int v14; 
	int v15;
	int v16;
	int v17; 
	int v18; 

	v4 = a1 - a3;
	if (a1 - a3 < 0)
		v4 = -v4;
	v5 = v4 + 1;
	v11 = a2 - a4;
	if (a2 - a4 < 0)
		v11 = -(a2 - a4);
	v13 = v11 + 1;
	if (v5 > (int)(v11 + 1))
		v13 = v5;
	if (v13 <= 10)
		return -1;
	v6 = v13 - 1;
	v16 = 1000 * v5 / v6;
	v15 = 1000 * (int)(v11 + 1) / v6;
	if (a3 < a1)
		v16 = -v16;
	if (a4 < a2)
		v15 = -v15;
	v7 = 1000 * a1;
	v8 = 1000 * a2;
	v9 = 1000 * a1 / 1000;
	v12 = 1000 * a2 / 1000;
	v14 = 0;
	if (v13 + 1 > 0) {
		while (colision2_ligne(v9, v12, &v18, &v17, 0, TOTAL_LIGNES) != 1 || v17 > _vm->_objectsManager.DERLIGNE) {
			v7 += v16;
			v8 += v15;
			v9 = v7 / 1000;
			v12 = v8 / 1000;
			++v14;
			if (v14 >= v13 + 1)
				return 0;
		}
		return -1;
	}
	return 0;
}

int LinesManager::SMOOTH_MOVE(int a3, int a4, int a5, int a6) {
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10;
	int v11;
	int v12;
	int v13; 
	int v14; 
	int v15; 
	int v16;
	int v17;
	int v18;
	int v19;
	int v20;
	int v21;
	int v22; 
	int v23; 
	int v24; 
	int v25; 
	int v26; 
	int v27;
	int v28;
	int v29;
	int v30;
	int v31;
	int v32;
	int v33;
	int v34;
	int v35; 
	int v37; 
	int v38; 
	int v39; 
	int v40; 
	int v41; 
	int v42; 
	int v43; 
	int v44; 
	int v45; 
	int v46; 
	int v47; 
	int v48; 
	int v49 = 0; 
	int v50; 
	int v51; 
	int v52; 
	int v53; 
	int v54; 
	int v55; 
	int v56; 
	int v57; 
	int v58; 
	int v59; 
	int v60; 
	int v61; 
	int v62; 
	int v63;

	v62 = a3;
	v63 = a4;
	if (a3 >= a5 || a6 >= a4) {
		if (a3 <= a5 || a6 >= a4) {
			if (a3 >= a5 || a6 <= a4) {
				if (a3 > a5 && a6 > a4) {
					v53 = 36;
					v61 = 0;
					v57 = 0;
					do {
						v25 = _vm->_globals.Hopkins[v53].field0;
						v40 = _vm->_globals.Hopkins[v53].field2;
						v26 = _vm->_globals.STAILLE[v63];
						if (v26 < 0) {
							v27 = v26;
							v28 = v26;
							if (v26 < 0)
								v28 = -v26;
							v48 = _vm->_globals.STAILLE[v63];
							v45 = v63;
							v25 = _vm->_graphicsManager.Reel_Reduc(v25, v28);
							v29 = v27;
							if ((v27 & 0x80000000u) != 0)
								v29 = -v27;
							v40 = _vm->_graphicsManager.Reel_Reduc(v40, v29);
							v63 = v45;
							v26 = v48;
						}
						if (v26 > 0) {
							v30 = v26;
							v31 = v26;
							if (v26 < 0)
								v31 = -v26;
							v46 = v63;
							v25 = _vm->_graphicsManager.Reel_Zoom(v25, v31);
							v32 = v30;
							if ((v30 & 0x80000000u) != 0)
								v32 = -v30;
							v40 = _vm->_graphicsManager.Reel_Zoom(v40, v32);
							v63 = v46;
						}
						v33 = v63 + v40;
						v34 = 0;
						if (v25 > 0) {
							do {
								--v62;
								SMOOTH[v61].field0 = v62;
								if (v63 != v33)
									v63 = v63 + 1;
								SMOOTH[v61++].field2 = v63;
								++v34;
							} while (v34 < v25);
						}
						++v53;
						if (v53 == 48)
							v53 = 36;
						++v57;
						if (v62 <= a5 || a6 <= v63)
							v49 = 1;
					} while (v49 != 1);
					if (v57 > 5) {
						v35 = v61;
						SMOOTH[v35].field0 = -1;
						SMOOTH[v35].field2 = -1;
						_vm->_linesManager.SMOOTH_SENS = 6;
						goto LABEL_85;
					}
				}
			} else {
				v52 = 36;
				v60 = 0;
				v56 = 0;
				do {
					v14 = _vm->_globals.Hopkins[v52].field0;
					v39 = _vm->_globals.Hopkins[v52].field2;
					v15 = _vm->_globals.STAILLE[v63];
					if (v15 < 0) {
						v16 = v15;
						v17 = v15;
						if (v15 < 0)
							v17 = -v15;
						v47 = _vm->_globals.STAILLE[v63];
						v43 = v63;
						v14 = _vm->_graphicsManager.Reel_Reduc(v14, v17);
						v18 = v16;
						if ((v16 & 0x80000000u) != 0)
							v18 = -v16;
						v39 = _vm->_graphicsManager.Reel_Reduc(v39, v18);
						v63 = v43;
						v15 = v47;
					}
					if (v15 > 0) {
						v19 = v15;
						v20 = v15;
						if (v15 < 0)
							v20 = -v15;
						v44 = v63;
						v14 = _vm->_graphicsManager.Reel_Zoom(v14, v20);
						v21 = v19;
						if ((v19 & 0x80000000u) != 0)
							v21 = -v19;
						v39 = _vm->_graphicsManager.Reel_Zoom(v39, v21);
						v63 = v44;
					}
					v22 = v63 + v39;
					v23 = 0;
					if (v14 > 0) {
						do {
							++v62;
							SMOOTH[v60].field0 = v62;
							if (v63 != v22)
								v63 = v63 + 1;
							SMOOTH[v60++].field2 = v63;
							++v23;
						} while (v23 < v14);
					}
					++v52;
					if (v52 == 48)
						v52 = 36;
					++v56;
					if (v62 >= a5 || a6 <= (int)v63)
						v49 = 1;
				} while (v49 != 1);
				if (v56 > 5) {
					v24 = v60;
					SMOOTH[v24].field0 = -1;
					SMOOTH[v24].field2 = -1;
					_vm->_linesManager.SMOOTH_SENS = 4;
					goto LABEL_85;
				}
			}
		} else {
			v51 = 12;
			v59 = 0;
			v55 = 0;
			do {
				v10 = _vm->_globals.Hopkins[v51].field2;
				v42 = v63;
				v11 = _vm->_graphicsManager.Reel_Reduc(_vm->_globals.Hopkins[v51].field0, 0x19u);
				v38 = _vm->_graphicsManager.Reel_Reduc(v10, 0x19u);
				v63 = v42;
				v12 = 0;
				if (v11 > 0) {
					do {
						--v62;
						SMOOTH[v59].field0 = v62;
						if ((uint16)v63 != (uint16)v42 + v38)
							v63 = v63 - 1;
						SMOOTH[v59++].field2 = v63;
						++v12;
					} while (v12 < v11);
				}
				++v51;
				if (v51 == 24)
					v51 = 12;
				++v55;
				if (v62 <= a5 || a6 >= (int)v63)
					v49 = 1;
			} while (v49 != 1);
			if (v55 > 5) {
				v13 = 4 * v59;
				SMOOTH[v13].field0 = -1;
				SMOOTH[v13].field2 = -1;
				_vm->_linesManager.SMOOTH_SENS = 8;
				goto LABEL_85;
			}
		}
	} else {
		v50 = 12;
		v58 = 0;
		v54 = 0;
		do {
			v6 = _vm->_globals.Hopkins[v50].field2;
			v41 = v63;
			v7 = _vm->_graphicsManager.Reel_Reduc(_vm->_globals.Hopkins[v50].field0, 0x19u);
			v37 = _vm->_graphicsManager.Reel_Reduc(v6, 0x19u);
			v63 = v41;
			v8 = 0;
			if (v7 > 0) {
				do {
					++v62;
					SMOOTH[v58].field0 = v62;
					if ((uint16)v63 != (uint16)v41 + v37)
						v63 = v63 - 1;
					SMOOTH[v58++].field2 = v63;
					++v8;
				} while (v8 < v7);
			}
			++v50;
			if (v50 == 24)
				v50 = 12;
			++v54;
			if (v62 >= a5 || a6 >= (int)v63)
				v49 = 1;
		} while (v49 != 1);
		if (v54 > 5) {
			v9 = v58;
			SMOOTH[v9].field0 = -1;
			SMOOTH[v9].field2 = -1;
			_vm->_linesManager.SMOOTH_SENS = 2;
LABEL_85:
			SMOOTH_X = v62;
			SMOOTH_Y = v63;
			return 0;
		}
	}
	return -1;
}

int LinesManager::PLAN_TEST(int a1, int a2, int a3, int a4, int a5, int a6) {
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10; 
	int v11; 
	int v12; 
	int v13; 
	int v14; 
	int16 *v15; 
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	int16 *v20; 
	int v21; 
	int v22; 
	int v23; 
	int16 *v25; 
	int16 *v26; 
	int v27; 
	int v28; 
	int v29; 
	int v30; 
	int v31; 
	int v32; 
	int v33 = 0; 
	int v34 = 0; 
	int v35; 
	int v36; 
	int v37; 
	int v38; 
	int v39; 
	int v40; 
	int v41; 
	int v42; 
	int v43; 
	int v44; 
	int v45; 
	int v46; 
	int v47; 
	int v48; 
	int v49; 
	int v50; 
	int v51; 
	int v52; 
	int v53; 

	v41 = a3;
	v40 = TEST_LIGNE(a1, a2 - 2, &v42, &v50, &v46);
	v39 = TEST_LIGNE(a1, a2 + 2, &v43, &v51, &v47);
	v38 = TEST_LIGNE(a1 - 2, a2, &v44, &v52, &v48);
	v6 = TEST_LIGNE(a1 + 2, a2, &v45, &v53, &v49);
	v37 = v6;
	if (v40 == -1 && v39 == -1 && v38 == -1 && v6 == -1)
		return -1;
	if (a4 == -1 || a5 == -1) {
		v8 = 0;
		if (v40 != -1)
			v8 = 1;
		if (v8)
			goto LABEL_60;
		if (v39 != -1)
			v8 = 2;
		if (v8)
			goto LABEL_60;
		if (v38 != -1)
			v8 = 3;
		if (v8)
			goto LABEL_60;
		if (v6 != -1)
			v8 = 4;
		goto LABEL_59;
	}
	v28 = 100;
	v7 = 100;
	v35 = 100;
	v27 = 100;
	v8 = 0;
	v9 = a4 - a5;
	if (a4 - a5 < 0)
		v9 = -v9;
	v36 = v9;
	if (v40 != -1) {
		v10 = v50 - a5;
		if (v50 - a5 < 0)
			v10 = -v10;
		v28 = v10;
	}
	if (v39 != -1) {
		v11 = v51 - a5;
		if (v11 < 0)
			v11 = -v11;
		v7 = v11;
	}
	if (v38 != -1) {
		v12 = v52 - a5;
		if (v12 < 0)
			v12 = -v12;
		v35 = v12;
	}
	if (v37 != -1) {
		v13 = v53 - a5;
		if (v13 < 0)
			v13 = -v13;
		v27 = v13;
	}
	if (v28 < v36 && v28 <= v7 && v28 <= v35 && v28 <= v27)
		v8 = 1;
	if (v8)
		goto LABEL_60;
	if (v36 > v7 && v28 >= v7 && v35 >= v7 && v27 >= v7)
		v8 = 2;
	if (v8)
		goto LABEL_60;
	if (v35 < v36 && v35 <= v28 && v35 <= v7 && v35 <= v27)
		v8 = 3;
	if (v8)
		goto LABEL_60;
	if (v27 >= v36 || v27 > v28 || v27 > v7 || v27 > v35) {
LABEL_59:
		if (v8)
			goto LABEL_60;
		return -1;
	}
	v8 = 4;
LABEL_60:
	if (v8 == 1) {
		v34 = v40;
		v33 = v42;
		NV_LIGNEDEP = v50;
		NV_LIGNEOFS = v46;
	}
	if (v8 == 2) {
		v34 = v39;
		v33 = v43;
		NV_LIGNEDEP = v51;
		NV_LIGNEOFS = v47;
	}
	if (v8 == 3) {
		v34 = v38;
		v33 = v44;
		NV_LIGNEDEP = v52;
		NV_LIGNEOFS = v48;
	}
	if (v8 == 4) {
		v34 = v37;
		v33 = v45;
		NV_LIGNEDEP = v53;
		NV_LIGNEOFS = v49;
	}
	if (v33 == 1) {
		v14 = 0;
		if (Ligne[v34].field0 > 0) {
			v32 = v34;
			v25 = _vm->_globals.essai0;
			do {
				v15 = Ligne[v32].lineData;
				v16 = v15[2 * v14];
				v29 = v15[2 * v14 + 1];
				if (!a6) {
					v17 = v41;
					_vm->_globals.super_parcours[v17] = v16;
					_vm->_globals.super_parcours[v17 + 1] = v29;
					_vm->_globals.super_parcours[v17 + 2] = Ligne[v32].field6;
					_vm->_globals.super_parcours[v17 + 3] = 0;
				}
				if (a6 == 1) {
					v18 = v41;
					v25[v18] = v16;
					v25[v18 + 1] = v29;
					v25[v18 + 2] = Ligne[v32].field6;
					v25[v18 + 3] = 0;
				}
				v41 += 4;
				++v14;
			} while (Ligne[v32].field0 > v14);
		}
	}
	if (v33 == 2) {
		v19 = Ligne[v34].field0 - 1;
		if (v19 > -1) {
			v31 = v34;
			v26 = _vm->_globals.essai0;
			do {
				v20 = Ligne[v31].lineData;
				v21 = v20[2 * v19];
				v30 = v20[2 * v19 + 1];
				if (a6) {
					v23 = v41;
					v26[v23] = v21;
					v26[v23 + 1] = v30;
					v26[v23 + 2] = Ligne[v31].field8;
					v26[v23 + 3] = 0;
				} else {
					v22 = v41;
					_vm->_globals.super_parcours[v22] = v21;
					_vm->_globals.super_parcours[v22 + 1] = v30;
					_vm->_globals.super_parcours[v22 + 2] = Ligne[v31].field8;
					_vm->_globals.super_parcours[v22 + 3] = 0;
				}
				v41 += 4;
				--v19;
			} while (v19 > -1);
		}
	}
	NV_POSI = v41;
	return 1;
}

// Test line
int LinesManager::TEST_LIGNE(int a1, int a2, int *a3, int *a4, int *a5) {
	int i; 
	int v6; 
	int16 *v7; 
	int v8; 
	int v9; 
	int v10; 
	int v11; 
	int16 *v12; 
	int v13; 
	int v14; 
	int v15; 
	int v16; 
	int16 *v17; 
	int v18; 
	int v19; 
	int v20; 
	int v21; 
	int result; 
	int v23; 
	int v24 = 0; 
	int v25; 
	int v26; 
	int v27; 
	int v28; 

	v26 = 0;
	v25 = _vm->_objectsManager.DERLIGNE + 1;
	for (i = (int)(_vm->_objectsManager.DERLIGNE + 1); i < _vm->_linesManager.TOTAL_LIGNES + 1; i = v25) {
		v6 = i;
		v7 = Ligne[i].lineData;
		v8 = Ligne[v6].field0;
		v23 = v7[2 * v8 - 2];
		v9 = v7[2 * v8 - 1];
		if (v7[0] == a1 && a2 == v7[1]) {
			v24 = v25;
			v26 = 1;
			*a3 = 1;
		}
		if (v23 == a1 && a2 == v9) {
			v24 = v25;
			v26 = 1;
			*a3 = 2;
		}
		if (v26 == 1)
			goto LABEL_12;
		++v25;
	}
	if (v26 != 1)
		goto LABEL_33;
LABEL_12:
	if (*a3 == 1) {
		v10 = v24;
		v11 = Ligne[v10].field0;
		v12 = Ligne[v10].lineData;
		v13 = v12[2 * v11 - 2];
		v14 = v12[2 * v11 - 1];
		v15 = Ligne[v10].field6;
		if (v15 == 5 || v15 == 1)
			v14 += 2;
		v16 = v24;
		if (Ligne[v16].field6 == 3 || Ligne[v16].field8 == 7)
			v13 += 2;
		if (!colision2_ligne(v13, v14, &v28, &v27, 0, _vm->_objectsManager.DERLIGNE))
			error("error");
		*a4 = v27;
		*a5 = v28;
	}
	if (v26 == 1 && *a3 == 2) {
		v17 = Ligne[v25].lineData;
		v18 = v17[0];
		v19 = v17[1];
		v20 = Ligne[v24].field6;
		if (v20 == 5 || v20 == 1)
			v19 -= 2;
		v21 = v24;
		if (Ligne[v21].field6 == 3 || Ligne[v21].field8 == 7)
			v18 -= 2;
		if (!colision2_ligne(v18, v19, &v28, &v27, 0, _vm->_objectsManager.DERLIGNE))
			error("erreure");
		*a4 = v27;
		*a5 = v28;
	}
LABEL_33:
	if (v26)
		result = v24;
	else
		result = -1;
	return result;
}

} // End of namespace Hopkins
