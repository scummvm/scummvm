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

#include "common/algorithm.h"
#include "m4/graphics/rend.h"
#include "m4/core/errors.h"
#include "m4/core/term.h"
#include "m4/vars.h"

namespace M4 {

#define _GR(X) _G(rend)._##X
#define DEPTH_MASK  0x0f
#define OPTIMIZE_CLIP

constexpr int32 kShadowFlag = 0x80;

enum {
	kDepthed = 0x0001,
	kScaled = 0x0002,
	kClipped = 0x0004
};

static RenderResult skip_RLE8_line() {
	uint8 *from = _GR(sourceAddress);

	for (;;) {
		if (*from++)
			++from;
		else if (*from++ <= kJumpXY) {
			_GR(sourceAddress) = from;
			return (RenderResult)(*(from - 1));
		} else
			from += *(from - 1);
	}
}

#define EMPIRICALLY_FOUND       5

static RenderResult r_0() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);

	for (;;) {
		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				if (data)
					*to = data;
				++to;
			}
		} else {
			uint8 data = *from++;

			if (!data)
				to += runLength;
			else {
				if (runLength > EMPIRICALLY_FOUND) {
					while ((uintptr)to & 3) {
						*to++ = data;
						--runLength;
					}

					uint32 ddata = data | ((uint32)data << 8);
					ddata = ddata | (ddata << 16);

					uint32 *dto = (uint32 *)to;

					uint32 longRunLength = runLength >> 2;

					runLength -= longRunLength * 4;

					while (longRunLength--)
						*dto++ = ddata;

					to = (uint8 *)dto;
				}

				while (runLength--)
					*to++ = data;
			}
		}
	}
}

static RenderResult r_0_5() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);

	for (;;) {
		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				if (data)
					*to = data;
				--to;
			}
		} else {
			uint8 data = *from++;

			if (!data)
				to -= runLength;
			else {
				if (runLength > EMPIRICALLY_FOUND) {
					while (((uintptr)to & 3) != 3) {
						*to-- = data;
						--runLength;
					}

					uint32 ddata = data | ((uint32)data << 8);
					ddata = ddata | (ddata << 16);

					uint32 *dto = (uint32 *)(to - 3);

					uint32 longRunLength = runLength >> 2;

					runLength -= longRunLength * 4;

					while (longRunLength--)
						*dto-- = ddata;

					to = (uint8 *)dto + 3;
				}

				while (runLength--)
					*to-- = data;
			}
		}
	}
}

static RenderResult r_1() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	uint8 *depth = _GR(depthAddress);
	uint8 sdepth = _GR(spriteDepth);
	long Inc = _GR(Increment);

	int32 myWidth = 0;

	for (;;) {
		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				++myWidth;

				uint8 data = *from++;
				uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

				if (data && (!ddepth || sdepth < ddepth))
					*to = data;
				to += Inc;
				depth += Inc;
			}
		} else {
			uint8 data = *from++;

			myWidth += runLength;

			if (!data) {
				if (Inc == 1) {
					to += runLength;
					depth += runLength;
				} else {
					to -= runLength;
					depth -= runLength;
				}
			} else {
				while (runLength--) {
					uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

					if (!ddepth || sdepth < ddepth)
						*to = data;
					to += Inc;
					depth += Inc;
				}
			}
		}

		if (myWidth > 666) {
			error_show(FL, 0);
		}
	}
}

static RenderResult r_2() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	int32 Error = _GR(X_error);
	uint32 scaleX = _GR(X_scale);
	long Inc = _GR(Increment);

	for (;;) {
		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				Error -= scaleX;

				while (Error < 0) {
					if (data)
						*to = data;

					to += Inc;

					Error += 100;
				}
			}
		} else {
			uint8 data = *from++;

			while (runLength--) {
				Error -= scaleX;

				while (Error < 0) {
					if (data)
						*to = data;

					to += Inc;

					Error += 100;
				}
			}
		}
	}
}

static RenderResult r_3() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	uint8 *depth = _GR(depthAddress);
	uint8 sdepth = _GR(spriteDepth);
	int32 Error = _GR(X_error);
	uint32 scaleX = _GR(X_scale);
	long Inc = _GR(Increment);

	for (;;) {
		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				Error -= scaleX;

				while (Error < 0) {
					uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

					if (data && (!ddepth || sdepth < ddepth))
						*to = data;

					to += Inc;
					depth += Inc;

					Error += 100;
				}
			}
		} else {
			uint8 data = *from++;

			while (runLength--) {
				Error -= scaleX;

				while (Error < 0) {
					uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

					if (data && (!ddepth || sdepth < ddepth))
						*to = data;

					to += Inc;
					depth += Inc;

					Error += 100;
				}
			}
		}
	}
}

