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

#include "voyeur/screen.h"
#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Voyeur {

/*------------------------------------------------------------------------*/

DrawInfo::DrawInfo(int penColor, const Common::Point &pos) {
	_penColor = penColor;
	_pos = pos;
}

/*------------------------------------------------------------------------*/

Screen::Screen(VoyeurEngine *vm) : Graphics::Screen(), _vm(vm), _drawPtr(&_defaultDrawInfo),
		_defaultDrawInfo(1, Common::Point()) {
	_SVGAMode = 0;
	_planeSelect = 0;
	_saveBack = true;
	_clipPtr = NULL;
	_viewPortListPtr = NULL;
	_backgroundPage = NULL;
	_vPort = NULL;
	_fontPtr = NULL;
	Common::fill(&_VGAColors[0], &_VGAColors[PALETTE_SIZE], 0);
	_fontChar = new PictureResource(DISPFLAG_NONE, 0xff, 0xff, 0, Common::Rect(), 0, NULL, 0);
	_backColors = nullptr;
}

void Screen::sInitGraphics() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	create(SCREEN_WIDTH, SCREEN_HEIGHT);
	clearPalette();
}

Screen::~Screen() {
	delete _fontChar;
}

void Screen::setupMCGASaveRect(ViewPortResource *viewPort) {
	if (viewPort->_activePage) {
		viewPort->_activePage->_flags |= DISPFLAG_1;
		Common::Rect *clipRect = _clipPtr;
		_clipPtr = &viewPort->_clipRect;

		sDrawPic(viewPort->_activePage, viewPort->_currentPic, Common::Point());

		_clipPtr = clipRect;
	}

	viewPort->_rectListCount[1] = -1;
}

void Screen::addRectOptSaveRect(ViewPortResource *viewPort, int idx, const Common::Rect &bounds) {
	if (viewPort->_rectListCount[idx] == -1)
		return;

	// TODO: Lots of code in original, which I suspect may be overlapping rect merging
	viewPort->_rectListPtr[idx]->push_back(bounds);
	++viewPort->_rectListCount[idx];
}

void Screen::restoreMCGASaveRect(ViewPortResource *viewPort) {
	if (viewPort->_rectListCount[0] != -1) {
		for (int i = 0; i < viewPort->_rectListCount[0]; ++i) {
			addRectOptSaveRect(viewPort, 1, (*viewPort->_rectListPtr[0])[i]);
		}
	} else {
		viewPort->_rectListCount[1] = -1;
	}

	restoreBack(*viewPort->_rectListPtr[1], viewPort->_rectListCount[1], viewPort->_pages[0],
		viewPort->_pages[1]);

	int count = viewPort->_rectListCount[0];
	restoreBack(*viewPort->_rectListPtr[0], viewPort->_rectListCount[0],
		viewPort->_activePage, viewPort->_currentPic);

	SWAP(viewPort->_rectListPtr[0], viewPort->_rectListPtr[1]);
	viewPort->_rectListCount[1] = count;
}

void Screen::addRectNoSaveBack(ViewPortResource *viewPort, int idx, const Common::Rect &bounds) {
	// Stubbed/dummy method in the original.
}

