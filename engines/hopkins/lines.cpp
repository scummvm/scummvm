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

void LinesManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void LinesManager::CLEAR_ZONE() {
	for (int idx = 0; idx < 400; ++idx)
		RETIRE_LIGNE_ZONE(idx);

	next_ligne = 0;
}

void LinesManager::RETIRE_LIGNE_ZONE(int idx) {
	if (idx > 400)
		error("Attempting to add a line obstacle > MAX_LIGNE.");
	if (_vm->_linesManager.LigneZone[idx].field4 != PTRNUL)
		_vm->_globals.dos_free2(_vm->_linesManager.LigneZone[idx].field4);

	_vm->_linesManager.LigneZone[idx].field4 = PTRNUL;
}


void LinesManager::AJOUTE_LIGNE_ZONE(int idx, int a2, int a3, int a4, int a5, int a6) {
	int v7; 
	int v8; 
	int v9; 
	byte *v10;
	int v11; 
	byte *v12; 
	int v13; 
	int v14; 
	byte *v15; 
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	int v20; 
	int v21; 
	int v22; 
	int v23; 

	if (a2 != a3 || a3 != a4 || a3 != a5) {
		if (idx > 400)
			error("Attempting to add a line obstacle > MAX_LIGNE.");
		v7 = idx;
		if (LigneZone[v7].field4 != PTRNUL)
			LigneZone[v7].field4 = _vm->_globals.dos_free2(LigneZone[v7].field4);
		v8 = a2 - a4;
		if (a2 - a4 < 0)
			v8 = -v8;
		v19 = v8;
		v9 = a3 - a5;
		if (a3 - a5 < 0)
			v9 = -v9;
		v18 = v9;
		if (v19 <= (int)v9)
			v20 = v9 + 1;
		else
			v20 = v19 + 1;
		v10 = _vm->_globals.dos_malloc2(4 * v20 + 8);
		v11 = idx;
		LigneZone[v11].field4 = v10;
		if (PTRNUL == v10)
			error("AJOUTE LIGNE ZONE");

		v12 = LigneZone[v11].field4;
		v23 = 1000 * v19 / v20;
		v22 = 1000 * v18 / v20;
		if (a4 < a2)
			v23 = -v23;
		if (a5 < a3)
			v22 = -v22;
		v13 = 1000 * a2;
		v16 = 1000 * a3;
		v17 = 1000 * a2 / 1000;
		v21 = 1000 * a3 / 1000;
		v14 = 0;
		if (v20 > 0) {
			do {
				WRITE_LE_UINT16(v12, v17);
				v15 = v12 + 2;
				WRITE_LE_UINT16(v15, v21);
				v12 = v15 + 2;
				v13 += v23;
				v16 += v22;
				v17 = v13 / 1000;
				v21 = v16 / 1000;
				++v14;
			} while (v20 > v14);
		}
		WRITE_LE_UINT16(v12, (uint16)-1);
		WRITE_LE_UINT16(v12 + 2, (uint16)-1);
		
		LigneZone[idx].field0 = v20;
		LigneZone[idx].field2 = a6;
	} else {
		_vm->_globals.BOBZONE_FLAG[idx] = 1;
		_vm->_globals.BOBZONE[idx] = a3;
	}
}


void LinesManager::RESET_OBSTACLE() {
	for (int idx =0; idx < 400; ++idx) {
		RETIRE_LIGNE(idx);
		Ligne[idx].field0 = 0;
		Ligne[idx].fieldC = PTRNUL;
	}
}

void LinesManager::RETIRE_LIGNE(int idx) {
	if (idx > 400)
		error("Attempting to add a line obstacle > MAX_LIGNE.");
	if (Ligne[idx].fieldC != PTRNUL)
		_vm->_globals.dos_free2(Ligne[idx].fieldC);
	Ligne[idx].fieldC = PTRNUL;
}

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
	byte *v27;
	int v30;
	int v31;
	byte *v32;
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

	if (Ligne[idx].fieldC != PTRNUL)
		_vm->_globals.dos_free2(Ligne[idx].fieldC);
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
	Ligne[idx].fieldC = v10;
	if (v10 == PTRNUL)
		error("AJOUTE LIGNE OBSTACLE");
	v32 = Ligne[idx].fieldC;
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
			WRITE_LE_UINT16(v32, v31);
			WRITE_LE_UINT16(v32 + 2, v30);
			v32 += 4;
			v24 += v40;
			v25 += v38;
			v31 = v24 / 1000;
			v30 = v25 / 1000;
			++v26;
		} while (v35 > v26);
	}
	WRITE_LE_UINT16(v32, a5);
	WRITE_LE_UINT16(v32 + 2, a6);
	v27 = v32 + 4;
	WRITE_LE_UINT16(v27, (uint16)-1);
	WRITE_LE_UINT16(v27 + 2, (uint16)-1);
	
	Ligne[idx].field0 = v35 + 1;
	Ligne[idx].field2 = a7;
	Ligne[idx].field4 = a2;
}

