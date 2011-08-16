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

#if !defined(SCUMM_HE_LOGIC_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_LOGIC_HE_H

namespace Scumm {

class ScummEngine_v90he;

class LogicHE {
public:
	ScummEngine_v90he *_vm;

	LogicHE(ScummEngine_v90he *vm);
	virtual ~LogicHE();

	void writeScummVar(int var, int32 value);
	int getFromArray(int arg0, int idx2, int idx1);
	void putInArray(int arg0, int idx2, int idx1, int val);

	virtual void beforeBootScript() {}
	virtual void initOnce() {}
	virtual int startOfFrame() { return 1; }
	void endOfFrame() {}
	void processKeyStroke(int keyPressed) {}

	virtual int versionID();
	virtual int32 dispatch(int op, int numArgs, int32 *args);
};

class LogicHErace : public LogicHE {
private:
	float *_userData;
	double *_userDataD;
public:
	LogicHErace(ScummEngine_v90he *vm);
	~LogicHErace();

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

private:
	int32 op_1003(int32 *args);
	int32 op_1004(int32 *args);
	int32 op_1100(int32 *args);
	int32 op_1101(int32 *args);
	int32 op_1102(int32 *args);
	int32 op_1103(int32 *args);
	int32 op_1110();
	int32 op_1120(int32 *args);
	int32 op_1130(int32 *args);
	int32 op_1140(int32 *args);

	void op_sub1(float arg);
	void op_sub2(float arg);
	void op_sub3(float arg);
};

class LogicHEfunshop : public LogicHE {
public:
	LogicHEfunshop(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

private:
	void op_1004(int32 *args);
	void op_1005(int32 *args);
	int checkShape(int32 data0, int32 data1, int32 data4, int32 data5, int32 data2, int32 data3, int32 data6, int32 data7, int32 *x, int32 *y);
};

class LogicHEfootball : public LogicHE {
public:
	LogicHEfootball(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

private:
	int op_1004(int32 *args);
	int op_1006(int32 *args);
	int op_1007(int32 *args);
	int op_1010(int32 *args);
	int op_1022(int32 *args);
	int op_1023(int32 *args);
	int op_1024(int32 *args);
};

class LogicHEsoccer : public LogicHE {
private:
	double *_userDataD;

public:
	LogicHEsoccer(ScummEngine_v90he *vm);
	~LogicHEsoccer();

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

	void beforeBootScript();
	void initOnce();
	int startOfFrame();

private:
	int op_1005(float x1, float y1, float z1, float x2, float y2, float z2, float *nextVelX, float *nextVelY, float *nextVelZ, float *a10);
	int op_1006(int32 a1, int32 a2, int32 a3, int32 a4);
	int op_1007(int32 *args);
	int op_1008(int outArray, int srcX, int srcY, int srcZ, int vecX, int vecY, int vecZ, int airResX, int airResY, int airResZ, int vecNumerator, int vecDenom, int gravityMult, int requiredSegments, int a15, int a16, int a17, int a18, int fieldType);
	int op_1011(int32 worldPosArray, int32 screenPosArray, int32 a3, int32 closestActorArray, int32 maxDistance, int32 fieldAreaArray);
	int op_1012(int32 *args);
	int op_1013(int32 a1, int32 a2, int32 a3);
	int op_1014(int32 srcX, int32 srcY, int32 srcZ, int32 velX, int32 velY, int32 velZ, int32 outArray, int32 dataArrayId, int32 indexArrayId, int32 requestType, int32 vecNumerator, int32 vecDenom, int32 a13, int32 a14);
	int op_1016(int32 *args);
	int op_1017(int32 *args);
	int op_1019(int32 *args);
	int op_1021(int32 inX, int32 inY, int32 inZ, int32 velX, int32 velY, int32 velZ, int32 internalUse);

	// op_1007 allocates some arrays
	// they're then filled by op_1019
	byte _collisionObjIds[4096], _collisionNodeEnabled[585];

	// op_1011 has a subfunction
	void calculateDistances(int32 worldPosArray, int32 a2, int32 closestActorArray, int32 maxDistance);

	// array containing collision detection tree
	bool _collisionTreeAllocated;
	uint32 *_collisionTree;
	int addCollisionTreeChild(int depth, int index, int parent);

	// op_1014 has several subops
	// ...and several sub-subops
	int generateCollisionObjectList(float srcX, float srcY, float srcZ, float velX, float velY, float velZ);
	int addFromCollisionTreeNode(int index, int parent, uint32 *indices, int objIndexBase);
	void addCollisionObj(byte objId);
	int findCollisionWith(int objId, float inX, float inY, float inZ, float inXVec, float inYVec, float inZVec, float &collideX, float &collideY, float &collideZ, int indexArrayId, int dataArrayId, float *nextVelX, float *nextVelY, float *nextVelZ, float *a15);
	void getPointsForFace(int faceId, float &x1, float &y1, float &z1, float &x2, float &y2, float &z2, float &x3, float &y3, float &z3, float &x4, float &y4, float &z4, const int *objPoints);
	void crossProduct(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, float &outX, float &outY, float &outZ);
	double dotProduct(float a1, float a2, float a3, float a4, float a5, float a6);
	void sortCollisionList(float *data, int numEntries, int entrySize, int compareOn);
	int setCollisionOutputData(float *collisionData, int entrySize, int dataArrayId, int indexArrayId, int startX, int startY, int startZ, float a8, int a9, int a10, int a11, int *out);

	// op_1014 sets an array optionally based upon
	// setCollisionOutputData; it is then used by op_1008
	int _internalCollisionOutData[10];
	Common::List<byte> _collisionObjs;

	// op_1021 can (optionally) set two variables for use in op_1008
	uint32 _var1021[2];
};

class LogicHEbaseball2001 : public LogicHE {
public:
	LogicHEbaseball2001(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);
};

class LogicHEbasketball : public LogicHE {
public:
	LogicHEbasketball(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);
};

class LogicHEmoonbase : public LogicHE {
public:
	LogicHEmoonbase(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
};

} // End of namespace Scumm

#endif
