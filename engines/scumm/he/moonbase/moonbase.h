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

#ifndef SCUMM_HE_MOONBASE_MOONBASE_H
#define SCUMM_HE_MOONBASE_MOONBASE_H

#ifdef ENABLE_HE

namespace Common {
class PEResources;
}

namespace Scumm {

class AI;
class Net;

class Moonbase {
public:
	Moonbase(ScummEngine_v100he *vm);
	~Moonbase();

	int readFromArray(int array, int y, int x);
	void deallocateArray(int array);
	int callScummFunction(int scriptNumber, int paramCount,...);

	void blitT14WizImage(uint8 *dst, int dstw, int dsth, int dstPitch, const Common::Rect *clipBox,
			 uint8 *wizd, int srcx, int srcy, int rawROP, int paramROP);
	void blitDistortion(byte *bufferData, const int bufferWidth, const int bufferHeight, const int bufferPitch,
			const Common::Rect *optionalClippingRect, byte *dataStream, const int x, const int y, byte *altSourceBuffer);

	// FOW Stuff
	bool isFOW(int resNum, int state, uint32 conditionBits) {
		return resNum == _fowSentinelImage && state == _fowSentinelState && conditionBits == _fowSentinelConditionBits;
	}

	void initFOW();
	void releaseFOWResources();

	bool setFOWImage(int id);

	void setFOWInfo(int fowInfoArray, int downDim, int acrossDim, int viewX, int viewY, int clipX1,
		int clipY1, int clipX2, int clipY2, int technique, int nFrame);


	void renderFOW(uint8 *destSurface, int dstPitch, int dstType, int dstw, int dsth, int flags);

private:
	int readFOWVisibilityArray(int array, int y, int x);
	void renderFOWState(uint8 *destSurface, int dstPitch, int dstType, int dstw, int dsth, int x, int y, int srcw, int srch, int state, int flags);

public:
	int _fowSentinelImage;
	int _fowSentinelState;
	uint32 _fowSentinelConditionBits;

	AI *_ai;
#ifdef USE_LIBCURL
	Net *_net;
#endif

private:
	ScummEngine_v100he *_vm;

	int _fowFrameBaseNumber;
	int _fowAnimationFrames;
	int _fowCurrentFOWFrame;

	int32 _fowTileW;
	int32 _fowTileH;

	uint8 *_fowImage;
	int _fowClipX1;
	int _fowClipY1;
	int _fowClipX2;
	int _fowClipY2;

	int _fowDrawX;
	int _fowDrawY;

	int _fowVtx1;
	int _fowVty1;
	int _fowMvx;
	int _fowMvy;
	int _fowVw;
	int _fowVh;

	bool _fowBlackMode;

	int32 _fowRenderTable[32768];

	Common::PEResources *_exe;
	Common::String _fileName;
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_MOONBASE_H