void Screen::sDrawPic(DisplayResource *srcDisplay, DisplayResource *destDisplay,
		const Common::Point &initialOffset) {
	int width1, width2;
	int widthDiff, widthDiff2;
	int height1;
	int srcOffset;
	int screenOffset;
	int srcFlags, destFlags;
	ViewPortResource *destViewPort = NULL;
	Common::Rect newBounds;
	Common::Rect backBounds;
	int tmpWidth = 0;
	int tmpHeight = 0;
	bool isClipped = false;
	byte pixel = 0;
	int runLength;

	byte *srcImgData, *destImgData;
	const byte *srcP;
	byte *destP;
	byte byteVal, byteVal2;

	PictureResource *srcPic;
	PictureResource *destPic;

	// Get the picture parameters, or deference viewport pointers to get their pictures
	if (srcDisplay->_flags & DISPFLAG_VIEWPORT) {
		// A viewport was passed, not a picture
		srcPic = ((ViewPortResource *)srcDisplay)->_currentPic;
	} else {
		srcPic = (PictureResource *)srcDisplay;
	}

	if (destDisplay->_flags & DISPFLAG_VIEWPORT) {
		// A viewport was passed, not a picture
		destViewPort = (ViewPortResource *)destDisplay;
		destPic = destViewPort->_currentPic;
	} else {
		destPic = (PictureResource *)destDisplay;
	}

	Common::Point offset = Common::Point(initialOffset.x + srcPic->_bounds.left - destPic->_bounds.left,
		initialOffset.y + srcPic->_bounds.top - destPic->_bounds.top);
	width1 = width2 = srcPic->_bounds.width();
	height1 = srcPic->_bounds.height();
	srcOffset = 0;
	srcFlags = srcPic->_flags;
	destFlags = destPic->_flags;
	byte *cursorData = NULL;

	if (srcFlags & DISPFLAG_1) {
		if (_clipPtr) {
			int xs = _clipPtr->left - destPic->_bounds.left;
			int ys = _clipPtr->top - destPic->_bounds.top;
			newBounds = Common::Rect(xs, ys, xs + _clipPtr->width(), ys + _clipPtr->height());
		} else if (destViewPort) {
			int xs = destViewPort->_clipRect.left - destPic->_bounds.left;
			int ys = destViewPort->_clipRect.top - destPic->_bounds.top;
			newBounds = Common::Rect(xs, ys, xs + destViewPort->_clipRect.width(),
				ys + destViewPort->_clipRect.height());
		} else {
			newBounds = Common::Rect(0, 0, destPic->_bounds.width(), destPic->_bounds.height());
		}

		tmpHeight = offset.y - newBounds.top;
		if (tmpHeight < 0) {
			srcOffset -= tmpHeight * width2;
			height1 += tmpHeight;
			offset.y = newBounds.top;

			if (height1 <= 0)
				return;

			isClipped = true;
		}

		int yMin = newBounds.bottom - (offset.y + height1);
		if (yMin < 0) {
			height1 += yMin;
			if (height1 <= 0)
				return;
		}

		tmpWidth = offset.x - newBounds.left;
		if (tmpWidth < 0) {
			srcOffset -= tmpWidth;
			width2 += tmpWidth;
			offset.x = newBounds.left;

			if (width2 <= 0)
				return;

			isClipped = true;
		}

		int xMin = newBounds.right - (offset.x + width2);
		if (xMin < 0) {
			width2 += xMin;
			if (width2 <= 0)
				return;

			isClipped = true;
		}
	}

	screenOffset = offset.y * destPic->_bounds.width() + offset.x;
	widthDiff = width1 - width2;
	widthDiff2 = destPic->_bounds.width() - width2;

	if (destViewPort) {
		if (!_saveBack || (srcPic->_flags & DISPFLAG_800)) {
			backBounds.left = destPic->_bounds.left + offset.x;
			backBounds.top = destPic->_bounds.top + offset.y;
			backBounds.setWidth(width2);
			backBounds.setHeight(height1);
			addRectOptSaveRect(destViewPort, 1, backBounds);

		} else if (!destViewPort->_addFn) {
			if (destViewPort->_rectListCount[destViewPort->_pageIndex] < -1) {
				Common::Rect r;
				r.left = destPic->_bounds.left + offset.x;
				r.top = destPic->_bounds.top + offset.y;
				r.setWidth(width2);
				r.setHeight(height1);

				(*destViewPort->_rectListPtr[destViewPort->_pageIndex]).push_back(r);
				++destViewPort->_rectListCount[destViewPort->_pageIndex];
			}
		} else {
			int xs = offset.x + destPic->_bounds.left;
			int ys = offset.y + destPic->_bounds.top;
			backBounds = Common::Rect(xs, ys, xs + width2, ys + height1);

			(this->*destViewPort->_addFn)(destViewPort, destViewPort->_bounds.top, backBounds);
		}
	}

	if (srcFlags & DISPFLAG_1000) {
		int imageDataShift = 0;
		srcImgData = srcPic->_imgData + (imageDataShift << 14);
		for (uint idx = 0; idx < srcPic->_maskData; ++idx) {
			if (imageDataShift < 4)
				++imageDataShift;
		}

		destImgData = destPic->_imgData + (imageDataShift << 14);
		for (uint idx = 0; idx < srcPic->_maskData; ++idx) {
			if (imageDataShift < 4)
				++imageDataShift;
		}
	} else {
		srcImgData = srcPic->_imgData;
		destImgData = destPic->_imgData;
	}

	if (srcPic->_select != 0xff)
		return;

	if (destFlags & DISPFLAG_CURSOR) {
		cursorData = new byte[width2 * height1];
		Common::fill(cursorData, cursorData + width2 * height1, 0);
		destImgData = cursorData;
	}

	if (srcPic->_pick == 0xff) {
		if (srcFlags & DISPFLAG_8) {
			error("TODO: sDrawPic variation");
		} else {
			// loc_258B8
			srcP = srcImgData + srcOffset;

			if (destFlags & DISPFLAG_8) {
				// loc_258D8
				destP = destImgData + screenOffset;

				if (srcFlags & DISPFLAG_2) {
					// loc_25652
					srcP = srcImgData + srcOffset;

					if (destFlags & DISPFLAG_8) {
						// loc_2566F
						if (srcFlags & DISPFLAG_2) {
							// loc_256FA
							srcP = (const byte *)getPixels() + srcOffset;

							for (int yp = 0; yp < height1; ++yp) {
								for (int xp = 0; xp < width2; ++xp, ++srcP, ++destP) {
									pixel = *srcP;
									if (pixel)
										*destP = pixel;
								}

								srcP += widthDiff;
								destP += widthDiff2;
							}
						} else {
							// loc_25706
							for (int yp = 0; yp < height1; ++yp) {
								Common::copy(srcP, srcP + width2, destP);
								srcP += width2 + widthDiff;
								destP += width2 + widthDiff2;
							}
						}
					} else {
						// loc_25773
						destP = destImgData + screenOffset;

						if (srcFlags & DISPFLAG_2) {
							// loc_25793
							for (int yp = 0; yp < height1; ++yp) {
								Common::copy(srcP, srcP + width2, destP);
								srcP += width2 + widthDiff;
								destP += width2 + widthDiff2;
							}
						} else {
							// loc_25829
							destP = (byte *)getPixels() + screenOffset;

							for (int yp = 0; yp < height1; ++yp) {
								Common::copy(srcP, srcP + width2, destP);
								srcP += width2 + widthDiff;
								destP += width2 + widthDiff2;
							}

							addDirtyRect(Common::Rect(offset.x, offset.y, offset.x + width2,
								offset.y + height1));
						}
					}
				} else {
					// loc_25D40
					if (srcFlags & DISPFLAG_100) {
						// loc_25D4A
						error("TODO: sDrawPic variation");
					} else {
						// loc_2606D
						destP = (byte *)getPixels() + screenOffset;

						for (int yp = 0; yp < height1; ++yp) {
							Common::copy(srcP, srcP + width2, destP);
							destP += width2 + widthDiff2;
							srcP += width2 + widthDiff;
						}

						addDirtyRect(Common::Rect(offset.x, offset.y, offset.x + width2,
							offset.y + height1));
					}
				}
			} else {
				// loc_2615E
				destP = destImgData + screenOffset;

				if (srcFlags & DISPFLAG_2) {
					// loc_2617e
					if (srcFlags & DISPFLAG_100) {
						// loc_26188
						srcP = srcImgData;
						if (isClipped) {
							// loc_26199
							tmpWidth = (tmpWidth < 0) ? -tmpWidth : 0;
							int xMax = tmpWidth + width2;
							tmpHeight = (tmpHeight < 0) ? -tmpHeight : 0;

							width2 = srcPic->_bounds.width();
							height1 = tmpHeight + height1;

							for (int yp = 0; yp < height1; ++yp) {
								runLength = 0;

								for (int xp = 0; xp < width2; ++xp, --runLength) {
									if (runLength <= 0) {
										pixel = *srcP++;
										if (pixel & 0x80) {
											pixel &= 0x7f;
											runLength = *srcP++;
											if (!runLength)
												runLength = width2;
										}
									}

									if (yp >= tmpHeight && xp >= tmpWidth && xp < xMax) {
										if (pixel > 0)
											*destP = pixel;
										++destP;
									}
								}

								if (yp >= tmpHeight)
									destP += widthDiff2;
							}
						} else {
							// loc_262BE
							byteVal = 0;
							for (int yp = 0; yp < height1; ++yp) {
								for (int xp = 0; xp < width2; ++xp) {
									byteVal2 = 0;
									if (!byteVal2) {
										byteVal = *++srcP;
										if (byteVal & 0x80) {
											byteVal &= 0x7f;
											byteVal2 = *srcP++;

											if (!byteVal2)
												byteVal2 = width2;
										}
									}

									if (byteVal > 0)
										*destP = byteVal;

									++destP;
									--byteVal2;
								}

								destP += widthDiff2;
							}
						}
					} else {
						// loc_2637F
						// Copy with transparency
						for (int yp = 0; yp < height1; ++yp) {
							for (int xp = 0; xp < width2; ++xp, ++srcP, ++destP) {
								if (*srcP != 0)
									*destP = *srcP;
							}

							destP += widthDiff2;
							srcP += widthDiff;
						}
					}
				} else {
					if (srcFlags & DISPFLAG_100) {
						// Simple run-length encoded image
						srcP = srcImgData;

						if (isClipped) {
							// loc_26424
							tmpWidth = (tmpWidth < 0) ? -tmpWidth : 0;
							int xMax = tmpWidth + width2;
							tmpHeight = (tmpHeight < 0) ? -tmpHeight : 0;
							width2 = srcPic->_bounds.width();
							height1 = tmpHeight + height1;

							for (int yp = 0; yp < height1; ++yp) {
								runLength = 0;
								for (int xp = 0; xp < width2; ++xp, --runLength) {
									if (runLength <= 0) {
										pixel = *srcP++;
										if (pixel & 0x80) {
											pixel &= 0x7F;
											runLength = *srcP++;

											if (!runLength)
												runLength = width2;
										}
									}

									if (yp >= tmpHeight && xp >= tmpWidth && xp < xMax) {
										*destP++ = pixel;
									}
								}

								if (yp >= tmpHeight)
									destP += widthDiff2;
							}
						} else {
							// loc_26543
							for (int yp = 0; yp < height1; ++yp) {
								int runLen = 0;
								for (int xp = 0; xp < width2; ++xp, --runLen) {
									if (runLen <= 0) {
										// Start of run length, so get pixel and repeat length
										pixel = *srcP++;
										if (pixel & 0x80) {
											pixel &= 0x7f;
											runLen = *srcP++;
											if (runLen == 0)
												runLen = width2;
										}
									}

									// Copy pixel to output
									*destP++ = pixel;
								}

								destP += widthDiff2;
							}
						}
					} else {
						for (int yp = 0; yp < height1; ++yp) {
							Common::copy(srcP, srcP + width2, destP);
							destP += width2 + widthDiff2;
							srcP += width2 + widthDiff;
						}
					}
				}
			}
		}
	} else {
		// loc_26666
		if (srcPic->_pick == 0) {
			// loc_2727A
			byte onOff = srcPic->_onOff;

			if (srcFlags & DISPFLAG_2) {
				if (!(srcFlags & DISPFLAG_8)) {
					srcP = srcImgData + srcOffset;

					if (destFlags & DISPFLAG_8) {
						// loc_272C3
						error("TODO: sDrawPic variation");
					} else {
						destP = destImgData + screenOffset;
						for (int yp = 0; yp < height1; ++yp) {
							for (int xp = 0; xp < width2; ++xp, ++destP) {
								if ((int8)*srcP++ < 0)
									*destP = onOff;
							}

							destP += widthDiff2;
							srcP += widthDiff;
						}
					}
				}
			} else {
				// loc_27477
				if (destFlags & DISPFLAG_8) {
					// loc_27481
					destP = (byte *)getPixels() + screenOffset;
					for (int yp = 0; yp < height1; ++yp) {
						Common::fill(destP, destP + width2, onOff);
						destP += width2 + widthDiff2;
					}

					addDirtyRect(Common::Rect(offset.x, offset.y, offset.x + width2,
						offset.y + height1));
				} else {
					// loc_2753C
					destP = destImgData + screenOffset;

					for (int yp = 0; yp < height1; ++yp) {
						Common::fill(destP, destP + width2, onOff);
						destP += width2 + widthDiff2;
					}
				}
			}

		} else {
			// loc_26673
			byte pick = srcPic->_pick;
			byte onOff = srcPic->_onOff;

			if (!(srcFlags & PICFLAG_PIC_OFFSET)) {
				srcP = srcImgData += srcOffset;
				pixel = 0;

				if (destFlags & PICFLAG_PIC_OFFSET) {
					destP = destImgData + screenOffset;
					if (srcFlags & PICFLAG_2) {
						if (srcFlags & PICFLAG_100) {
							if (isClipped) {
								// loc_266E3
								destP = (byte *)getPixels() + screenOffset;
								tmpWidth = (tmpWidth < 0) ? -tmpWidth : 0;
								int xMax = tmpWidth + width2;
								tmpHeight = (tmpHeight < 0) ? -tmpHeight : 0;
								pick = 0x7F;
								width2 = srcPic->_bounds.width();
								height1 = tmpHeight + height1;

								for (int yp = 0; yp < height1; ++yp) {
									int runLen = 0;
									for (int xp = 0; xp < width2; ++xp, --runLen) {
										if (runLen <= 0) {
											pixel = *srcP++;
											if (pixel & 0x80) {
												pixel &= 0x7F;
												runLen = *srcP++;
												if (!runLen)
													runLen = width2;
											}
										}

										if (yp >= tmpHeight && xp >= tmpWidth && xp < xMax) {
											if (pixel) {
												*destP = (pixel & pick) ^ onOff;
											}
											++destP;
										}
									}
									if (yp >= tmpHeight)
										destP += widthDiff2;
								}

								addDirtyRect(Common::Rect(offset.x, offset.y, offset.x + width2,
									offset.y + height1));
							} else {
								// loc_26815
								destP = (byte *)getPixels() + screenOffset;

								for (int yp = 0; yp < height1; ++yp) {
									for (int xi = 0; xi < width2; ++xi, ++destP) {
										byteVal2 = 0;
										for (int xp = 0; xp < width2; ++xp, ++destP, --byteVal2) {
											if (!byteVal2) {
												pixel = *srcP++;
												if (pixel & 0x80) {
													pixel &= 0x7F;
													byteVal2 = *srcP++;
													if (!byteVal2) {
														byteVal2 = width2;
													}
												}
											}

											if (pixel)
												*destP = (pixel & pick) ^ onOff;
										}
									}

									destP += widthDiff2;
								}

								addDirtyRect(Common::Rect(offset.x, offset.y, offset.x + width2,
									offset.y + height1));
							}
						} else {
							// Direct screen write
							destP = (byte *)getPixels() + screenOffset;

							for (int yp = 0; yp < height1; ++yp) {
								for (int xp = 0; xp < width2; ++xp, ++srcP, ++destP) {
									if (*srcP)
										*destP = (*srcP & pick) ^ onOff;
								}
								destP += widthDiff2;
								srcP += widthDiff;
							}

							addDirtyRect(Common::Rect(offset.x, offset.y, offset.x + width2,
								offset.y + height1));
						}
					} else if (srcFlags & PICFLAG_100) {
						srcP = srcImgData;
						if (isClipped) {
							// loc_269FD
							tmpWidth = (tmpWidth < 0) ? -tmpWidth : 0;
							int xMax = tmpWidth + width2;
							tmpHeight = (tmpHeight < 0) ? -tmpHeight : 0;
							width2 = srcPic->_bounds.width();
							height1 = tmpHeight + height1;

							for (int yp = 0; yp < height1; ++yp) {
								runLength = 0;
								for (int xp = 0; xp < width2; ++xp, --runLength) {
									if (runLength <= 0) {
										pixel = *srcP++;
										if (pixel & 0x80) {
											pixel &= 0x7F;
											runLength = *srcP++;

											if (!runLength)
												runLength = width2;
										}
									}

									if (yp >= tmpHeight && xp >= tmpWidth && xp < xMax) {
										*destP++ = (pixel & 0x80) ^ onOff;
									}
								}
							}
						} else {
							// loc_26BD5
							destP = (byte *)getPixels() + screenOffset;

							for (int yp = 0; yp < height1; ++yp) {
								byteVal2 = 0;

								for (int xp = 0; xp < width2; ++xp, ++destP) {
									if (!byteVal2) {
										pixel = *srcP++;
										if (pixel & 0x80) {
											pixel &= 0x7F;
											byteVal2 = *srcP++;
											if (!byteVal2)
												byteVal2 = width2;
										}
									}

									*destP = (pixel & pick) ^ onOff;
								}

								destP += widthDiff2;
							}

							addDirtyRect(Common::Rect(offset.x, offset.y, offset.x + width2,
								offset.y + height1));
						}
					} else {
						// loc_26C9A
						destP = (byte *)getPixels() + screenOffset;

						for (int yp = 0; yp < height1; ++yp) {
							for (int xp = 0; xp < width2; ++xp, ++srcP, ++destP) {
								*destP = (*srcP & pick) ^ onOff;
							}
							destP += widthDiff2;
							srcP += widthDiff;
						}

						addDirtyRect(Common::Rect(offset.x, offset.y, offset.x + width2,
							offset.y + height1));
					}
				} else {
					// loc_26D2F
					destP = destImgData + screenOffset;

					if (srcFlags & PICFLAG_2) {
						// loc_26D4F
						if (srcFlags & PICFLAG_100) {
							srcP = srcImgData;

							if (isClipped) {
								// loc_26D6A
								tmpWidth = (tmpWidth < 0) ? -tmpWidth : 0;
								int xMax = tmpWidth + width2;
								tmpHeight = (tmpHeight < 0) ? -tmpHeight : 0;
								width2 = srcPic->_bounds.width();
								height1 = tmpHeight + height1;

								for (int yp = 0; yp < height1; ++yp) {
									runLength = 0;

									for (int xp = 0; xp < width2; ++xp, --runLength) {
										if (runLength <= 0) {
											pixel = *srcP++;
											if (pixel & 0x80) {
												pixel &= 0x7F;
												runLength = *srcP++;
												if (!runLength)
													runLength = width2;
											}
										}

										if (yp >= tmpHeight && xp >= tmpWidth && xp < xMax) {
											if (pixel)
												*destP = (pixel & pick) ^ onOff;

											++destP;
										}
									}

									if (yp >= tmpHeight)
										destP += widthDiff2;
								}
							} else {
								// loc_26E95
								for (int yp = 0; yp < height1; ++yp) {
									byteVal2 = 0;
									for (int xp = 0; xp < width2; ++xp, ++destP, --byteVal2) {
										if (!byteVal2) {
											pixel = *srcP++;
											if (pixel & 0x80) {
												pixel &= 0x7F;
												byteVal2 = *srcP++;
												if (!byteVal2)
													byteVal2 = width2;
											}
										}

										if (pixel)
											*destP = (pixel & pick) ^ onOff;
									}

									destP += widthDiff2;
								}
							}
						} else {
							// loc_26F5D
							for (int yp = 0; yp < height1; ++yp) {
								for (int xp = 0; xp < width2; ++xp, ++srcP, ++destP) {
									if (*srcP)
										*destP = (*srcP & pick) ^ onOff;
								}
								destP += widthDiff2;
								srcP += widthDiff;
							}
						}
					} else {
						// loc_26FEF
						if (srcFlags & PICFLAG_100) {
							// loc_26FF9
							for (int yp = 0; yp < height1; ++yp) {
								for (int xp = 0; xp < width2; ++xp, ++srcP, ++destP) {
									*destP = (*srcP & pick) ^ onOff;
								}
								destP += widthDiff2;
								srcP += widthDiff;
							}
						} else {
							// loc_271F0
							srcP = srcImgData;

							if (isClipped) {
								// loc_2700A
								tmpWidth = (tmpWidth < 0) ? -tmpWidth : 0;
								int xMax = tmpWidth + width2;
								tmpHeight = (tmpHeight < 0) ? -tmpHeight : 0;
								width2 = srcPic->_bounds.width();
								height1 = tmpHeight + height1;

								for (int yp = 0; yp < height1; ++yp) {
									runLength = 0;

									for (int xp = 0; xp < width2; ++xp, --runLength) {
										if (runLength <= 0) {
											pixel = *srcP++;
											if (pixel & 0x80) {
												pixel &= 0x7F;
												runLength = *srcP++;
												if (!runLength)
													runLength = width2;
											}
										}

										if (yp >= tmpHeight && xp >= tmpWidth && xp < xMax) {
											*destP++ = (pixel & pick) ^ onOff;
										}
									}

									if (yp >= tmpHeight)
										destP += widthDiff2;
								}
							} else {
								// loc_2712F
								for (int yp = 0; yp < height1; ++yp) {
									byteVal2 = 0;
									for (int xp = 0; xp < width2; ++xp, ++destP, --byteVal2) {
										if (!byteVal2) {
											pixel = *srcP++;
											if (pixel & 0x80) {
												pixel &= 0x7F;
												byteVal2 = *srcP++;
												if (!byteVal2)
													byteVal2 = width2;
											}
										}

										*destP = (*srcP & pick) ^ onOff;
									}
									destP += widthDiff2;
								}
							}
						}
					}
				}
			}
		}
	}

	if (cursorData) {
		_vm->_eventsManager->setCursor(cursorData, width2, height1, srcPic->_keyColor);
		delete[] cursorData;
	}
}

