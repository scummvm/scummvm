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

#ifndef ULTIMA8_GUMPS_MOVIEGUMP_H
#define ULTIMA8_GUMPS_MOVIEGUMP_H

#include "ultima/shared/std/string.h"
#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

class RawArchive;
class MoviePlayer;

class MovieGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	MovieGump();
	MovieGump(int width, int height, Common::SeekableReadStream *rs,
			  bool introMusicHack = false, bool noScale = false,
			  const byte *overridePal = nullptr,
	          uint32 flags = FLAG_PREVENT_SAVE, int32 layer = LAYER_MODAL);
	~MovieGump() override;

	void InitGump(Gump *newparent, bool take_focus = true) override;

	void Close(bool no_del = false) override;

	void run() override;

	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	bool OnKeyDown(int key, int mod) override;

	static ProcId U8MovieViewer(Common::SeekableReadStream *rs, bool fade, bool introMusicHack, bool noScale);
	static MovieGump *CruMovieViewer(const Std::string fname, int x, int y, const byte *pal, Gump *parent, uint16 frameshape);

	bool loadData(Common::ReadStream *rs);
	void saveData(Common::WriteStream *ws) override;

	INTRINSIC(I_playMovieOverlay);
	INTRINSIC(I_playMovieCutscene);
	INTRINSIC(I_playMovieCutsceneAlt);
	INTRINSIC(I_playMovieCutsceneRegret);

protected:
	MoviePlayer *_player;

	/// Load subtitles with format detection
	void loadSubtitles(Common::SeekableReadStream *rs);

	/// Load subtitles from a txt file (No Remorse format)
	void loadTXTSubs(Common::SeekableReadStream *rs);

	/// Load subtitles from a iff file (No Regret format)
	void loadIFFSubs(Common::SeekableReadStream *rs);

	/// Update the offset of the player if a shape has been set
	void ClearPlayerOffset();

	/// Subtitles, by frame number.  Only used for Crusader movies.
	Common::HashMap<int, Common::String> _subtitles;

	/// Last widget used for displaying subtitles.
	uint16 _subtitleWidget;

	/// Last frame that was displayed, so we can catch up subtitles.
	int _lastFrameNo;

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