int LinesManager::colision2_ligne(int a1, int a2, int *a3, int *a4, int a5, int a6) {
	int v6; 
	byte *v7; 
	int v8;
	int v9;
	int v10;
	int v11;
	int v12;
	byte *v13; 
	int v14;
	int result; 
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	int v20; 
	int v21; 
	int v22; 
	int v23; 
	int v24; 

	v24 = a5;
	v6 = a5;
	if (a5 >= a6 + 1) {
LABEL_29:
		result = 0;
	} else {
		v22 = a1 + 4;
		v21 = a1 - 4;
		v20 = a2 + 4;
		v19 = a2 - 4;
		while (1) {
			v7 = Ligne[v6].fieldC;
			if (v7 != PTRNUL) {
				v23 = 1;
				v8 = 2 * Ligne[v6].field0;
				v9 = READ_LE_UINT16(v7);
				v16 = READ_LE_UINT16(v7 + 2);
				v10 = READ_LE_UINT16(v7 + 2 * v8 - 4);
				v18 = READ_LE_UINT16(v7 + 2 * v8 - 2);
				if (READ_LE_UINT16(v7) >= v10)
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
		while (1) {
			v12 = READ_LE_UINT16(v7);
			v13 = v7 + 2;
			v14 = READ_LE_UINT16(v13);
			v7 = v13 + 2;
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
	int v6; 
	byte *v7; 
	int v8; 
	int v9; 
	int v10;
	int v11;
	int v12;
	byte *v13;
	int result;
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

	v24 = a5;
	v6 = a5;
	if (a5 >= a6 + 1) {
LABEL_47:
		result = 0;
	} else {
		v22 = a1 + 10;
		v21 = a1 - 10;
		v20 = a2 + 10;
		v19 = a2 - 10;
		while (1) {
			v7 = Ligne[v6].fieldC;
			if (PTRNUL != v7) {
				v23 = 1;
				v8 = 2 * Ligne[v6].field0;
				v9 = READ_LE_UINT16(v7);
				v15 = READ_LE_UINT16(v7 + 2);
				v10 = READ_LE_UINT16(v7 + 2 * v8 - 4);
				v17 = READ_LE_UINT16(v7 + 2 * v8 - 2);
				if (READ_LE_UINT16(v7) >= v10)
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
		while (1) {
			v12 = READ_LE_UINT16(v7);
			v13 = v7 + 2;
			v16 = READ_LE_UINT16(v13);
			v7 = v13 + 2;
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

void LinesManager::INIPARCOURS() {
	int v0; 
	int v1; 
	int v2; 
	byte *v3; 
	int v4; 
	int v5; 
	int v6; 
	byte *v7; 
	int v8; 
	int v9; 
	int v11;
	int v12;
	int v13;
	int v14;
	int v15;

	v15 = READ_LE_UINT16(Ligne[0].fieldC);
	v14 = READ_LE_UINT16(Ligne[0].fieldC + 2);

	v0 = 1;
	v1 = 0;
	do {
		v2 = Ligne[v0].field0;
		v3 = Ligne[v0].fieldC;
		v4 = READ_LE_UINT16(v3 + 4 * v2 - 4);
		v5 = READ_LE_UINT16(v3 + 4 * v2 - 2);
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
		v7 = Ligne[v0 + 1].fieldC;
		if (READ_LE_UINT16(v7) != v4 && READ_LE_UINT16(v7 + 2) != v5)
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

int LinesManager::CONTOURNE(int a1, int a2, int a3, int a4, int a5, byte *a6, int a7) {
	int v7; 
	int v8; 
	int i; 
	byte *v10;
	int v11;
	int v12;
	int v13;
	byte *v14;
	int v15;
	int v16;
	int j; 
	byte *v18;
	int v19;
	int v20;
	int k; 
	byte *v22;
	int v23; 
	int v24; 
	int v25; 
	byte *v26; 
	int v27; 
	int v28; 
	int v29; 
	byte *v30; 
	int v31; 
	int v32; 
	int v34; 
	int v35; 
	int v36; 
	int v50;

	v36 = a1;
	v7 = a2;
	v8 = a3;
	if (a1 < a4) {
		for (i = a2; Ligne[a1].field0 > i; ++i) {
			v10 = Ligne[a1].fieldC;
			v11 = READ_LE_UINT16(v10 + 4 * i);
			v50 = READ_LE_UINT16(v10 + 4 * i + 2);
			v12 = v8;
			WRITE_LE_UINT16(a6 + 2 * v12, v11);
			WRITE_LE_UINT16(a6 + 2 * v12 + 2, v50);
			WRITE_LE_UINT16(a6 + 2 * v12 + 4, Ligne[a1].field6);
			v8 += a7;
		}
		v34 = a1 + 1;
		if ((int)(a1 + 1) < a4) {
			do {
				v13 = 0;
				if (Ligne[v34].field0 > 0) {
					do {
						v14 = Ligne[v34].fieldC;
						v15 = READ_LE_UINT16(v14 + 4 * v13);
						v50 = READ_LE_UINT16(v14 + 4 * v13 + 2);
						v16 = v8;
						WRITE_LE_UINT16(a6 + 2 * v16, v15);
						WRITE_LE_UINT16(a6 + 2 * v16 + 2, v50);
						WRITE_LE_UINT16(a6 + 2 * v16 + 4, Ligne[v34].field6);
						v8 += a7;
						++v13;
					} while (Ligne[v34].field0 > v13);
				}
				++v34;
			} while (v34 < a4);
		}
		v7 = 0;
		v36 = a4;
	}
	if (v36 > a4) {
		for (j = v7; j > 0; --j) {
			v18 = Ligne[v36].fieldC;
			v19 = READ_LE_UINT16(v18 + 4 * j);
			v50 = READ_LE_UINT16(v18 + 4 * j + 2);
			v20 = v8;
			WRITE_LE_UINT16(a6 + 2 * v20, v19);
			WRITE_LE_UINT16(a6 + 2 * v20 + 2, v50);
			WRITE_LE_UINT16(a6 + 2 * v20 + 4, Ligne[v36].field8);
			v8 += a7;
		}
		v35 = v36 - 1;
		if ((int)(v36 - 1) > a4) {
			do {
				for (k = Ligne[v35].field0 - 1; k > 0; --k) {
					v22 = Ligne[v35].fieldC;
					v23 = READ_LE_UINT16(v22 + 4 * k);
					v50 = READ_LE_UINT16(v22 + 4 * k + 2);
					v24 = v8;
					WRITE_LE_UINT16(a6 + 2 * v24, v23);
					WRITE_LE_UINT16(a6 + 2 * v24 + 2, v50);
					WRITE_LE_UINT16(a6 + 2 * v24 + 4, Ligne[v35].field8);
					v8 += a7;
				}
				--v35;
			} while (v35 > a4);
		}
		v7 = Ligne[a4].field0 - 1;
		v36 = a4;
	}
	if (v36 == a4) {
		if (a5 >= v7) {
			if (a5 > v7) {
				v29 = v7;
				do {
					v30 = Ligne[a4].fieldC;
					v31 = READ_LE_UINT16(v30 + 4 * v29);
					v50 = READ_LE_UINT16(v30 + 4 * v29 + 2);
					v32 = v8;
					WRITE_LE_UINT16(a6 + 2 * v32, v31);
					WRITE_LE_UINT16(a6 + 2 * v32 + 2, v50);
					WRITE_LE_UINT16(a6 + 2 * v32 + 4, Ligne[a4].field6);
					v8 += a7;
					++v29;
				} while (a5 > v29);
			}
		} else {
			v25 = v7;
			do {
				v26 = Ligne[a4].fieldC;
				v27 = READ_LE_UINT16(v26 + 4 * v25);
				v50 = READ_LE_UINT16(v26 + 4 * v25 + 2);
				v28 = v8;
				WRITE_LE_UINT16(a6 + 2 * v28, v27);
				WRITE_LE_UINT16(a6 + 2 * v28 + 2, v50);
				WRITE_LE_UINT16(a6 + 2 * v28 + 4, Ligne[a4].field8);
				v8 += a7;
				--v25;
			} while (a5 < v25);
		}
	}
	return v8;
}

int LinesManager::CONTOURNE1(int a1, int a2, int a3, int a4, int a5, byte *a6, int a7, int a8, int a9) {
	int v9; 
	int v10;
	int i; 
	byte *v12;
	int v13;
	int v14;
	int v15;
	int v16;
	byte *v17;
	int v18;
	int v19;
	int j; 
	byte *v21;
	int v22;
	int v23;
	int v24; 
	int k; 
	byte *v26;
	int v27;
	int v28;
	int v29;
	byte *v30;
	int v31;
	int v32;
	int v33;
	byte *v34;
	int v35;
	int v36;
	int v38;
	int v39;
	int v40;
	int v50;

	v9 = a1;
	v10 = a2;
	v40 = a3;
	if (a4 < a1) {
		for (i = a2; Ligne[a1].field0 > i; ++i) {
			v12 = Ligne[a1].fieldC;
			v13 = READ_LE_UINT16(v12 + 4 * i);
			v50 = READ_LE_UINT16(v12 + 4 * i + 2);
			v14 = v40;
			WRITE_LE_UINT16(a6 + 2 * v14, v13);
			WRITE_LE_UINT16(a6 + 2 * v14 + 2, v50);
			WRITE_LE_UINT16(a6 + 2 * v14 + 4, Ligne[a1].field6);
			v40 += a7;
		}
		v15 = a1 + 1;
		if ((int)(a1 + 1) == a9 + 1)
			v15 = a8;
		while (a4 != v15) {
			v16 = 0;
			if (Ligne[v15].field0 > 0) {
				do {
					v17 = Ligne[v15].fieldC;
					v18 = READ_LE_UINT16(v17 + 4 * v16);
					v50 = READ_LE_UINT16(v17 + 4 * v16 + 2);
					v19 = v40;
					WRITE_LE_UINT16(a6 + 2 * v19, v18);
					WRITE_LE_UINT16(a6 + 2 * v19 + 2, v50);
					WRITE_LE_UINT16(a6 + 2 * v19 + 4, Ligne[v15].field6);
					v40 += a7;
					++v16;
				} while (Ligne[v15].field0 > v16);
			}
			++v15;
			if (a9 + 1 == v15)
				v15 = a8;
		}
		v10 = 0;
		v9 = a4;
	}
	if (a4 > v9) {
		for (j = v10; j > 0; --j) {
			v21 = Ligne[v9].fieldC;;
			v22 = READ_LE_UINT16(v21 + 4 * j);
			v50 = READ_LE_UINT16(v21 + 4 * j + 2);
			v23 = v40;
			WRITE_LE_UINT16(a6 + 2 * v23, v22);
			WRITE_LE_UINT16(a6 + 2 * v23 + 2, v50);
			WRITE_LE_UINT16(a6 + 2 * v23 + 4, Ligne[v9].field8);
			v40 += a7;
		}
		v24 = v9 - 1;
		if (v24 == a8 - 1)
			v24 = a9;
		while (a4 != v24) {
			for (k = Ligne[v24].field0 - 1; k > 0; --k) {
				v26 = Ligne[v24].fieldC;
				v27 = READ_LE_UINT16(v26 + 4 * k);
				v50 = READ_LE_UINT16(v26 + 4 * k + 2);
				v28 = v40;
				WRITE_LE_UINT16(a6 + 2 * v28, v27);
				WRITE_LE_UINT16(a6 + 2 * v28 + 2, v50);
				WRITE_LE_UINT16(a6 + 2 * v28 + 4, Ligne[v24].field8);
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
				v33 = v10;
				v39 = a4;
				do {
					v34 = Ligne[v39].fieldC;
					v35 = READ_LE_UINT16(v34 + 4 * v33);
					v50 = READ_LE_UINT16(v34 + 4 * v33 + 2);
					v36 = v40;
					WRITE_LE_UINT16(a6 + 2 * v36, v35);
					WRITE_LE_UINT16(a6 + 2 * v36 + 2, v50);
					WRITE_LE_UINT16(a6 + 2 * v36 + 4, Ligne[v39].field6);
					v40 += a7;
					++v33;
				} while (a5 > v33);
			}
		} else {
			v29 = v10;
			v38 = a4;
			do {
				v30 = Ligne[v38].fieldC;
				v31 = READ_LE_UINT16(v30 + 4 * v29);
				v50 = READ_LE_UINT16(v30 + 4 * v29 + 2);
				v32 = v40;
				WRITE_LE_UINT16(a6 + 2 * v32, v31);
				WRITE_LE_UINT16(a6 + 2 * v32 + 2, v50);
				WRITE_LE_UINT16(a6 + 2 * v32 + 4, Ligne[v38].field8);
				v40 += a7;
				--v29;
			} while (a5 < v29);
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
									v23 = GENIAL(v46, v47, v41, v40 - v22, v41, v40 - v39, v7, (byte *)&_vm->_globals.super_parcours[0], 4);
							if (v23 == -1)
								return 0;
							v7 = v23;
							if (_vm->_globals.NVPY != -1)
								v22 = _vm->_globals.NVPY - v40;
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
							v26 = GENIAL(v46, v47, v41, v25 + v40, v41, v37 + v40, v7, (byte *)&_vm->_globals.super_parcours[0], 4);
							if (v26 == -1)
								return 0;
							v7 = v26;
							if (_vm->_globals.NVPY != -1)
								v25 = v40 - _vm->_globals.NVPY;
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
							v29 = GENIAL(v46, v47, v41 - v28, v40, v41 - v18, v40, v7, (byte *)&_vm->_globals.super_parcours[0], 4);
							if (v29 == -1)
								return 0;
							v7 = v29;
							if (_vm->_globals.NVPX != -1)
								v28 = v41 - _vm->_globals.NVPX;
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
							v32 = GENIAL(v46, v47, v31 + v41, v40, v38 + v41, v40, v7, (byte *)&_vm->_globals.super_parcours[0], 4);
							if (v32 == -1)
								return 0;
							v7 = v32;
							if (_vm->_globals.NVPX != -1)
								v31 = _vm->_globals.NVPX - v41;
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

int LinesManager::GENIAL(int a1, int a2, int a3, int a4, int a5, int a6, int a7, byte *a8, int a9) {
	int v9; 
	byte *v10;
	int v11; 
	byte *v12; 
	byte *v13; 
	int v14; 
	int v15;
	byte *v16; 
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
	byte *v28; 
	int v29;
	int v30;
	int v31; 
	int v32;
	int v33;
	int v34;
	int v35 = 0;
	int v36 = 0;
	int v37;
	int v38;
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
	void *v51;
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
	int v66 = 0;
	int v67;
	int v68 = 0;
	int v69;
	int v70 = 0;
	int v71;
	int v72 = 0;
	int v73;
	int v74;
	int v75;
	int v76;
	int v77 = 0; 
	int v78 = 0; 
	int v79 = 0; 
	int v80; 
	int v81; 
	int v82; 
	int v83; 
	int v84; 
	int v85;
	int v86; 
	int v87; 
	int v88; 
	int v89; 
	int i; 
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

	v99 = a7;
	v80 = -1;
	++_vm->_globals.STOP_BUG;
	if (_vm->_globals.STOP_BUG > 10) {
		v9 = a7;
LABEL_112:
		WRITE_LE_UINT16(a8 + 2 * v9, (uint16)-1);
		WRITE_LE_UINT16(a8 + 2 * v9 + 2, (uint16)-1);
		WRITE_LE_UINT16(a8 + 2 * v9 + 4, (uint16)-1);
		return -1;
	}
	v10 = Ligne[a1].fieldC;
	v98 = READ_LE_UINT16(v10);
	v97 = READ_LE_UINT16(v10 + 2);
	v92 = a1;

	while (1) {
		v86 = v92 - 1;
		v11 = 2 * Ligne[v92 - 1].field0;
		v12 = Ligne[v92 - 1].fieldC;
		if (v12 == PTRNUL)
			break;
		while (READ_LE_UINT16(v12 + 2 * v11 - 4) != v98 || v97 != READ_LE_UINT16(v12 + 2 * v11 - 2)) {
			--v86;
			if (_vm->_objectsManager.DERLIGNE - 1 != v86) {
				v11 = 2 * Ligne[v86].field0;
				v12 = Ligne[v86].fieldC;
				if (v12 != PTRNUL)
					continue;
			}
			goto LABEL_11;
		}
		v92 = v86;
		v98 = READ_LE_UINT16(v12);
		v97 = READ_LE_UINT16(v12 + 2);
	}
LABEL_11:
	v13 = Ligne[a1].fieldC;
	v14 = 2 * Ligne[a1].field0;
	v95 = READ_LE_UINT16(v13 + 2 * v14 - 4);
	v93 = READ_LE_UINT16(v13 + 2 * v14 - 2);
	v91 = a1;
	while (1) {
		v87 = v91 + 1;
		v15 = 2 * Ligne[v91 + 1].field0;
		v16 = Ligne[v91 + 1].fieldC;
		if (PTRNUL == v16)
			break;
		while (1) {
			v65 = v15;
			v17 = READ_LE_UINT16(v16 + 2 * v15 - 4);
			if (READ_LE_UINT16(v16) == v95) {
				if (v93 == READ_LE_UINT16(v16 + 2))
					break;
			}
			++v87;
			if (v87 != TOTAL_LIGNES + 1) {
				v15 = 2 * Ligne[v87].field0;
				v16 = Ligne[v87].fieldC;
				if (v16 != PTRNUL)
					continue;
			}
			goto LABEL_17;
		}
		v91 = v87;
		v95 = v17;
		v93 = READ_LE_UINT16(v16 + 2 * v65 - 2);
	}
LABEL_17:
	v18 = a3 - a5;
	if (a3 - a5 < 0)
		v18 = -v18;
	v58 = v18 + 1;
	v19 = a4 - a6;
	if (a4 - a6 < 0)
		v19 = -v19;
	v85 = v19 + 1;
	v20 = v19 + 1;
	if (v58 > v20)
		v85 = v18 + 1;
	v84 = 1000 * v58 / v85;
	v83 = 1000 * v20 / v85;
	v21 = 1000 * a3;
	v22 = 1000 * a4;
	v82 = v21 / 1000;
	v81 = v22 / 1000;
	if (a5 < a3)
		v84 = -v84;
	if (a6 < a4)
		v83 = -v83;
	if (v85 > 800)
		v85 = 800;
	memset(_vm->_globals.BufLig, 0, 0x7D0u);
	v23 = 0;
	v88 = 0;
	if (v85 + 1 > 0) {
		v51 = _vm->_globals.BufLig;
		do {
			v24 = v23;
			*((uint16 *)v51 + v24) = v82;
			*((uint16 *)v51 + v24 + 1) = v81;
			v21 += v84;
			v22 += v83;
			v82 = v21 / 1000;
			v81 = v22 / 1000;
			v23 += 2;
			++v88;
		} while (v88 < v85 + 1);
	}
	v25 = v23 - 2;
	v26 = 0;
	v89 = v85 + 1;
	if ((int)(v85 + 1) > 0) {
		do {
			v96 = READ_LE_UINT16((uint16 *)_vm->_globals.BufLig + v25);
			v94 = READ_LE_UINT16((uint16 *)_vm->_globals.BufLig + v25 + 1);
			if (colision2_ligne(v96, v94, &v101, &v100, v92, v91) == 1 && _vm->_objectsManager.DERLIGNE < v100) {
				v80 = v100;
				v77 = v101;
				v78 = v96;
				v79 = v94;
				v26 = 1;
			}
			if (v26 == 1)
				break;
			v25 -= 2;
			--v89;
		} while (v89 > 0);
	}
	for (i = v92; i < v91 + 1; ++i) {
		v27 = 2 * Ligne[i].field0;
		v28 = Ligne[i].fieldC;
		if (PTRNUL == v28)
			error("erreur dans da routine genial");
		v29 = *(uint16 *)v28;
		v30 = READ_LE_UINT16(v28 + 2);
		v59 = READ_LE_UINT16(v28 + 2);
		v31 = v27;
		v32 = READ_LE_UINT16(v28 + 2 * v27 - 4);
		v33 = READ_LE_UINT16(v28 + 2 * v31 - 2);
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
	v69 = v68 - 2;
	v73 = v72 - 2;
	v67 = v66 + 2;
	v71 = v70 + 2;
	if (a5 >= v69 && a5 <= v67 && a6 >= v73 && a6 <= v71) {
		v34 = a6;
		v76 = -1;
		do {
			--v34;
			v60 = colision2_ligne(a5, v34, &v101, &v100, v92, v91);
			if (v60 == 1)
				v76 = v100;
			if (!v34 || v73 > v34)
				v60 = 1;
		} while (v60 != 1);
		v35 = a6;
		v75 = -1;
		do {
			++v35;
			v61 = colision2_ligne(a5, v35, &v101, &v100, v92, v91);
			if (v61 == 1)
				v75 = v100;
			if (_vm->_globals.Max_Perso_Y <= v35 || v71 <= v35)
				v61 = 1;
		} while (v61 != 1);
		v36 = a5;
		v74 = -1;
		do {
			++v36;
			v62 = colision2_ligne(v36, a6, &v101, &v100, v92, v91);
			if (v62 == 1)
				v74 = v100;
			if (_vm->_graphicsManager.max_x <= v36 || v67 <= v36)
				v62 = 1;
		} while (v62 != 1);
		v37 = a5;
		v38 = -1;
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
		_vm->_globals.NVPX = v78;
		_vm->_globals.NVPY = v79;
		if (a1 < v80) {
			v40 = v80 - a1;
			if (v80 - a1 < 0)
				v40 = -v40;
			v41 = v40;
			v42 = a1;
			v43 = 0;
			v52 = v92 - 1;
			do {
				if (v52 == v42)
					v42 = v91;
				++v43;
				--v42;
				if (v52 == v42)
					v42 = v91;
			} while (v80 != v42);
			if (v41 == v43) {
				v44 = Ligne[a1].field0 / 2;
				v54 = Ligne[a1].field0 / 2;
				if (v44 < 0)
					v54 = -v44;
				if (a2 > v54) {
					v55 = Ligne[a1].field0 / 2;
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
			v45 = a1 - v80;
			if (a1 - v80 < 0)
				v45 = -v45;
			v46 = v45;
			v47 = a1;
			v48 = 0;
			v53 = v91 + 1;
			do {
				if (v53 == v47)
					v47 = v92;
				++v48;
				++v47;
				if (v53 == v47)
					v47 = v92;
			} while (v80 != v47);
			if (v46 == v48) {
				v49 = Ligne[a1].field0 / 2;
				v56 = Ligne[a1].field0 / 2;
				if (v49 < 0)
					v56 = -v49;
				if (a2 > v56) {
					v57 = Ligne[a1].field0 / 2;
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
		do {
			v64 = colision2_ligne(_vm->_globals.NVPX, _vm->_globals.NVPY, &v101, &v100, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES);
			if (v64 == 1) {
				v50 = 16 * v100;
				if (*(uint16 *)&Ligne[v50 + 4] == 1)
					--_vm->_globals.NVPY;
				if (*(uint16 *)&Ligne[v50 + 4] == 2) {
					--_vm->_globals.NVPY;
					++_vm->_globals.NVPX;
				}
				if (*(uint16 *)&Ligne[v50 + 4] == 3)
					++_vm->_globals.NVPX;
				if (*(uint16 *)&Ligne[v50 + 4] == 4) {
					++_vm->_globals.NVPY;
					++_vm->_globals.NVPX;
				}
				if (*(uint16 *)&Ligne[v50 + 4] == 5)
					++_vm->_globals.NVPY;
				if (*(uint16 *)&Ligne[v50 + 4] == 6) {
					++_vm->_globals.NVPY;
					--_vm->_globals.NVPX;
				}
				if (*(uint16 *)&Ligne[v50 + 4] == 7)
					--_vm->_globals.NVPX;
				if (*(uint16 *)&Ligne[v50 + 4] == 8) {
					--_vm->_globals.NVPY;
					--_vm->_globals.NVPX;
				}
			}
		} while (v64);
	} else {
		_vm->_globals.NVPX = -1;
		_vm->_globals.NVPY = -1;
	}
	return v99;
}

// TODO: method casting int arrays as byte pointers. Double-check later whether
// we can convert the return to a uint16 *
byte *LinesManager::PARCOURS2(int a1, int a2, int a3, int a4) {
	int v4; 
	int v5; 
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
	int v31;
	int v32; 
	int v33; 
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
	byte *v70; 
	int v71; 
	int v72; 
	int j; 
	byte *v74;
	int v75; 
	int v76;
	int v77; 
	int v78; 
	int v79; 
	int v80;
	byte *v81; 
	int v82; 
	int v83; 
	byte *v84;
	int v85; 
	int v86;
	int v87; 
	int v88; 
	int v89; 
	int v90; 
	byte *v91; 
	int v92; 
	int v93; 
	int v94;
	byte *v95; 
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
	int v118 = 0; 
	int v119 = 0; 
	int v120; 
	int v121; 
	int v122;
	int v123; 
	int v124; 
	int v125;
	int v126; 
	int v127 = 0;
	int v128 = 0; 
	int v129 = 0; 
	int v130 = 0; 
	int v131; 
	int v132; 
	int v133; 
	int v134; 
	int v135;
	int v136; 
	int v137 = 0; 
	int v138 = 0;
	int v139 = 0;
	int v140 = 0;
	int v141; 
	int v142 = 0;
	int v143 = 0; 
	int v144 = 0; 
	int v145 = 0; 

	v123 = a3;
	v122 = a4;
	v121 = 0;
	v120 = 0;
	v115 = 0;
	v114 = 0;
	v113 = 0;
	v111 = 0;
	if (a4 <= 24)
		v122 = 25;
	if (!_vm->_globals.NOT_VERIF) {
		v4 = a1 - _vm->_globals.old_x1_65;
		if (v4 < 0)
			v4 = -v4;
		if (v4 <= 4) {
			v5 = a2 - _vm->_globals.old_y1_66;
			if (v5 < 0)
				v5 = -v5;
			if (v5 <= 4) {
				v6 = _vm->_globals.old_x2_67 - a3;
				if (v6 < 0)
					v6 = -v6;
				if (v6 <= 4) {
					v7 = _vm->_globals.old_y2_68 - v122;
					if (v7 < 0)
						v7 = -v7;
					if (v7 <= 4)
						return PTRNUL;
				}
			}
		}
		v8 = a1 - a3;
		if (v8 < 0)
			v8 = -v8;
		if (v8 <= 4) {
			v9 = a2 - v122;
			if (v9 < 0)
				v9 = -v9;
			if (v9 <= 4)
				return PTRNUL;
		}
		if (_vm->_globals.old_z_69 > 0 && _vm->_objectsManager.NUMZONE > 0 && _vm->_globals.old_z_69 == _vm->_objectsManager.NUMZONE)
			return PTRNUL;
	}
	_vm->_globals.NOT_VERIF = 0;
	_vm->_globals.old_z_69 = _vm->_objectsManager.NUMZONE;
	_vm->_globals.old_x1_65 = a1;
	_vm->_globals.old_x2_67 = a3;
	_vm->_globals.old_y1_66 = a2;
	_vm->_globals.old_y2_68 = v122;
	_vm->_globals.STOP_BUG = 0;
	v112 = 0;
	if (a3 <= 19)
		v123 = 20;
	if (v122 <= 19)
		v122 = 20;
	if (v123 > _vm->_graphicsManager.max_x - 10)
		v123 = _vm->_graphicsManager.max_x - 10;
	if (v122 > _vm->_globals.Max_Perso_Y)
		v122 = _vm->_globals.Max_Perso_Y;
	v10 = a1 - v123;
	if (v10 < 0)
		v10 = -v10;
	if (v10 <= 3) {
		v11 = a2 - v122;
		if (v11 < 0)
			v11 = -v11;
		if (v11 <= 3)
			return PTRNUL;
	}
	v12 = 0;
error("TODO: Fix v141/v136 - they look like arrays, not a single int");
	do {
		v13 = v12;
		*(&v141 + v13) = -1;
		*(&v136 + v13) = 0;
		*(&v131 + v13) = 1300;
		*(&v126 + v13) = 1300;
		++v12;
	} while (v12 <= 8);
	if (PARC_PERS(a1, a2, v123, v122, -1, -1, 0) != 1) {
		v14 = 0;
		v15 = v122;
		if (_vm->_graphicsManager.max_y > v122) {
			v16 = 5;
			do {
				v101 = v16;
				v17 = colision2_ligne(v123, v15, &v139, &v144, 0, _vm->_objectsManager.DERLIGNE);
				v16 = v101;
				if (v17 == 1 && *(&v141 + v101) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v101) = 0;
				*(&v141 + v101) = -1;
				++v14;
				++v15;
			} while (_vm->_graphicsManager.max_y > v15);
		}
		v134 = v14;
		v18 = 0;
		v19 = v122;
		if (_vm->_graphicsManager.min_y < v122) {
			v20 = 1;
			do {
				v102 = v20;
				v21 = colision2_ligne(v123, v19, &v137, &v142, 0, _vm->_objectsManager.DERLIGNE);
				v20 = v102;
				if (v21 == 1 && *(&v141 + v102) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v102) = 0;
				*(&v141 + v102) = -1;
				if (v134 < v18) {
					if (v144 != -1)
						break;
				}
				++v18;
				--v19;
			} while (_vm->_graphicsManager.min_y < v19);
		}
		v132 = v18;
		v22 = 0;
		v23 = v123;
		if (_vm->_graphicsManager.max_x > v123) {
			v24 = 3;
			do {
				v103 = v24;
				v25 = colision2_ligne(v23, v122, &v138, &v143, 0, _vm->_objectsManager.DERLIGNE);
				v24 = v103;
				if (v25 == 1 && *(&v141 + v103) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v103) = 0;
				*(&v141 + v103) = -1;
				++v22;
				if (v132 < v22) {
					if (v142 != -1)
						break;
				}
				if (v134 < v22 && v144 != -1)
					break;
				++v23;
			} while (_vm->_graphicsManager.max_x > v23);
		}
		v133 = v22;
		v26 = 0;
		v27 = v123;
		if (_vm->_graphicsManager.min_x < v123) {
			v28 = 7;
			do {
				v104 = v28;
				v29 = colision2_ligne(v27, v122, &v140, &v145, 0, _vm->_objectsManager.DERLIGNE);
				v28 = v104;
				if (v29 == 1 && *(&v141 + v104) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v104) = 0;
				*(&v141 + v104) = -1;
				++v26;
				if (v132 < v26) {
					if (v142 != -1)
						break;
				}
				if (v134 < v26 && v144 != -1)
					break;
				if (v133 < v26 && v143 != -1)
					break;
				--v27;
			} while (_vm->_graphicsManager.min_x < v27);
		}
		v135 = v26;
		if (v142 < 0 || _vm->_objectsManager.DERLIGNE < v142)
			v142 = -1;
		if (v143 < 0 || _vm->_objectsManager.DERLIGNE < v143)
			v143 = -1;
		if (v144 < 0 || _vm->_objectsManager.DERLIGNE < v144)
			v144 = -1;
		if (v145 < 0 || _vm->_objectsManager.DERLIGNE < v145)
			v145 = -1;
		if (v142 < 0)
			v132 = 1300;
		if (v143 < 0)
			v133 = 1300;
		if (v144 < 0)
			v134 = 1300;
		if (v145 < 0)
			v135 = 1300;
		if (v142 == -1 && v143 == -1 && v144 == -1 && v145 == -1)
			return PTRNUL;
		v31 = 0;
		if (v144 != -1 && v132 >= v134 && v133 >= v134 && v135 >= v134) {
			v121 = v144;
			v120 = v139;
			v31 = 1;
		}
		if (v142 != -1 && !v31 && v134 >= v132 && v133 >= v132 && v135 >= v132) {
			v121 = v142;
			v120 = v137;
			v31 = 1;
		}
		if (v143 != -1 && !v31 && v132 >= v133 && v134 >= v133 && v135 >= v133) {
			v121 = v143;
			v120 = v138;
			v31 = 1;
		}
		if (v145 != -1 && !v31 && v134 >= v135 && v133 >= v135 && v132 >= v135) {
			v121 = v145;
			v120 = v140;
		}
		v32 = 0;
		do {
			v33 = v32;
			*(&v141 + v33) = -1;
			*(&v136 + v33) = 0;
			*(&v131 + v33) = 1300;
			*(&v126 + v33) = 1300;
			++v32;
		} while (v32 <= 8);
		v34 = 0;
		v35 = a2;
		if (_vm->_graphicsManager.max_y > a2) {
			v36 = 5;
			do {
				v105 = v36;
				v37 = colision2_ligne(a1, v35, &v139, &v144, 0, _vm->_objectsManager.DERLIGNE);
				v36 = v105;
				if (v37 == 1 && *(&v141 + v105) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v105) = 0;
				*(&v141 + v105) = -1;
				++v34;
				++v35;
			} while (_vm->_graphicsManager.max_y > v35);
		}
		v134 = v34 + 1;
		v38 = 0;
		v39 = a2;
		if (_vm->_graphicsManager.min_y < a2) {
			v40 = 1;
			do {
				v106 = v40;
				v41 = colision2_ligne(a1, v39, &v137, &v142, 0, _vm->_objectsManager.DERLIGNE);
				v40 = v106;
				if (v41 == 1 && *(&v141 + v106) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v106) = 0;
				*(&v141 + v106) = -1;
				++v38;
				if (v144 != -1) {
					if (v38 > 80)
						break;
				}
				--v39;
			} while (_vm->_graphicsManager.min_y < v39);
		}
		v132 = v38 + 1;
		v42 = 0;
		v43 = a1;
		if (_vm->_graphicsManager.max_x > a1) {
			v44 = 3;
			do {
				v107 = v44;
				v45 = colision2_ligne(v43, a2, &v138, &v143, 0, _vm->_objectsManager.DERLIGNE);
				v44 = v107;
				if (v45 == 1 && *(&v141 + v107) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v107) = 0;
				*(&v141 + v107) = -1;
				++v42;
				if (v144 != -1 || v142 != -1) {
					if (v42 > 100)
						break;
				}
				++v43;
			} while (_vm->_graphicsManager.max_x > v43);
		}
		v133 = v42 + 1;
		v46 = 0;
		v47 = a1;
		if (_vm->_graphicsManager.min_x < a1) {
			v48 = 7;
			do {
				v108 = v48;
				v49 = colision2_ligne(v47, a2, &v140, &v145, 0, _vm->_objectsManager.DERLIGNE);
				v48 = v108;
				if (v49 == 1 && *(&v141 + v108) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v108) = 0;
				*(&v141 + v108) = -1;
				++v46;
				if (v144 != -1 || v142 != -1 || v143 != -1) {
					if (v46 > 100)
						break;
				}
				--v47;
			} while (_vm->_graphicsManager.min_x < v47);
		}
		v135 = v46 + 1;
		if (v142 != -1) {
			v50 = v142 - v121;
			if (v50 < 0)
				v50 = -v50;
			v127 = v50;
		}
		if (v143 != -1) {
			v51 = v143 - v121;
			if (v51 < 0)
				v51 = -v51;
			v128 = v51;
		}
		if (v144 != -1) {
			v52 = v144 - v121;
			if (v52 < 0)
				v52 = -v52;
			v129 = v52;
		}
		if (v145 != -1) {
			v53 = v145 - v121;
			if (v53 < 0)
				v53 = -v53;
			v130 = v53;
		}
		if (v142 == -1 && v143 == -1 && v144 == -1 && v145 == -1)
			error("ERREUR POINT PLUS PROCHE Du perso NON TROUVE");
		v54 = 0;
		if (v142 != -1 && v128 >= v127 && v129 >= v127 && v130 >= v127) {
			v54 = 1;
			v115 = v142;
			v111 = v132;
			v113 = 1;
			v114 = v137;
		}
		if (!v54) {
			if (v144 != -1 && v128 >= v129 && v127 >= v129 && v130 >= v129) {
				v54 = 1;
				v115 = v144;
				v111 = v134;
				v113 = 5;
				v114 = v139;
			}
			if (!v54) {
				if (v143 != -1 && v127 >= v128 && v129 >= v128 && v130 >= v128) {
					v54 = 1;
					v115 = v143;
					v111 = v133;
					v113 = 3;
					v114 = v138;
				}
				if (!v54 && v145 != -1 && v127 >= v130 && v129 >= v130 && v128 >= v130) {
					v115 = v145;
					v111 = v135;
					v113 = 7;
					v114 = v140;
				}
			}
		}
		v55 = PARC_PERS(a1, a2, v123, v122, v115, v121, 0);
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
							if ((uint16)colision2_ligne(a1, a2 - v56, &v125, &v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v58 = GENIAL(v124, v125, a1, a2 - v56, a1, a2 - v111, v112, (byte *)&_vm->_globals.super_parcours[0], 4);
								if (v58 == -1)
									goto LABEL_282;
								v112 = v58;
								if (_vm->_globals.NVPY != -1)
									v56 = a2 - _vm->_globals.NVPY;
							}
							v59 = v112;
							_vm->_globals.super_parcours[v59] = a1;
							_vm->_globals.super_parcours[v59 + 1] = a2 - v56;
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
							if ((uint16)colision2_ligne(a1, v60 + a2, &v125, &v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v61 = GENIAL(v124, v125, a1, v60 + a2, a1, v111 + a2, v112, (byte *)&_vm->_globals.super_parcours[0], 4);
								if (v61 == -1)
									goto LABEL_282;
								v112 = v61;
								if (_vm->_globals.NVPY != -1)
									v60 = _vm->_globals.NVPY - a2;
							}
							v62 = v112;
							_vm->_globals.super_parcours[v62] = a1;
							_vm->_globals.super_parcours[v62 + 1] = v60 + a2;
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
							if ((uint16)colision2_ligne(a1 - v63, a2, &v125, &v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v64 = GENIAL(v124, v125, a1 - v63, a2, a1 - v111, a2, v112, (byte *)&_vm->_globals.super_parcours[0], 4);
								if (v64 == -1)
									goto LABEL_282;
								v112 = v64;
								if (_vm->_globals.NVPX != -1)
									v63 = a1 - _vm->_globals.NVPX;
							}
							v65 = v112;
							_vm->_globals.super_parcours[v65] = a1 - v63;
							_vm->_globals.super_parcours[v65 + 1] = a2;
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
							if ((uint16)colision2_ligne(v66 + a1, a2, &v125, &v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v67 = GENIAL(v124, v125, v66 + a1, a2, v111 + a1, a2, v112, (byte *)&_vm->_globals.super_parcours[0], 4);
								if (v67 == -1)
									goto LABEL_282;
								v112 = v67;
								if (_vm->_globals.NVPX != -1)
									v66 = _vm->_globals.NVPX - a1;
							}
							v68 = v112;
							_vm->_globals.super_parcours[v68] = v66 + a1;
							_vm->_globals.super_parcours[v68 + 1] = a2;
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
					v70 = Ligne[v115].fieldC;
					v119 = READ_LE_UINT16(v70 + 4 * i);
					v118 = READ_LE_UINT16(v70 + 4 * i + 2);
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
							v74 = Ligne[v110].fieldC;
							v119 = READ_LE_UINT16(v74 + 4 * v72);
							v118 = READ_LE_UINT16(v74 + 4 * v72 + 2);
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
										return (byte *)&_vm->_globals.super_parcours[0];
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
							return (byte *)&_vm->_globals.super_parcours[0];
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
					v98 = 16 * v115;
					do {
						v81 = Ligne[v98].fieldC;
						v119 = READ_LE_UINT16(v81 + 4 * v80);
						v118 = READ_LE_UINT16(v81 + 4 * v80 + 2);
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
							v109 = 16 * v117;
							do {
								v84 = Ligne[v109].fieldC;
								v119 = READ_LE_UINT16(v84 + 4 * v83);
								v118 = READ_LE_UINT16(v84 + 4 * v83 + 2);
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
											return (byte *)&_vm->_globals.super_parcours[0];
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
							return (byte *)&_vm->_globals.super_parcours[0];
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
						v100 = 16 * v121;
						do {
							v95 = Ligne[v100].fieldC;;
							v96 = READ_LE_UINT16(v95 + 4 * v94 + 2);
							v97 = v112;
							_vm->_globals.super_parcours[v97] = READ_LE_UINT16(v95 + 4 * v94);
							_vm->_globals.super_parcours[v97 + 1] = v96;
							_vm->_globals.super_parcours[v97 + 2] = Ligne[v100].field6;
							_vm->_globals.super_parcours[v97 + 3] = 0;
							v112 += 4;
							++v94;
						} while (v120 > v94);
					}
				} else {
					v90 = v114;
					v99 = 16 * v121;
					do {
						v91 = Ligne[v99].fieldC;
						v92 = READ_LE_UINT16(v91 + 4 * v90 + 2);
						v93 = v112;
						_vm->_globals.super_parcours[v93] = READ_LE_UINT16(v91 + 4 * v90);
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
			            _vm->_globals.Param[v112 + 4192 / 2],
			            _vm->_globals.Param[v112 + 4194 / 2],
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
			return (byte *)&_vm->_globals.super_parcours[0];
		}
	}
	return (byte *)&_vm->_globals.super_parcours[0];
}

int LinesManager::PARC_PERS(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
	warning("TODO: PARC_PERS");
	return 0;
}


} // End of namespace Hopkins