void Screen::drawANumber(DisplayResource *display, int num, const Common::Point &pt) {
	PictureResource *pic = _vm->_bVoy->boltEntry(num + 261)._picResource;
	sDrawPic(pic, display, pt);
}

void Screen::fillPic(DisplayResource *display, byte onOff) {
	PictureResource *pic;
	if (display->_flags & DISPFLAG_VIEWPORT) {
		pic = ((ViewPortResource *)display)->_currentPic;
	} else {
		pic = (PictureResource *)display;
	}

	PictureResource picResource;
	picResource._flags = DISPFLAG_NONE;
	picResource._select = 0xff;
	picResource._pick = 0;
	picResource._onOff = onOff;
	picResource._bounds = pic->_bounds;

	sDrawPic(&picResource, display, Common::Point());
}

/**
 * Queues the given picture for display
 */
void Screen::sDisplayPic(PictureResource *pic) {
	_vm->_eventsManager->_intPtr._flipWait = true;
}

void Screen::flipPage() {
	Common::Array<ViewPortResource *> &viewPorts = _viewPortListPtr->_entries;
	bool flipFlag = false;

	for (uint idx = 0; idx < viewPorts.size(); ++idx) {
		if (viewPorts[idx]->_flags & DISPFLAG_20) {
			flipFlag = false;
			if ((viewPorts[idx]->_flags & DISPFLAG_8) && (viewPorts[idx]->_flags & DISPFLAG_1)) {
				if (_planeSelect == idx)
					sDisplayPic(viewPorts[idx]->_currentPic);
				flipFlag = true;
			}
		}

		if (flipFlag) {
			ViewPortResource &viewPort = *viewPorts[idx];

			viewPort._lastPage = viewPort._pageIndex;
			++viewPort._pageIndex;

			if (viewPort._pageIndex >= viewPort._pageCount)
				viewPort._pageIndex = 0;

			assert(viewPort._pageIndex < 2);
			viewPort._currentPic = viewPort._pages[viewPort._pageIndex];
			viewPort._flags = (viewPort._flags & ~DISPFLAG_8) | DISPFLAG_40;
		}
	}
}

