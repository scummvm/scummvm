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

#ifndef ULTIMA0_SDW_H
#define ULTIMA0_SDW_H

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"

namespace Ultima {
namespace Ultima0 {

extern Graphics::Screen *Display;		// The Display screen
extern int GameSpeed;					// Scales the system clock.

// It's all nicely in a namespace SDLWrapper
namespace SDLWrapper {

#define DEFAULT_SCBPP	(0)
#define MAXSOUND        (16)                    // Maximum number of sounds

#define SDWASSERT(x)	if (!(x)) SDWERROR()	// ASSERT and ERROR macros
#define SDWERROR()	FatalError(__LINE__,__FILE__)

extern void SetSpeed(int n);
extern void FatalError(int line, const char *filename);    // Prototyping

extern int GameClock(void);				        // No point in wrapping this, it just goes to SDL_GetTicks()
extern int SystemClock(void);                   // This one is unaffected by the game speed.

extern int ReadStick(int &A, int &B, int &dx, int &dy);   // Read a joystick
extern int MouseClick(int &x, int &y);			// Read a mouse select - and - click
extern int ExitKey(void);
extern int GetKey(void);

struct SDL_Rect {
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	operator const Common::Rect() const {
		return Common::Rect(x, y, x + w, y + h);
	}
};

typedef Graphics::ManagedSurface SDL_Surface;

class Rect {
public:
	Rect() {}							    // Constructors
	Rect(int x1, int y1, int x2, int y2) {
		Left = x1; Top = y1; Right = x2; Bottom = y2;
	}
	int Left, Top, Right, Bottom;				// The rectangle
};

// A basic Surface
class Surface {
public:
	Surface(int x = 0, int y = 0, int Trans = 0, int UseDisplay = 0, const char *File = nullptr);
	~Surface();

	void SetColour(int r, int g, int b);
	void SetColour() {
		SetColour(-1, -1, -1);
	}
	void SetColour(int Col) {
		Colour = Col;
	}
	unsigned int GetColour(void) {
		return Colour;
	}
	int  Width(void) {
		return xSize;
	}
	int  Height(void) {
		return ySize;
	}
	void SetOrigin(int x = 0, int y = 0) {
		xOrigin = x; yOrigin = y;
	}
	void SetScale(int x = 256, int y = 256) {
		xScale = x; yScale = y;
	}

	void Plot(int x1, int y1);
	void FillRect(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);
	void FillRect(Rect &r) {
		FillRect(r.Left, r.Top, r.Right, r.Bottom);
	}
	void FrameRect(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);
	void FrameRect(Rect &r) {
		FrameRect(r.Left, r.Top, r.Right, r.Bottom);
	}
	void FillEllipse(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);
	void FillEllipse(Rect &r) {
		FillEllipse(r.Left, r.Top, r.Right, r.Bottom);
	}
	void FrameEllipse(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);
	void FrameEllipse(Rect &r) {
		FrameEllipse(r.Left, r.Top, r.Right, r.Bottom);
	}
	void Line(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);

	void Copy(Surface &Target, Rect &SrcRect, int x = 0, int y = 0);
	void Copy(Rect &SrcRect, int x = 0, int y = 0);
	void Copy(Surface &Target, int x = 0, int y = 0);
	void Copy(int x = 0, int y = 0);

	void HorizontalMirror(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);
	void VerticalMirror(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);

	void Char(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0, char c = ' ');
	void Char(Rect &r, char c) {
		Char(r.Left, r.Top, r.Right, r.Bottom, c);
	}
	void String(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0, const char *s = "");
	void String(Rect &r, char *s) {
		String(r.Left, r.Top, r.Right, r.Bottom, s);
	}

	void Flip(void);


protected:
	void SortAndValidate(int &x1, int &y1, int &x2, int &y2);
	void PointProcess(int &x1, int &y1);

private:
	SDL_Surface *sSurface;
	int xSize, ySize;						// Surface size (physical)
	unsigned int Colour;					// Drawing colour
	int IsTransparent;						// Set if transparent
	unsigned int TransColour;				// Transparency drawing colour
	int IsDisplay;							// Set if is the physical display object
	int xOrigin, yOrigin;					// Mobile origin and scaling
	int xScale, yScale;
};

// A surface but with transparency
class TransparentSurface : public Surface {
public:
	TransparentSurface(int x = 0, int y = 0) : Surface(x, y, 1, 0, nullptr) {
	}
};

// A surface with a bitmap on it, one solid, one transparent
class BitmapSurface : public Surface {
public:
	BitmapSurface(const char *File) : Surface(0, 0, 0, 0, File) {
	}
};

class TransparentBitmapSurface : public Surface {
public:
	TransparentBitmapSurface(const char *File) : Surface(0, 0, 1, 0, File) {
	}
};

// The actual physical display
class DisplaySurface : public Surface {
public:
	DisplaySurface(int x = 0, int y = 0) : Surface(0, 0, 0, 1, nullptr) {
	}
};

// A simple timer
class Timer {
public:
	Timer(int TimeOut = 0);
	void ResetTimer(int t = 0);
	unsigned int Elapsed(void);
	int TimedOut(void);
	void WaitTimer(void);

private:
	int StartClock;
	int EndClock;
	int EventTime;
};

// An audio object
class AudioObject {
public:
	AudioObject() {
		Data = nullptr; Position = Length = 0; Attach(); SoundOn = 0; LoopSound = 0;
	}
	~AudioObject() {
		Detach(); if (Data != nullptr) free(Data);
	}
	void CopyStream(void *Stream, int Reqd);
	void Play(void) {
		Position = 0; SoundOn = 1;
	}
	void PlayLoop(void) {
		Position = 0; SoundOn = 1; LoopSound = 1;
	}
	void Stop(void) {
		SoundOn = 0; LoopSound = 0;
	}
	int  Size(void) {
		return Length / 2;
	}
	void Write(int Pos, int Dat);
protected:
	void Attach(void);
	void Detach(void);
	void *Data;
	int Position;
	int Length;
	int SoundOn;
	int LoopSound;
};

class AudioWave : public AudioObject {
private:
	Audio::SoundHandle _soundHandle;
	Audio::AudioStream *_stream;

public:
	AudioWave(const char *File) : AudioObject() {
		Load(File);
	}
protected:
	void Load(const char *File);
};

class AudioBeep : public AudioObject {
public:
	AudioBeep(int p, int l) : AudioObject() {
		CreateBeep(p, l);
	}
protected:
	void CreateWave(void *Data, int Size, int sPitch);
	void CreateBeep(int sPitch, int sLength);
};

class AudioNoise : public AudioBeep {
public:
	AudioNoise(int l) : AudioBeep(0, l) {
	}
};

inline void SDL_FillRect(SDL_Surface *surf, const SDL_Rect *rect, uint Color) {
	surf->fillRect(*rect, Color);
}

inline void SDL_BlitSurface(SDL_Surface *srcSurf, const SDL_Rect *srcRect,
		SDL_Surface *destSurf, const SDL_Rect *destRect) {
	destSurf->blitFrom(*srcSurf, *srcRect, *destRect);
}

} // namespace SDLWrapper
} // namespace Ultima0
} // namespace Ultima

#endif
