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
#include "allfiles.h"
#include "objtypes.h"
#include "variable.h"
#include "newfatal.h"
#include "moreio.h"
#include "fileset.h"
#include "CommonCode/version.h"

namespace Sludge {

objectType *allObjectTypes = NULL;
extern char *outputDir;

#define DEBUG_COMBINATIONS  0

bool initObjectTypes() {
	return true;
}

objectType *findObjectType(int i) {
	objectType *huntType = allObjectTypes;

	while (huntType) {
		if (huntType -> objectNum == i) return huntType;
		huntType = huntType -> next;
	}

	return loadObjectType(i);
}

objectType *loadObjectType(int i) {
	int a, nameNum;
	objectType *newType = new objectType;

	if (checkNew(newType)) {
		if (openObjectSlice(i)) {
			nameNum = get2bytes(bigDataFile);
			newType -> r = (byte) getch(bigDataFile);
			newType -> g = (byte) getch(bigDataFile);
			newType -> b = (byte) getch(bigDataFile);
			newType -> speechGap = getch(bigDataFile);
			newType -> walkSpeed = getch(bigDataFile);
			newType -> wrapSpeech = get4bytes(bigDataFile);
			newType -> spinSpeed = get2bytes(bigDataFile);

			if (gameVersion >= VERSION(1, 6)) {
				// aaLoad
				getch(bigDataFile);
				getFloat(bigDataFile);
				getFloat(bigDataFile);
			}

			if (gameVersion >= VERSION(1, 4)) {
				newType -> flags = get2bytes(bigDataFile);
			} else {
				newType -> flags = 0;
			}

			newType -> numCom = get2bytes(bigDataFile);
			newType -> allCombis = (newType -> numCom) ? new combination[newType -> numCom] : NULL;

#if DEBUG_COMBINATIONS
			FILE *callEventLog = fopen("callEventLog.txt", "at");
			if (callEventLog) {
				fprintf(callEventLog, "Object type %d has %d combinations... ", i, newType -> numCom);
			}
#endif

			for (a = 0; a < newType -> numCom; a ++) {
				newType -> allCombis[a].withObj = get2bytes(bigDataFile);
				newType -> allCombis[a].funcNum = get2bytes(bigDataFile);
#if DEBUG_COMBINATIONS
				if (callEventLog) {
					fprintf(callEventLog, "%d(%d) ", newType -> allCombis[a].withObj, newType -> allCombis[a].funcNum);
				}
#endif
			}
#if DEBUG_COMBINATIONS
			if (callEventLog) {
				fprintf(callEventLog, "\n");
				fclose(callEventLog);
			}
#endif
			finishAccess();
			newType -> screenName = getNumberedString(nameNum);
			newType -> objectNum = i;
			newType -> next = allObjectTypes;
			allObjectTypes = newType;
			return newType;
		}
	}

	return NULL;
}

objectType *loadObjectRef(Common::SeekableReadStream *stream) {
	objectType *r = loadObjectType(get2bytes(stream));
	delete r -> screenName;
	r -> screenName = readString(stream);
	return r;
}

void saveObjectRef(objectType *r, Common::WriteStream *stream) {
	put2bytes(r -> objectNum, stream);
	writeString(r -> screenName, stream);
}

int getCombinationFunction(int withThis, int thisObject) {
	int i, num = 0;
	objectType *obj = findObjectType(thisObject);

#if DEBUG_COMBINATIONS
	FILE *callEventLog = fopen("callEventLog.txt", "at");
	if (callEventLog) {
		fprintf(callEventLog, "Combining %d and %d - ", thisObject, withThis);
	}
#endif

	for (i = 0; i < obj -> numCom; i ++) {
		if (obj -> allCombis[i].withObj == withThis) {
			num = obj -> allCombis[i].funcNum;
			break;
		}
	}

#if DEBUG_COMBINATIONS
	if (callEventLog) {
		fprintf(callEventLog, "got function number %d\n", num);
		fclose(callEventLog);
	}
#endif

	return num;
}

void removeObjectType(objectType *oT) {
	objectType * * huntRegion = & allObjectTypes;

	while (* huntRegion) {
		if ((* huntRegion) == oT) {
//			FILE * debuggy2 = fopen ("debug.txt", "at");
//			fprintf (debuggy2, "DELETING OBJECT TYPE: %p %s\n", oT, oT -> screenName);
//			fclose (debuggy2);

			* huntRegion = oT -> next;
			delete oT -> allCombis;
			delete oT -> screenName;
			delete oT;
			return;
		} else {
			huntRegion = & ((* huntRegion) -> next);
		}
	}
	fatal("Can't delete object type: bad pointer");
}

} // End of namespace Sludge