static RenderResult r_4() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	int32 PixelPos = _GR(StartingPixelPos);
	long Inc = _GR(Increment);

	for (;;) {
#ifdef OPTIMIZE_CLIP
		if (Inc == 1) {
			if (PixelPos >= _GR(RightPorch)) {
				_GR(sourceAddress) = from;
				return skip_RLE8_line();
			}
		} else if (PixelPos < _GR(LeftPorch)) {
			_GR(sourceAddress) = from;
			return skip_RLE8_line();
		}
#endif

		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				if (data && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch))
					*to = data;
				to += Inc;
				PixelPos += Inc;
			}
		} else {
			uint8 data = *from++;

			if (!data) {
				if (Inc == 1) {
					to += runLength;
					PixelPos += runLength;
				} else {
					to -= runLength;
					PixelPos -= runLength;
				}
			} else while (runLength--) {
				if (PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch))
					*to = data;
				to += Inc;
				PixelPos += Inc;
			}
		}
	}
}

static RenderResult r_5() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	uint8 *depth = _GR(depthAddress);
	uint8 sdepth = _GR(spriteDepth);
	int32 PixelPos = _GR(StartingPixelPos);
	long Inc = _GR(Increment);

	for (;;) {
#ifdef OPTIMIZE_CLIP
		if (Inc == 1) {
			if (PixelPos >= _GR(RightPorch)) {
				_GR(sourceAddress) = from;
				return skip_RLE8_line();
			}
		} else if (PixelPos < _GR(LeftPorch)) {
			_GR(sourceAddress) = from;
			return skip_RLE8_line();
		}
#endif

		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;
				uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

				if (data && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch) && (!ddepth || sdepth < ddepth))
					*to = data;
				to += Inc;
				depth += Inc;
				PixelPos += Inc;
			}
		} else {
			uint8 data = *from++;

			if (!data)
				if (Inc == 1) {
					to += runLength;
					depth += runLength;
					PixelPos += runLength;
				} else {
					to -= runLength;
					depth -= runLength;
					PixelPos -= runLength;
				} else while (runLength--) {
					uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

					if (PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch) && (!ddepth || sdepth < ddepth))
						*to = data;
					to += Inc;
					depth += Inc;
					PixelPos += Inc;
				}
		}
	}
}

static RenderResult r_6() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	int32 PixelPos = _GR(StartingPixelPos), Error = _GR(X_error);
	uint32 scaleX = _GR(X_scale);
	long Inc = _GR(Increment);

	for (;;) {
#ifdef OPTIMIZE_CLIP
		if (Inc == 1) {
			if (PixelPos >= _GR(RightPorch)) {
				_GR(sourceAddress) = from;
				return skip_RLE8_line();
			}
		} else if (PixelPos < _GR(LeftPorch)) {
			_GR(sourceAddress) = from;
			return skip_RLE8_line();
		}
#endif

		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				Error -= scaleX;

				while (Error < 0) {
					if (data && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch))
						*to = data;
					to += Inc;
					PixelPos += Inc;

					Error += 100;
				}
			}
		} else {
			uint8 data = *from++;

			while (runLength--) {
				Error -= scaleX;

				while (Error < 0) {
					if (data && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch))
						*to = data;
					to += Inc;
					PixelPos += Inc;

					Error += 100;
				}
			}
		}
	}
}

static RenderResult r_7() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	uint8 *depth = _GR(depthAddress);
	uint8 sdepth = _GR(spriteDepth);
	int32 PixelPos = _GR(StartingPixelPos), Error = _GR(X_error);
	uint32 scaleX = _GR(X_scale);
	long Inc = _GR(Increment);

	for (;;) {
#ifdef OPTIMIZE_CLIP
		if (Inc == 1) {
			if (PixelPos >= _GR(RightPorch)) {
				_GR(sourceAddress) = from;
				return skip_RLE8_line();
			}
		} else if (PixelPos < _GR(LeftPorch)) {
			_GR(sourceAddress) = from;
			return skip_RLE8_line();
		}
#endif

		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				Error -= scaleX;

				while (Error < 0) {
					uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

					if (data && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch) && (!ddepth || sdepth < ddepth))
						*to = data;
					to += Inc;
					depth += Inc;
					PixelPos += Inc;

					Error += 100;
				}
			}
		} else {
			uint8 data = *from++;

			while (runLength--) {
				Error -= scaleX;

				while (Error < 0) {
					uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

					if (data && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch) && (!ddepth || sdepth < ddepth))
						*to = data;
					to += Inc;
					depth += Inc;
					PixelPos += Inc;

					Error += 100;
				}
			}
		}
	}
}