void Screen::restoreBack(Common::Array<Common::Rect> &rectList, int rectListCount,
		PictureResource *srcPic, PictureResource *destPic) {
	// WORKAROUND: Since _backgroundPage can point to a resource freed at the end of display methods,
	// I'm now explicitly resetting it to null in screenReset(), so at this point it can be null
	if (!srcPic)
		return;

	bool saveBack = _saveBack;
	_saveBack = false;

	if (rectListCount == -1) {
		sDrawPic(srcPic, destPic, Common::Point());
	} else {
		for (int i = rectListCount - 1; i >= 0; --i) {
			_clipPtr = &rectList[i];
			sDrawPic(srcPic, destPic, Common::Point());
		}
	}

	_saveBack = saveBack;
}

void Screen::setPalette(const byte *palette, int start, int count) {
	Graphics::Screen::setPalette(palette, start, count);
	_vm->_eventsManager->_gameData._hasPalette = false;
}

void Screen::setPalette128(const byte *palette, int start, int count) {
	byte rgb[3];
	getPalette(&rgb[0], 128, 1);
	Graphics::Screen::setPalette(palette, start, count);
	Graphics::Screen::setPalette(&rgb[0], 128, 1);
}

void Screen::resetPalette() {
	for (int i = 0; i < 256; ++i)
		setColor(i, 0, 0, 0);

	_vm->_eventsManager->_intPtr._hasPalette = true;
}

