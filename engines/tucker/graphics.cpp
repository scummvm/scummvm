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

#include "tucker/graphics.h"

namespace Tucker {

int Graphics::encodeRLE(const uint8 *src, uint8 *dst, int w, int h) {
	int count = 0;
	int dstOffset = 0;
	int code = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			code = src[x];
			if (code == 0) {
				++count;
				if (count > 200) {
					dst[dstOffset++] = 0;
					dst[dstOffset++] = count;
					count = 0;
				}
			} else {
				if (count > 0) {
					dst[dstOffset++] = 0;
					dst[dstOffset++] = count;
					count = 0;
				}
				dst[dstOffset++] = code;
			}
		}
		src += 320;
	}
	if (count > 0) {
		dst[dstOffset++] = 0;
		dst[dstOffset++] = count;
	}
	return dstOffset;
}

int Graphics::encodeRAW(const uint8 *src, uint8 *dst, int w, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(dst, src, w);
		dst += w;
		src += 320;
	}
	return w * h;
}

void Graphics::decodeRLE(uint8 *dst, const uint8 *src, int w, int h) {
	int code = 0;
	int offset = 0;
	int color = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (code == 0) {
				color = src[offset++];
				if (color == 0) {
					code = src[offset++];
					--code;
					if (x + code < w) {
						x += code;
						code = 0;
					} else {
						code -= w - 1 - x;
						x = w - 1;
					}
				}
			} else {
				--code;
				if (x + code < w) {
					x += code;
					code = 0;
				} else {
					code -= w - 1 - x;
					x = w - 1;
				}
			}
			if (color != 0) {
				dst[x] = color;
			}
		}
		dst += 640;
	}
}

void Graphics::decodeRLE_224(uint8 *dst, const uint8 *src, int w, int h) {
	int code = 0;
	int offset = 0;
	int color = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (code == 0) {
				color = src[offset++];
				if (color != 0) {
					if (dst[x] < 0xE0) {
						dst[x] = color;
					}
				} else {
					code = src[offset++];
					--code;
					if (x + code < w) {
						x += code;
						code = 0;
					} else {
						code -= w - 1 - x;
						x = w - 1;
					}
				}
			} else {
				--code;
				if (x + code < w) {
					x += code;
					code = 0;
				} else {
					code -= w - 1 - x;
					x = w - 1;
				}
			}
		}
		dst += 640;
	}
}

void Graphics::decodeRLE_248(uint8 *dst, const uint8 *src, int w, int h, int y1, int y2, bool xflip) {
	int code = 0;
	int offset = 0;
	int color = 0;
	int dstOffset = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			dstOffset = xflip ? (w - 1 - x) : x;
			if (code == 0) {
				color = src[offset++];
				if (color == 0) {
					code = src[offset++];
					--code;
					if (x + code < w) {
						x += code;
						code = 0;
					} else {
						code -= w - 1 - x;
						x = w - 1;
					}
				}
			} else {
				--code;
				if (x + code < w) {
					x += code;
					code = 0;
				} else {
					code -= w - 1 - x;
					x = w - 1;
				}
			}
			if (color != 0) {
				if ((dst[dstOffset] < 0xE0 || y + y1 < y2) && dst[dstOffset] < 0xF8) {
					dst[dstOffset] = color;
				}
			}
		}
		dst += 640;
	}
}

void Graphics::decodeRLE_320(uint8 *dst, const uint8 *src, int w, int h) {
	int code = 0;
	int offset = 0;
	int color = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (code == 0) {
				color = src[offset++];
				if (color == 0) {
					code = src[offset++];
				}
			}
			if (code == 0) {
				dst[x] = color;
			} else {
				--code;
			}
		}
		dst += 320;
	}
}

void Graphics::copyFrom640(const uint8 *src, uint8 *dst, int w, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(dst, src, w);
		dst += w;
		src += 640;
	}
}

void Graphics::copyTo640(uint8 *dst, const uint8 *src, int w, int srcPitch, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(dst, src, w);
		dst += 640;
		src += srcPitch;
	}
}

void Graphics::drawStringChar(uint8 *dst, uint8 chr, int pitch, uint8 chrColor, const uint8 *src) {
	if (chr < 32 || chr - 32 >= _charset->xCount * _charset->yCount) {
		return;
	}
	int offset = (chr - 32) * _charset->charH * _charset->charW;
	for (int y = 0; y < _charset->charH; ++y) {
		for (int x = 0; x < _charset->charW; ++x) {
			const int color = src[offset++];
			if (color != 0) {
				if (_charset == &_creditsCharset) {
					dst[x] = color;
				} else {
					dst[x] = (color == 128) ? color : chrColor;
				}
			}
		}
		dst += pitch;
	}
}

} // namespace Tucker
