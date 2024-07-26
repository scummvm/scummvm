
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

#ifndef BAGEL_BAGLIB_FMOVIE_H
#define BAGEL_BAGLIB_FMOVIE_H

#include "video/smk_decoder.h"
#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/gfx/palette.h"

namespace Bagel {

class CBagFMovie : public CBofDialog {
public:
	enum MovieStatus { MOVIE_STOPPED, MOVIE_PAUSED, MOVIE_FORWARD, MOVIE_REVERSE };

protected:
	Video::SmackerDecoder *_smk;

	bool _escCanStopFl;
	bool _loopFl;
	MovieStatus _movieStatus;

	CBofBitmap *_bmpBuf;
	CBofBitmap *_filterBmp;
	CBofPalette *_smackerPal;
	char *_bufferStart;
	int _bufferLength;
	CBofRect _bounds;
	bool _useNewPaletteFl;
	bool _blackOutWindowFl;

	/**
	 * Open the movie file, center it in parent, rewind it, and realize it's
	 * palette in the background
	 */
	virtual ErrorCode initialize(CBofWindow *pParent);

	/**
	 * Open the movie file, center it in parent, rewind it, and realize it's
	 * palette in the background.
	 */
	virtual bool openMovie(const char *sFilename);

	/**
	 * Close the MCI Device file
	 */
	virtual void closeMovie();

	void onReSize(CBofSize *pSize) override {};

	virtual bool play();
	virtual bool reverse();

	void onLButtonUp(uint32 flags, CBofPoint *point, void * = nullptr) override {
		onButtonUp(flags, point);
	}
	virtual void onButtonUp(uint32 flags, CBofPoint *point) {};
	void onPaint(CBofRect *pRect) override;
	virtual void onMovieDone();
	void onClose() override;
	void onMainLoop() override;

	/**
	 * Catch the ESC key when hit
	 */
	void onKeyHit(uint32 keyCode, uint32 repCount) override;

public:
	/**
	 * Initializes Movie Object and opens movie file @ Rect
	 * @param parent       A pointer to the parent window
	 * @param filename     String containing filename of movie to be opened
	 * @param bounds       Location for video object relative to parent
	 * @param useNewPalette
	 * @param blackOutWindow
	 */
	CBagFMovie(CBofWindow *parent = nullptr, const char *filename = nullptr,
	           CBofRect *bounds = nullptr, bool useNewPalette = true, bool blackOutWindow = false);

	/**
	 * Destructor
	 */
	~CBagFMovie();

	/**
	 * Open the movie file, place it @ bounds, rewind it, and realize it's
	 * palette in the background.
	 */
	virtual bool open(const char *filename = nullptr, CBofRect *bounds = nullptr);

	virtual bool play(bool loop, bool escCanStop = true);
	virtual bool reverse(bool loop, bool escCanStop = true);
	virtual bool pause();
	virtual bool stop();

	virtual MovieStatus status() {
		return _movieStatus;
	}

	virtual bool seekToStart();
	virtual bool seekToEnd();

	virtual uint32 getFrame();
	virtual bool setFrame(uint32 frameNum);

	virtual bool centerRect();
};

} // namespace Bagel

#endif