#define PALETTE_SHIFTER     10

static RenderResult r_8() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	int32 PixelPos = _GR(StartingPixelPos), Error = _GR(X_error);
	uint32 scaleX = _GR(X_scale);
	long Inc = _GR(Increment);
	uint32 r, g, b;

	for (;;) {
#ifdef OPTIMIZE_CLIP
		if (Inc == 1) {
			if (PixelPos >= _GR(RightPorch)) {
				_GR(sourceAddress) = from;
				return skip_RLE8_line();
			}
		} else if (PixelPos < _GR(LeftPorch)) {
			_GR(sourceAddress) = from;
			return skip_RLE8_line();
		}
#endif

		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				Error -= scaleX;

				while (Error < 0) {
					if (data != 128 && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch)) {
						// handle pixel
						r = _GR(Palette)[*to].r;
						r *= data;
						r >>= PALETTE_SHIFTER;
						if (r > 0x1f)
							r = 0x1f;

						g = _GR(Palette)[*to].g;
						g *= data;
						g >>= PALETTE_SHIFTER;
						if (g > 0x1f)
							g = 0x1f;

						b = _GR(Palette)[*to].b;
						b *= data;
						b >>= PALETTE_SHIFTER;
						if (b > 0x1f)
							b = 0x1f;

						*to = _GR(InverseColorTable)[(b << 10) | (g << 5) | r];
					}
					to += Inc;
					PixelPos += Inc;

					Error += 100;
				}
			}
		} else {
			uint8 data = *from++;

			while (runLength--) {
				Error -= scaleX;

				while (Error < 0) {
					if (data != 128 && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch)) {
						// handle pixel
						r = _GR(Palette)[*to].r;
						r *= data;
						r >>= PALETTE_SHIFTER;
						if (r > 0x1f)
							r = 0x1f;

						g = _GR(Palette)[*to].g;
						g *= data;
						g >>= PALETTE_SHIFTER;
						if (g > 0x1f)
							g = 0x1f;

						b = _GR(Palette)[*to].b;
						b *= data;
						b >>= PALETTE_SHIFTER;
						if (b > 0x1f)
							b = 0x1f;

						*to = _GR(InverseColorTable)[(b << 10) | (g << 5) | r];
					}
					to += Inc;
					PixelPos += Inc;

					Error += 100;
				}
			}
		}
	}
}

static RenderResult r_9() {
	uint8 *from = _GR(sourceAddress);
	uint8 *to = _GR(destinationAddress);
	uint8 *depth = _GR(depthAddress);
	uint8 sdepth = _GR(spriteDepth);
	int32 PixelPos = _GR(StartingPixelPos), Error = _GR(X_error);
	uint32 scaleX = _GR(X_scale);
	long Inc = _GR(Increment);
	uint32 r, g, b;

	for (;;) {
#ifdef OPTIMIZE_CLIP
		if (Inc == 1) {
			if (PixelPos >= _GR(RightPorch)) {
				_GR(sourceAddress) = from;
				return skip_RLE8_line();
			}
		} else if (PixelPos < _GR(LeftPorch)) {
			_GR(sourceAddress) = from;
			return skip_RLE8_line();
		}
#endif

		// Assume that the first byte is a run-length counter followed by pix data
		uint8 runLength = *from++;

		if (!runLength) {
			uint8 code = *from++;

			if (code <= kJumpXY) {
				_GR(sourceAddress) = from;
				return (RenderResult)code;
			} else while (code--) {
				uint8 data = *from++;

				Error -= scaleX;

				while (Error < 0) {
					uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

					if (data != 128 && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch) && (!ddepth || sdepth < ddepth)) {
						// Handle pixel
						r = _GR(Palette)[*to].r;
						r *= data;
						r >>= PALETTE_SHIFTER;
						if (r > 0x1f)
							r = 0x1f;

						g = _GR(Palette)[*to].g;
						g *= data;
						g >>= PALETTE_SHIFTER;
						if (g > 0x1f)
							g = 0x1f;

						b = _GR(Palette)[*to].b;
						b *= data;
						b >>= PALETTE_SHIFTER;
						if (b > 0x1f)
							b = 0x1f;

						*to = _GR(InverseColorTable)[(b << 10) | (g << 5) | r];
					}

					to += Inc;
					depth += Inc;
					PixelPos += Inc;

					Error += 100;
				}
			}
		} else {
			uint8 data = *from++;

			while (runLength--) {
				Error -= scaleX;

				while (Error < 0) {
					uint8 ddepth = (uint8)(*depth & DEPTH_MASK);

					if (data != 128 && PixelPos >= _GR(LeftPorch) && PixelPos < _GR(RightPorch) && (!ddepth || sdepth < ddepth)) {
						// Handle pixel
						r = _GR(Palette)[*to].r;
						r *= data;
						r >>= PALETTE_SHIFTER;
						if (r > 0x1f)
							r = 0x1f;

						g = _GR(Palette)[*to].g;
						g *= data;
						g >>= PALETTE_SHIFTER;
						if (g > 0x1f)
							g = 0x1f;

						b = _GR(Palette)[*to].b;
						b *= data;
						b >>= PALETTE_SHIFTER;
						if (b > 0x1f)
							b = 0x1f;

						*to = _GR(InverseColorTable)[(b << 10) | (g << 5) | r];
					}

					to += Inc;
					depth += Inc;
					PixelPos += Inc;

					Error += 100;
				}
			}
		}
	}
}