void Screen::setColor(int idx, byte r, byte g, byte b) {
	byte *vgaP = &_VGAColors[idx * 3];
	vgaP[0] = r;
	vgaP[1] = g;
	vgaP[2] = b;

	_vm->_eventsManager->_intPtr._palStartIndex = MIN(_vm->_eventsManager->_intPtr._palStartIndex, idx);
	_vm->_eventsManager->_intPtr._palEndIndex = MAX(_vm->_eventsManager->_intPtr._palEndIndex, idx);
}

void Screen::setOneColor(int idx, byte r, byte g, byte b) {
	byte palEntry[3];
	palEntry[0] = r;
	palEntry[1] = g;
	palEntry[2] = b;
	g_system->getPaletteManager()->setPalette(&palEntry[0], idx, 1);
}

void Screen::setColors(int start, int count, const byte *pal) {
	for (int i = 0; i < count; ++i) {
		if ((i + start) != 128) {
			const byte *rgb = pal + i * 3;
			setColor(i + start, rgb[0], rgb[1], rgb[2]);
		}
	}

	_vm->_eventsManager->_intPtr._hasPalette = true;
}

void Screen::screenReset() {
	resetPalette();

	_backgroundPage = NULL;
	_vPort->setupViewPort(NULL);
	fillPic(_vPort, 0);

	_vm->flipPageAndWait();
}

