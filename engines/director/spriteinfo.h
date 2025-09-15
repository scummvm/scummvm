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

#ifndef DIRECTOR_SPRITEINFO_H
#define DIRECTOR_SPRITEINFO_H

namespace Director {

struct TweenInfo {
	int32 curvature;
	int32 flags;
	int32 easeIn;
	int32 easeOut;
	int32 padding;

	void read(Common::ReadStreamEndian &stream) {
		curvature = (int32)stream.readUint32();
		flags = (int32)stream.readUint32();
		easeIn = (int32)stream.readUint32();
		easeOut = (int32)stream.readUint32();
		padding = (int32)stream.readUint32();
	}
};

struct SpriteInfo {
	int32 startFrame;
	int32 endFrame;
	int32 xtraInfo;
	int32 flags;
	int32 channelNum;
	TweenInfo tweenInfo;

    Common::Array<int32> keyFrames;

	Common::String name; // Sits in a separate item

	void read(Common::ReadStreamEndian &stream) {
		startFrame = (int32)stream.readUint32();
		endFrame = (int32)stream.readUint32();
		xtraInfo = (int32)stream.readUint32();
		flags = (int32)stream.readUint32();
		channelNum = (int32)stream.readUint32();
		tweenInfo.read(stream);

		keyFrames.clear();
		while (!stream.eos()) {
			int32 frame = (int32)stream.readUint32();
			if (stream.eos())
				break;
			keyFrames.push_back(frame);
		}
	}

	Common::String toString() const {
		Common::String s;
		s += Common::String::format("startFrame: %d, endFrame: %d, xtraInfo: %d, flags: 0x%x, channelNum: %d\n",
			startFrame, endFrame, xtraInfo, flags, channelNum);
		s += Common::String::format("    tweenInfo: curvature: %d, flags: 0x%x, easeIn: %d, easeOut: %d\n",
			tweenInfo.curvature, tweenInfo.flags, tweenInfo.easeIn, tweenInfo.easeOut);
		s += Common::String::format("    name: '%s'\n", name.c_str());
		s += "    keyFrames: ";
		for (size_t i = 0; i < keyFrames.size(); i++) {
			s += Common::String::format("%d ", keyFrames[i]);
		}
		return s;
	}
};

struct BehaviorElement {
	CastMemberID memberID;
	int32 initializerIndex = 0;
	Common::String initializerParams;

	void read(Common::ReadStreamEndian &stream) {
		memberID.castLib = (int16)stream.readUint16();
		memberID.member = (int16)stream.readUint16();
		initializerIndex = (int32)stream.readUint32();
	}

	Common::String toString() const {
		Common::String s;
		s += Common::String::format("memberID: %s, initializerIndex: %d, initializerParams: '%s'",
			memberID.asString().c_str(), initializerIndex, initializerParams.c_str());
		return s;
	}
};

} // End of namespace Director

#endif