static RenderResult(*dispatches[])() = {
	r_0, r_1, r_2, r_3, r_4, r_5, r_6, r_7
};

// Render Sprite to 8 Bit Bit Map

int32 scale_this(int32 Value, int32 Scaler, int32 Error);

int32 scale_this(int32 Value, int32 Scaler, int32 Error) {
	int32 Scaled = 0;

	while (Value--) {
		Error -= Scaler;

		while (Error < 0) {
			++Scaled;
			Error += 100;
		}
	}

	return Scaled;
}

void GetUpdateRectangle(int32 x, int32 y, int32 hot_x, int32 hot_y, int32 scale_x, int32 scale_y, int32 Width, int32 Height, M4Rect *UpdateRect) {
	int errY = hot_y * scale_y % 100;

	UpdateRect->y1 = y - hot_y * scale_y / 100;
	UpdateRect->y2 = UpdateRect->y1 + scale_this(Height, scale_y, errY);

	int32 scaleX = scale_x;

	if (scaleX < 0) {
		scaleX = -scaleX;
		UpdateRect->x2 = x + hot_x * scaleX / 100;
		UpdateRect->x1 = UpdateRect->x2 - scale_this(Width, scaleX, hot_x * scaleX % 100);
	} else {
		UpdateRect->x1 = x - hot_x * scaleX / 100;
		UpdateRect->x2 = UpdateRect->x1 + scale_this(Width, scaleX, hot_x * scaleX % 100);
	}
}

typedef struct {
	RenderFunc addr;
	const char *name;
} FuncName;

FuncName fn[] = {
	{ r_0, "r_0" },
	{ r_0_5, "r_0_5" },
	{ r_1, "r_1" },
	{ r_2, "r_2" },
	{ r_3, "r_3" },
	{ r_4, "r_4" },
	{ r_5, "r_5" },
	{ r_6, "r_6" },
	{ r_7, "r_7" },
	{ r_8, "r_8" },
	{ r_9, "r_9" },
};