void Screen::fadeDownICF1(int steps) {
	if (steps > 0) {
		int stepAmount = _vm->_voy->_fadingAmount2 / steps;

		for (int idx = 0; idx < steps; ++idx) {
			_vm->_voy->_fadingAmount2 -= stepAmount;
			_vm->_eventsManager->delay(1);
		}
	}

	_vm->_voy->_fadingAmount2 = 0;
}

void Screen::fadeUpICF1(int steps) {
	if (steps > 0) {
		int stepAmount = (63 - _vm->_voy->_fadingAmount2) / steps;

		for (int idx = 0; idx < steps; ++idx) {
			_vm->_voy->_fadingAmount2 += stepAmount;
			_vm->_eventsManager->delay(1);
		}
	}

	_vm->_voy->_fadingAmount2 = 63;
}

void Screen::fadeDownICF(int steps) {
	if (steps > 0) {
		_vm->_eventsManager->hideCursor();
		int stepAmount1 = _vm->_voy->_fadingAmount1 / steps;
		int stepAmount2 = _vm->_voy->_fadingAmount2 / steps;

		for (int idx = 0; idx < steps; ++idx) {
			_vm->_voy->_fadingAmount1 -= stepAmount1;
			_vm->_voy->_fadingAmount2 -= stepAmount2;
			_vm->_eventsManager->delay(1);
		}
	}

	_vm->_voy->_fadingAmount1 = 0;
	_vm->_voy->_fadingAmount2 = 0;
}

void Screen::drawDot() {
	for (int idx = 0; idx < 9; ++idx) {
		uint offset = DOT_LINE_START[idx] + DOT_LINE_OFFSET[idx];
		int xp = offset % SCREEN_WIDTH;
		int yp = offset / SCREEN_WIDTH;

		byte *pDest = (byte *)getPixels() + offset;
		Common::fill(pDest, pDest + DOT_LINE_LENGTH[idx], 0x80);
		addDirtyRect(Common::Rect(xp, yp, xp + DOT_LINE_LENGTH[idx], yp + 1));
	}
}

void Screen::synchronize(Common::Serializer &s) {
	s.syncBytes(&_VGAColors[0], PALETTE_SIZE);
}

} // End of namespace Voyeur
