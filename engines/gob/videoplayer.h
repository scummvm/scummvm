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
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_VIDEOPLAYER_H
#define GOB_VIDEOPLAYER_H

#include "common/array.h"
#include "common/str.h"

#include "graphics/surface.h"
#include "graphics/video/coktel_decoder.h"

#include "gob/util.h"
#include "gob/draw.h"

namespace Gob {

class GobEngine;
class DataStream;

class VideoPlayer {
public:
	enum Flags {
		kFlagNone                  = 0x000000,
		kFlagUseBackSurfaceContent = 0x000040, ///< Use the back surface as a video "base".
		kFlagFrontSurface          = 0x000080, ///< Draw directly into the front surface.
		kFlagNoVideo               = 0x000100, ///< Only sound.
		kFlagOtherSurface          = 0x000800, ///< Draw into a specific sprite.
		kFlagScreenSurface         = 0x400000  ///< Draw into a newly created sprite of screen dimensions.
	};

	/** Video format. */
	enum Type {
		kVideoTypeTry    = -1, ///< Try any format.
		kVideoTypeIMD    =  0,
		kVideoTypePreIMD =  1, ///< Early IMD format found in Fascination.
		kVideoTypeVMD    =  2,
		kVideoTypeRMD    =  3  ///< VMD containing "reversed" video.
	};

	struct Properties {
		Type type; ///< Type of the video to open.

		int sprite; ///< The sprite onto which to draw the video.

		int32 x;      ///< X coordinate of the video.
		int32 y;      ///< Y coordinate of the video.
		int32 width;  ///< Width of the video.
		int32 height; ///< Height of the video.

		uint32 flags; ///< Video flags.

		int32 startFrame; ///< Frame to start playback from.
		int32 lastFrame;  ///< Frame to stop playback at.
		int32 endFrame;   ///< Last frame of this playback cycle.

		int16 breakKey; ///< Keycode of the break/abort key.

		uint16 palCmd;      ///< Palette command.
		 int16 palStart;    ///< Palette entry to start with.
		 int16 palEnd;      ///< Palette entry to end at.
		 int32 palFrame;    ///< Frame to apply the palette command at.

		bool fade; ///< Fade in?

		bool canceled; ///< Was the video canceled?

		Properties();
	};

	VideoPlayer(GobEngine *vm);
	~VideoPlayer();

	void evaluateFlags(Properties &properties);

	int  openVideo(bool primary, const Common::String &file, Properties &properties);
	bool closeVideo(int slot = 0);

	bool play(int slot, Properties &properties);

	bool slotIsOpen(int slot = 0) const;

	Common::String getFileName(int slot = 0) const;

	uint32 getFrameCount  (int slot = 0) const;
	uint32 getCurrentFrame(int slot = 0) const;
	uint16 getWidth       (int slot = 0) const;
	uint16 getHeight      (int slot = 0) const;
	uint16 getDefaultX    (int slot = 0) const;
	uint16 getDefaultY    (int slot = 0) const;

	bool                      hasExtraData(const Common::String &fileName, int slot = 0) const;
	Common::MemoryReadStream *getExtraData(const Common::String &fileName, int slot = 0);

	void writeVideoInfo(const Common::String &file, int16 varX, int16 varY,
			int16 varFrames, int16 varWidth, int16 varHeight);



	// Obsolete, to be deleted

	bool primaryOpen(const char *videoFile, int16 x = -1, int16 y = -1,
			int32 flags = kFlagFrontSurface, Type which = kVideoTypeTry,
			int16 width = -1, int16 height = -1);
	bool primaryPlay(int16 startFrame = -1, int16 lastFrame = -1,
			int16 breakKey = kShortKeyEscape,
			uint16 palCmd = 8, int16 palStart = 0, int16 palEnd = 255,
			int16 palFrame = -1, int16 endFrame = -1, bool fade = false,
			int16 reverseTo = -1, bool forceSeek = false);
	void primaryClose();

	int slotOpen(const char *videoFile, Type which = kVideoTypeTry,
			int16 width = -1, int16 height = -1);
	void slotPlay(int slot, int16 frame = -1);
	void slotClose(int slot);
	void slotCopyFrame(int slot, byte *dest,
			uint16 left, uint16 top, uint16 width, uint16 height,
			uint16 x, uint16 y, uint16 pitch, int16 transp = -1);
	void slotCopyPalette(int slot, int16 palStart = -1, int16 palEnd = -1);
	void slotWaitEndFrame(int slot = -1, bool onlySound = false);

	void slotSetDoubleMode(int slot, bool doubleMode);


private:
	struct Video {
		Graphics::CoktelDecoder *decoder;
		Common::String fileName;

		SurfaceDescPtr surface;

		Video();

		bool isEmpty() const;
		void close();
	};

	static const int kVideoSlotCount = 32;

	static const char *_extensions[];

	GobEngine *_vm;

	// _videoSlots[0] is reserved for the "primary" video
	Video _videoSlots[kVideoSlotCount];

	bool _needBlit;

	bool _noCursorSwitch;
	bool _woodruffCohCottWorkaround;

	const Video *getVideoBySlot(int slot) const;
	Video *getVideoBySlot(int slot);

	int getNextFreeSlot();

	Common::String findFile(const Common::String &file, Properties &properties);

	Graphics::CoktelDecoder *openVideo(const Common::String &file, Properties &properties);

	bool playFrame(int slot, Properties &properties);
	void blitFrame(SurfaceDescPtr dst, const Graphics::Surface &src);

	void checkAbort(Video &video, Properties &properties);
	void evalBgShading(Video &video);

	void copyPalette(const Video &video, int16 palStart, int16 palEnd);
};

} // End of namespace Gob

#endif // GOB_VIDEOPLAYER_H