void render_sprite_to_8BBM(RendGrBuff *Destination, DrawRequestX *dr, RendCell *Frame, M4Rect *ClipRectangle, M4Rect *UpdateRect) {
#ifdef DEBUGGING_REND
	int i, len;

	countStart = Frame->data;
	for (i = 0; i < Frame->Height; ++i)
		if ((len = count_RLE8_line()) != Frame->Width) {
			term_message("line %u of sprite %p doesn't look good...", i, Frame);
			return;
		}
#endif

	int errY = Frame->hot_y * dr->scale_y % 100;
	int ScaledWidth;

	UpdateRect->y1 = dr->y - Frame->hot_y * dr->scale_y / 100;
	UpdateRect->y2 = UpdateRect->y1 + scale_this(Frame->Height, dr->scale_y, errY);

	int32 scaleX = dr->scale_x;

	if (scaleX < 0) {
		scaleX = -scaleX;
		_GR(Increment) = -1;
		UpdateRect->x2 = dr->x + Frame->hot_x * scaleX / 100;
		UpdateRect->x1 = UpdateRect->x2 - (ScaledWidth = scale_this(Frame->Width, scaleX, _GR(X_error) = Frame->hot_x * scaleX % 100));
	} else {
		_GR(Increment) = 1;
		UpdateRect->x1 = dr->x - Frame->hot_x * scaleX / 100;
		UpdateRect->x2 = UpdateRect->x1 + (ScaledWidth = scale_this(Frame->Width, scaleX, _GR(X_error) = Frame->hot_x * scaleX % 100));
	}

	//-------------------------------------------------
	// If sprite is off screen, bail immediately

	if (UpdateRect->y2 < ClipRectangle->y1)
		return;

	if (UpdateRect->y1 > ClipRectangle->y2)
		return;

	if (UpdateRect->x2 < ClipRectangle->x1)
		return;

	if (UpdateRect->x1 > ClipRectangle->x2)
		return;

	//-------------------------------------------------
	// Choose which line renderer to use
	uint32 selector_mask = 0;
	RenderFunc renderer = nullptr;

	if (Frame->Comp & kShadowFlag) {
		_GR(Palette) = dr->Pal;
		_GR(InverseColorTable) = dr->ICT;

		_GR(LeftPorch) = 0;
		_GR(RightPorch) = Destination->Width;
		if (_GR(Increment) == 1)
			_GR(StartingPixelPos) = 0;
		else
			_GR(StartingPixelPos) = ScaledWidth;

		if (dr->depth)
			renderer = r_9;
		else
			renderer = r_8;
	}

	if (dr->depth) {
		if (dr->depth_map) {
			selector_mask |= kDepthed;
			_GR(depthAddress) = dr->depth_map;
			_GR(spriteDepth) = dr->depth;
		} else {
			assert(dr->depth_map == nullptr);
		}
	}

	if (scaleX != 100)
		selector_mask |= kScaled;

	// check for clipping
	if (UpdateRect->x1 < ClipRectangle->x1 || UpdateRect->x2 > ClipRectangle->x2) {
		selector_mask |= kClipped;
		_GR(LeftPorch) = ClipRectangle->x1;
		_GR(RightPorch) = ClipRectangle->x2 + 1;
		if (_GR(Increment) == 1)
			_GR(StartingPixelPos) = UpdateRect->x1;
		else
			_GR(StartingPixelPos) = UpdateRect->x2;
	}

	if (!renderer)
		renderer = dispatches[selector_mask];

	if (renderer == r_0 && _GR(Increment) == -1)
		renderer = r_0_5;

	//-------------------------------------------------
	// Set up horizontal scaler
	_GR(X_scale) = scaleX;

	//-------------------------------------------------
	// Get beginning address of bit map to render at
	if (_GR(Increment) == 1) {
		_GR(destinationAddress) = (uint8 *)Destination->PixMap + UpdateRect->x1 + Destination->Width * UpdateRect->y1;
		_GR(depthAddress) += UpdateRect->x1 + Destination->Width * UpdateRect->y1;
	} else {
		_GR(destinationAddress) = (uint8 *)Destination->PixMap + UpdateRect->x2 + Destination->Width * UpdateRect->y1;
		_GR(depthAddress) += UpdateRect->x2 + Destination->Width * UpdateRect->y1;
	}

	_GR(sourceAddress) = Frame->data;

	int posY = UpdateRect->y1;

	UpdateRect->x1 = MAX(UpdateRect->x1, ClipRectangle->x1);
	UpdateRect->x2 = MIN(UpdateRect->x2, ClipRectangle->x2);
	UpdateRect->y1 = MAX(UpdateRect->y1, ClipRectangle->y1);
	UpdateRect->y2 = MIN(UpdateRect->y2, ClipRectangle->y2);

	for (;;) {
		errY -= dr->scale_y;

		uint8 *prevSourceAddress = _GR(sourceAddress);
		bool AlreadySkippedClippedLine = false;

		if (errY < 0) {
			do {
				if (posY < ClipRectangle->y1) {
					if (!AlreadySkippedClippedLine) {
						_GR(sourceAddress) = prevSourceAddress;
						if (skip_RLE8_line() == kEndOfSprite) {
							term_message("hey nonny nonny");
							return;
						}

						AlreadySkippedClippedLine = true;
					}
				} else {
					_GR(sourceAddress) = prevSourceAddress;
					if (renderer() == kEndOfSprite)
						return;
				}

				if (++posY > ClipRectangle->y2)
					return;

				_GR(destinationAddress) += Destination->Width;
				_GR(depthAddress) += Destination->Width;

				errY += 100;
			} while (errY < 0);
		} else if (skip_RLE8_line() == kEndOfSprite) {
			return;
		}
	}
}

} // namespace M4
