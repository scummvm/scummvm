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

#include "video/video_decoder.h"
#include "video/qt_decoder.h"
#include "video/avi_decoder.h"

#include "common/file.h"
#include "common/substream.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"

#include "mtropolis/assets.h"
#include "mtropolis/audio_player.h"
#include "mtropolis/coroutines.h"
#include "mtropolis/elements.h"
#include "mtropolis/element_factory.h"
#include "mtropolis/miniscript.h"
#include "mtropolis/render.h"

namespace MTropolis {

GraphicElement::GraphicElement() : _cacheBitmap(false) {
}

GraphicElement::GraphicElement(const GraphicElement &other)
	: VisualElement(other), _cacheBitmap(other._cacheBitmap), _mask(nullptr) {
}

GraphicElement::~GraphicElement() {
}

bool GraphicElement::load(ElementLoaderContext &context, const Data::GraphicElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);

	return true;
}

bool GraphicElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "cache") {
		result.setBool(_cacheBitmap);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome GraphicElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "cache") {
		DynamicValueWriteBoolHelper::create(&_cacheBitmap, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, result, attrib);
}


void GraphicElement::render(Window *window) {
	bool haveEffect = (_bottomRightBevelShading != 0 || _topLeftBevelShading != 0 || _interiorShading != 0);

	if (!haveEffect) {
		if (_renderProps.getInkMode() == VisualElementRenderProperties::kInkModeDefault || _renderProps.getInkMode() == VisualElementRenderProperties::kInkModeInvisible || _rect.isEmpty()) {
			// Not rendered at all
			_mask.reset();
			return;
		}
	}

	if (!_visible)
		return;

	const bool needsMask = (_renderProps.getShape() != VisualElementRenderProperties::kShapeRect);
	bool needsMaskRedraw = _renderProps.isDirty();

	uint16 width = _rect.width();
	uint16 height = _rect.height();

	if (needsMask) {
		if (!_mask || _mask->w != width || _mask->h != height) {
			_mask.reset();
			_mask.reset(new Graphics::ManagedSurface());
			_mask->create(_rect.width(), _rect.height(), Graphics::PixelFormat::createFormatCLUT8());

			needsMaskRedraw = true;
		}
	} else {
		_mask.reset();
	}

	if (needsMaskRedraw) {
		Common::Array<Common::Point> starPoints;
		const Common::Array<Common::Point> *polyPoints = nullptr;

		VisualElementRenderProperties::Shape shape = _renderProps.getShape();
		if (shape == VisualElementRenderProperties::kShapeStar) {
			starPoints.resize(10);
			starPoints[0] = Common::Point(width / 2, 0);
			starPoints[1] = Common::Point(width * 2 / 3, height / 3);
			starPoints[2] = Common::Point(width, height / 3);
			starPoints[3] = Common::Point(width * 3 / 4, height / 2);
			starPoints[4] = Common::Point(width, height);
			starPoints[5] = Common::Point(width / 2, height * 2 / 3);
			starPoints[6] = Common::Point(0, height);
			starPoints[7] = Common::Point(width / 4, height / 2);
			starPoints[8] = Common::Point(0, height / 3);
			starPoints[9] = Common::Point(width / 3, height / 3);
			polyPoints = &starPoints;

			shape = VisualElementRenderProperties::kShapePolygon;
		} else if (shape == VisualElementRenderProperties::kShapePolygon) {
			polyPoints = &_renderProps.getPolyPoints();
		}

		// Notes for future:
		// Rounded rect corner arc size is fixed at 13x13 unless the graphic is smaller.

		// TODO: Overhaul this again to be more accurate, it was designed to work "OpenGL-style"
		// where a point exactly on an edge would be excluded on a right/bottom edge to ensure
		// that the canvas puzzle in Obsidian worked correctly, but it turns out that the canvas
		// puzzle only uses polys for the mouse collision and uses a special shape ID for the
		// triangles.  This most likely is supposed to work "QuickDraw style" where the pixel
		// coordinates of the poly points are always included.
		//
		// Maybe it doesn't matter, but if we find a game in the future with polygons, and
		// there are 1-pixel gaps or something.... that's why.  And that's what needs to be fixed.
		//
		// In fact, because of that, we should probably just use ScummVM's draw routines instead
		// of this code here.  Sigh.

		if (shape == VisualElementRenderProperties::kShapePolygon && polyPoints->size() >= 3) {
			_mask->clear(0);

			Common::Point firstPoint = (*polyPoints)[0];
			for (uint polyStart = 1; polyStart < polyPoints->size() - 1; polyStart++) {
				Common::Point points[3];
				points[0] = firstPoint;
				points[1] = (*polyPoints)[polyStart];
				points[2] = (*polyPoints)[polyStart + 1];

				// Sort poly points into height ascending order
				for (int sortStart = 0; sortStart < 2; sortStart++) {
					Common::Point *thisPoint = &points[sortStart];
					Common::Point *lowestY = thisPoint;
					for (int candidateIndex = sortStart + 1; candidateIndex < 3; candidateIndex++) {
						Common::Point *candidate = &points[candidateIndex];
						if (candidate->y < lowestY->y)
							lowestY = candidate;
					}

					if (lowestY != thisPoint) {
						Common::Point temp = *thisPoint;
						*thisPoint = *lowestY;
						*lowestY = temp;
					}
				}

				if (points[0].y == points[2].y)
					continue; // Degenerate triangle

				// Bin into 2 sets
				Common::Point *triPoints[2][3] = {{&points[0], &points[1], &points[2]},
											{&points[2], &points[1], &points[0]}};

				int32 yRanges[2][2] = {{points[0].y, points[1].y},
									   {points[1].y, points[2].y}};

				for (int half = 0; half < 2; half++) {
					Common::Point *commonPoint = triPoints[half][0];
					Common::Point *leftVert = triPoints[half][1];
					Common::Point *rightVert = triPoints[half][2];

					if (leftVert->x > rightVert->x) {
						Common::Point *temp = leftVert;
						leftVert = rightVert;
						rightVert = temp;
					}

					int32 minY = yRanges[half][0];
					if (minY < 0)
						minY = 0;
					int32 maxY = yRanges[half][1];
					if (maxY > static_cast<int32>(height))
						maxY = height;

					// Compute scanline rays
					// In theory we'd want rays that are x=y*scale+const
					// But since we're operating on pixel center space, what we actually want is
					// x=(y+0.5)*scale+const-0.5
					int32 rayScaleNum[2];
					int32 rayConstNum[2];
					int32 rayDenom[2];

					int32 verts[2][2] = {{leftVert->x, leftVert->y},
										 {rightVert->x, rightVert->y}};

					for (int ray = 0; ray < 2; ray++) {
						int32 x0 = verts[ray][0];
						int32 y0 = verts[ray][1];
						int32 x1 = commonPoint->x;
						int32 y1 = commonPoint->y;

						// Compute the base function for:
						// x0=y0*scale+const
						// x1=y1*scale+const
						// Where a=x0, b=y0, c=x1, d=y1, x=scale, y=const
						rayScaleNum[ray] = x0 - x1;
						rayConstNum[ray] = y0 * x1 - x0 * y1;
						rayDenom[ray] = y0 - y1;

						// Half-pixel nudge y: x=(y+1/2)*scale+const-1/2
						// x = (y*scale + 1/2*scale + const)/denom - 1/2
						// x = (y*2*scale + scale + 2*const - denom)/2*denom
						rayConstNum[ray] = 2 * rayConstNum[ray] + rayScaleNum[ray] - rayDenom[ray];
						rayScaleNum[ray] *= 2;
						rayDenom[ray] *= 2;

						// Ensure the denominator is positive
						if (rayDenom[ray] < 0) {
							rayDenom[ray] = -rayDenom[ray];
							rayScaleNum[ray] = -rayScaleNum[ray];
							rayConstNum[ray] = -rayConstNum[ray];
						}
					}

					for (int32 y = minY; y < maxY; y++) {
						int32 xSpan[2];
						for (int32 ray = 0; ray < 2; ray++) {
							int32 xNum = y * rayScaleNum[ray] + rayConstNum[ray];
							// Round up.  If x < 0 then the divide will be towards zero (up)
							if (xNum >= 0)
								xNum += rayDenom[ray] - 1;

							int32 resolved = xNum / rayDenom[ray];
							if (resolved < 0)
								resolved = 0;
							else if (resolved > width)
								resolved = width;

							xSpan[ray] = resolved;
						}

						if (xSpan[1] < xSpan[0]) {
							int32 temp = xSpan[1];
							xSpan[1] = xSpan[0];
							xSpan[0] = temp;
						}

						// Clip to the graphic area
						if (y >= 0 && y < static_cast<int32>(height)) {
							for (int i = 0; i < 2; i++) {
								int32 &xVal = xSpan[i];
								if (xVal < 0)
									xVal = 0;
								if (xVal >= static_cast<int32>(width))
									xVal = width - 1;
							}

							int32 spanWidth = xSpan[1] - xSpan[0];
							uint8 *bits = static_cast<uint8 *>(_mask->getBasePtr(xSpan[0], y));
							for (int32 i = 0; i < spanWidth; i++)
								bits[i] ^= 0xff;
						}
					}
				}
			}
		} else if (shape == VisualElementRenderProperties::kShapeObsidianCanvasPuzzleTri1) {
			// Upper-left right angle tri
			_mask->clear(0);
			for (int32 y = 0; y < height; y++) {
				uint8 *scanline = static_cast<uint8 *>(_mask->getBasePtr(0, y));
				int32 lineStart = 0;
				int32 lineEnd = 64 - y;
				for (int32 x = lineStart; x < lineEnd; x++)
					scanline[x] = 0xff;
			}
		} else if (shape == VisualElementRenderProperties::kShapeObsidianCanvasPuzzleTri2) {
			// Lower-left right-angle tri
			_mask->clear(0);
			for (int32 y = 0; y < height; y++) {
				uint8 *scanline = static_cast<uint8 *>(_mask->getBasePtr(0, y));
				int32 lineStart = 0;
				int32 lineEnd = y;
				for (int32 x = lineStart; x < lineEnd; x++)
					scanline[x] = 0xff;
			}
		} else if (shape == VisualElementRenderProperties::kShapeObsidianCanvasPuzzleTri3) {
			// Upper-right right-angle tri
			_mask->clear(0);
			for (int32 y = 0; y < height; y++) {
				uint8 *scanline = static_cast<uint8 *>(_mask->getBasePtr(0, y));
				int32 lineStart = y;
				int32 lineEnd = 64;
				for (int32 x = lineStart; x < lineEnd; x++)
					scanline[x] = 0xff;
			}
		} else if (shape == VisualElementRenderProperties::kShapeObsidianCanvasPuzzleTri4) {
			// Lower-right right-angle tri
			_mask->clear(0);
			for (int32 y = 0; y < height; y++) {
				uint8 *scanline = static_cast<uint8 *>(_mask->getBasePtr(0, y));
				int32 lineStart = 64 - y;
				int32 lineEnd = 64;
				for (int32 x = lineStart; x < lineEnd; x++)
					scanline[x] = 0xff;
			}
		} else if (shape != VisualElementRenderProperties::kShapeRect) {
			warning("Unimplemented graphic shape");
			return;
		}
	}

	Common::Rect srcRect = Common::Rect(0, 0, _rect.width(), _rect.height());
	Common::Rect drawRect = srcRect;
	drawRect.translate(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y);

	Common::Rect windowRect = Common::Rect(0, 0, window->getWidth(), window->getHeight());
	Common::Rect clippedDrawRect = drawRect.findIntersectingRect(windowRect);

	Common::Rect clippedSrcRect = srcRect;
	clippedSrcRect.left += clippedDrawRect.left - drawRect.left;
	clippedSrcRect.top += clippedDrawRect.top - drawRect.top;
	clippedSrcRect.right += clippedDrawRect.right - drawRect.right;
	clippedSrcRect.bottom += clippedDrawRect.bottom - drawRect.bottom;

	if (clippedSrcRect.isEmpty())
		return;

	int32 srcToDestY = clippedDrawRect.top - clippedSrcRect.top;

	switch (_renderProps.getInkMode()) {
	case VisualElementRenderProperties::kInkModeBackgroundTransparent:
	case VisualElementRenderProperties::kInkModeBackgroundMatte:
		// Background transparent and background matte seem to have no effect on simple graphics,
		// even if the foreground and background color are the same
	case VisualElementRenderProperties::kInkModeCopy: {
			const Graphics::PixelFormat &pixFmt = window->getPixelFormat();
			const ColorRGB8 fillColorRGB8 = _renderProps.getForeColor();
			uint32 fillColor = pixFmt.ARGBToColor(255, fillColorRGB8.r, fillColorRGB8.g, fillColorRGB8.b);

			for (int32 srcY = clippedSrcRect.top; srcY < clippedSrcRect.bottom; srcY++) {
				int32 spanWidth = clippedDrawRect.width();
				void *destPixels = window->getSurface()->getBasePtr(clippedDrawRect.left, srcY + srcToDestY);
				if (_mask) {
					const uint8 *maskBytes = static_cast<const uint8 *>(_mask->getBasePtr(clippedSrcRect.left, srcY));
					if (pixFmt.bytesPerPixel == 1) {
						for (int32 x = 0; x < spanWidth; x++) {
							if (maskBytes[x])
								static_cast<uint8 *>(destPixels)[x] = fillColor;
						}
					} else if (pixFmt.bytesPerPixel == 2) {
						for (int32 x = 0; x < spanWidth; x++) {
							if (maskBytes[x])
								static_cast<uint16 *>(destPixels)[x] = fillColor;
						}
					} else if (pixFmt.bytesPerPixel == 4) {
						for (int32 x = 0; x < spanWidth; x++) {
							if (maskBytes[x])
								static_cast<uint32 *>(destPixels)[x] = fillColor;
						}
					}
				} else {
					if (pixFmt.bytesPerPixel == 1) {
						for (int32 x = 0; x < spanWidth; x++)
							static_cast<uint8 *>(destPixels)[x] = fillColor;
					} else if (pixFmt.bytesPerPixel == 2) {
						for (int32 x = 0; x < spanWidth; x++)
							static_cast<uint16 *>(destPixels)[x] = fillColor;
					} else if (pixFmt.bytesPerPixel == 4) {
						for (int32 x = 0; x < spanWidth; x++)
							static_cast<uint32 *>(destPixels)[x] = fillColor;
					}
				}
			}
		} break;
	case VisualElementRenderProperties::kInkModeXor: {
			const Graphics::PixelFormat &pixFmt = window->getPixelFormat();
			uint32 colorMask = 0xff;

			if (pixFmt.bytesPerPixel > 1)
				colorMask = pixFmt.ARGBToColor(0, 255, 255, 255);

			for (int32 srcY = clippedSrcRect.top; srcY < clippedSrcRect.bottom; srcY++) {
				int32 spanWidth = clippedDrawRect.width();
				void *destPixels = window->getSurface()->getBasePtr(clippedDrawRect.left, srcY + srcToDestY);
				if (_mask) {
					const uint8 *maskBytes = static_cast<const uint8 *>(_mask->getBasePtr(clippedSrcRect.left, srcY));
					if (pixFmt.bytesPerPixel == 1) {
						for (int32 x = 0; x < spanWidth; x++) {
							if (maskBytes[x])
								static_cast<uint8 *>(destPixels)[x] ^= 0xff;
						}
					} else if (pixFmt.bytesPerPixel == 2) {
						for (int32 x = 0; x < spanWidth; x++) {
							if (maskBytes[x])
								static_cast<uint16 *>(destPixels)[x] ^= colorMask;
						}
					} else if (pixFmt.bytesPerPixel == 4) {
						for (int32 x = 0; x < spanWidth; x++) {
							if (maskBytes[x])
								static_cast<uint32 *>(destPixels)[x] ^= colorMask;
						}
					}
				} else {
					if (pixFmt.bytesPerPixel == 1) {
						for (int32 x = 0; x < spanWidth; x++)
							static_cast<uint8 *>(destPixels)[x] ^= 0xff;
					} else if (pixFmt.bytesPerPixel == 2) {
						for (int32 x = 0; x < spanWidth; x++)
							static_cast<uint16 *>(destPixels)[x] ^= colorMask;
					} else if (pixFmt.bytesPerPixel == 4) {
						for (int32 x = 0; x < spanWidth; x++)
							static_cast<uint32 *>(destPixels)[x] ^= colorMask;
					}
				}
			}
		} break;
	case VisualElementRenderProperties::kInkModeInvisible:
	case VisualElementRenderProperties::kInkModeDefault:
		break;
	default:
		warning("Unimplemented graphic ink mode");
		return;
	}

	// TODO: The accurate behavior for polys is complicated.
	// It looks like the way that it works is that a "line mask" is constructed by inverting the mask, dilating it
	// by HALF the bevel size, then masking that out using the original mask, which results in a border mask.
	// Then, the bevel diagonal is computed as simply a line going from the lower-left corner to the top-right corner.

	if (_interiorShading) {
		const Graphics::PixelFormat &pixFmt = window->getPixelFormat();

		if (pixFmt.bytesPerPixel > 1) {
			uint32 rMask = pixFmt.ARGBToColor(0, 255, 0, 0);
			uint32 gMask = pixFmt.ARGBToColor(0, 0, 255, 0);
			uint32 bMask = pixFmt.ARGBToColor(0, 0, 0, 255);

			uint32 rAdd = quantizeShading(rMask, _interiorShading);
			uint32 gAdd = quantizeShading(gMask, _interiorShading);
			uint32 bAdd = quantizeShading(bMask, _interiorShading);

			bool isBrighten = (_interiorShading > 0);

			Graphics::ManagedSurface *windowSurface = window->getSurface().get();

			for (int32 srcY = clippedSrcRect.top; srcY < clippedSrcRect.bottom; srcY++) {
				int32 spanWidth = clippedDrawRect.width();

				int32 effectLength = 0;

				if (_mask) {
					const uint8 *maskBytes = static_cast<const uint8 *>(_mask->getBasePtr(clippedSrcRect.left, srcY));

					for (int32 x = 0; x < spanWidth; x++) {
						if (maskBytes[x])
							effectLength++;
						else {
							if (effectLength > 0) {
								void *effectPixels = windowSurface->getBasePtr(clippedDrawRect.left + x - effectLength, srcY + srcToDestY);
								renderShadingScanlineDynamic(effectPixels, effectLength, rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, windowSurface->format.bytesPerPixel);
							}
							effectLength = 0;
						}
					}
				} else
					effectLength = spanWidth;  
				
				if (effectLength > 0) {
					void *effectPixels = windowSurface->getBasePtr(clippedDrawRect.left + spanWidth - effectLength, srcY + srcToDestY);
					renderShadingScanlineDynamic(effectPixels, effectLength, rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, windowSurface->format.bytesPerPixel);
				}
			}
		}
	}
}

Common::SharedPtr<Structural> GraphicElement::shallowClone() const {
	return Common::SharedPtr<Structural>(new GraphicElement(*this));
}

void GraphicElement::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	VisualElement::visitInternalReferences(visitor);

}

MovieResizeFilter::~MovieResizeFilter() {
}

MovieElement::MovieElement()
	: _cacheBitmap(false), _alternate(false), _playEveryFrame(false), _reversed(false), /* _haveFiredAtLastCel(false), */
	  /* _haveFiredAtFirstCel(false), */_shouldPlayIfNotPaused(true), _needsReset(true), _currentPlayState(kMediaStateStopped),
	  _assetID(0), _maxTimestamp(0), _timeScale(0), _currentTimestamp(0), _volume(100),
	  _displayFrame(nullptr) {
}

MovieElement::~MovieElement() {
	if (_unloadSignaller)
		_unloadSignaller->removeReceiver(this);
	if (_playMediaSignaller)
		_playMediaSignaller->removeReceiver(this);

	stopSubtitles();
}

bool MovieElement::load(ElementLoaderContext &context, const Data::MovieElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_paused = ((data.elementFlags & Data::ElementFlags::kPaused) != 0);
	_loop = ((data.animationFlags & Data::AnimationFlags::kLoop) != 0);
	_alternate = ((data.animationFlags & Data::AnimationFlags::kAlternate) != 0);
	_playEveryFrame = ((data.animationFlags & Data::AnimationFlags::kPlayEveryFrame) != 0);
	_assetID = data.assetID;
	_volume = data.volume;

	return true;
}

bool MovieElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "range") {
		result.setIntRange(_playRange);
		return true;
	}
	if (attrib == "timevalue") {
		result.setInt(_currentTimestamp);
		return true;
	}
	if (attrib == "timescale") {
		result.setInt(_timeScale);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome MovieElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "range") {
		DynamicValueWriteOrRefAttribFuncHelper<MovieElement, &MovieElement::scriptSetRange, &MovieElement::scriptRangeWriteRefAttribute>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "volume") {
		DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetVolume, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "timevalue") {
		DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetTimestamp, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, result, attrib);
}

CORO_BEGIN_DEFINITION(MovieElement::MovieElementConsumeCommandCoroutine)
	// The reaction to the Play command should be Shown -> Unpaused -> Played
	// At First Cel is NOT fired by Play commands for some reason.
	// The reaction to the Stop command should be Paused -> Hidden -> Stopped
	struct Locals {
		bool wasPaused;
	};

	CORO_BEGIN_FUNCTION
		CORO_IF(Event(EventIDs::kPlay, 0).respondsTo(params->msg->getEvent()))
			locals->wasPaused = params->self->_paused;

			CORO_CALL(ChangeVisibilityCoroutine, params->self, params->runtime, true);

			CORO_CALL(StartPlayingCoroutine, params->self, params->runtime);

			CORO_IF(locals->wasPaused)
				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kUnpause, 0), DynamicValue(), params->self->getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

				CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);
			CORO_END_IF

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kPlay, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));
			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);

			CORO_RETURN;
		CORO_ELSE_IF(Event(EventIDs::kStop, 0).respondsTo(params->msg->getEvent()))
			CORO_IF(!params->self->_paused)
				params->self->stopSubtitles();

				params->self->_paused = true;

				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kPause, 0), DynamicValue(), params->self->getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

				CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);
			CORO_END_IF

			CORO_CALL(ChangeVisibilityCoroutine, params->self, params->runtime, false);

			params->self->_paused = true;
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kPause, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);

			CORO_RETURN;
		CORO_END_IF

		CORO_CALL(VisualElement::VisualElementConsumeCommandCoroutine, params->self, params->runtime, params->msg);
	CORO_END_FUNCTION

CORO_END_DEFINITION

VThreadState MovieElement::asyncConsumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	runtime->getVThread().pushCoroutine<MovieElement::MovieElementConsumeCommandCoroutine>(this, runtime, msg);
	return kVThreadReturn;
}

void MovieElement::activate() {
	Project *project = getRuntime()->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Movie element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() == kAssetTypeMovie) {
		MovieAsset *movieAsset = static_cast<MovieAsset *>(asset.get());
		size_t streamIndex = movieAsset->getStreamIndex();
		int segmentIndex = project->getSegmentForStreamIndex(streamIndex);
		project->openSegmentStream(segmentIndex);
		Common::SeekableReadStream *stream = project->getStreamForSegment(segmentIndex);

		if (!stream) {
			warning("Movie element stream could not be opened");
			return;
		}

		Video::QuickTimeDecoder *qtDecoder = new Video::QuickTimeDecoder();
		qtDecoder->setVolume(_volume * 255 / 100);

		_videoDecoder.reset(qtDecoder);
		_damagedFrames = movieAsset->getDamagedFrames();

		Common::SeekableReadStream *movieDataStream;

		if (movieAsset->getMovieDataSize() > 0) {
			qtDecoder->setChunkBeginOffset(movieAsset->getMovieDataPos());
			movieDataStream = new Common::SafeSeekableSubReadStream(stream, movieAsset->getMovieDataPos(), movieAsset->getMovieDataPos() + movieAsset->getMovieDataSize(), DisposeAfterUse::NO);
		} else if (!movieAsset->getExtFileName().empty()) {
			Common::File *file = new Common::File();

			if (!file->open(Common::Path(Common::String("VIDEO/") + movieAsset->getExtFileName()))) {
				warning("Movie asset could not be opened: %s", movieAsset->getExtFileName().c_str());
				delete file;
				_videoDecoder.reset();
				return;
			}

			movieDataStream = file;
		} else {
			// If no data size, the movie data is all over the file and the MOOV atom may be after it.
			movieDataStream = new Common::SafeSeekableSubReadStream(stream, 0, stream->size(), DisposeAfterUse::NO);
			movieDataStream->seek(movieAsset->getMoovAtomPos());
		}

		if (!_videoDecoder->loadStream(movieDataStream))
			_videoDecoder.reset();
		else {
			if (getRuntime()->getHacks().removeQuickTimeEdits)
				qtDecoder->flattenEditLists();

			_timeScale = qtDecoder->getTimeScale();

			_maxTimestamp = qtDecoder->getDuration().convertToFramerate(qtDecoder->getTimeScale()).totalNumberOfFrames();
		}

		_unloadSignaller = project->notifyOnSegmentUnload(segmentIndex, this);
	} else if (asset->getAssetType() == kAssetTypeAVIMovie) {
		AVIMovieAsset *aviAsset = static_cast<AVIMovieAsset *>(asset.get());

		Common::File *f = new Common::File();
		if (!f->open(Common::Path(Common::String("VIDEO/") + aviAsset->getExtFileName()))) {
			warning("Movie asset could not be opened");
			delete f;
			return;
		}

		Video::AVIDecoder *aviDec = new Video::AVIDecoder();
		aviDec->setVolume(_volume * 255 / 100);

		_videoDecoder.reset(aviDec);

		if (!_videoDecoder->loadStream(f))
			_videoDecoder.reset();
		else {
			_timeScale = 1000;
			_maxTimestamp = aviDec->getDuration().convertToFramerate(1000).totalNumberOfFrames();
		}
	} else {
		warning("Movie element referenced a non-movie asset");
		return;
	}

	_playMediaSignaller = project->notifyOnPlayMedia(this);

	_playRange = IntRange(0, 0);
	_currentTimestamp = 0;

	if (_name.empty())
		_name = project->getAssetNameByID(_assetID);

	const SubtitleTables &subtitleTables = project->getSubtitles();
	if (subtitleTables.assetMapping) {
		const Common::String *subSetIDPtr = subtitleTables.assetMapping->findSubtitleSetForAssetID(_assetID);
		if (!subSetIDPtr) {
			Common::String assetName = project->getAssetNameByID(_assetID);
			subSetIDPtr = subtitleTables.assetMapping->findSubtitleSetForAssetName(assetName);
		}

		if (subSetIDPtr)
			_subtitles.reset(new SubtitlePlayer(getRuntime(), *subSetIDPtr, subtitleTables));
	}
}

void MovieElement::deactivate() {
	if (_unloadSignaller) {
		_unloadSignaller->removeReceiver(this);
		_unloadSignaller.reset();
	}
	if (_playMediaSignaller) {
		_playMediaSignaller->removeReceiver(this);
		_playMediaSignaller.reset();
	}

	_videoDecoder.reset();
}

bool MovieElement::canAutoPlay() const {
	return _visible && !_paused;
}

void MovieElement::queueAutoPlayEvents(Runtime *runtime, bool isAutoPlaying) {
	// At First Cel event fires even if the movie isn't playing, and it fires before Played
	if (_visible) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kAtFirstCel, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		runtime->queueMessage(dispatch);
	}

	VisualElement::queueAutoPlayEvents(runtime, isAutoPlaying);
}

void MovieElement::render(Window *window) {
	const IntRange realRange = computeRealRange();

	if (!_videoDecoder)
		return;

	if (_needsReset) {
		_videoDecoder->setReverse(_reversed);
		_videoDecoder->seek(Audio::Timestamp(0, _timeScale).addFrames(_currentTimestamp));
		_videoDecoder->setEndTime(Audio::Timestamp(0, _timeScale).addFrames(_reversed ? realRange.min : realRange.max));
		const Graphics::Surface *decodedFrame = _videoDecoder->decodeNextFrame();
		if (decodedFrame) {
			_displayFrame = decodedFrame;
			_scaledFrame.reset();
		}

		_needsReset = false;
	}

	if (_displayFrame) {
		const Graphics::Surface *displaySurface = _displayFrame;
		if (_resizeFilter) {
			if (!_scaledFrame)
				_scaledFrame = _resizeFilter->scaleFrame(*_displayFrame, _currentTimestamp);
			displaySurface = _scaledFrame->surfacePtr();
		}

		Graphics::ManagedSurface *target = window->getSurface().get();
		Common::Rect srcRect(0, 0, displaySurface->w, displaySurface->h);
		Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.width(), _cachedAbsoluteOrigin.y + _rect.height());

		initFallbackPalette();
		target->blitFrom(*displaySurface, srcRect, destRect, _fallbackPalette.get());
	}
}

void MovieElement::playMedia(Runtime *runtime, Project *project) {
	// If this isn't visible, then it wasn't rendered
	if (!_visible)
		return;

	if (_videoDecoder) {
		bool checkContinuously = false;
		if (_shouldPlayIfNotPaused) {
			if (_paused) {
				// Goal state is paused
				if (_videoDecoder->isPlaying() && !_videoDecoder->isPaused()) {
					_videoDecoder->pauseVideo(true);
					_currentPlayState = kMediaStatePaused;
				}
			} else {
				// Goal state is playing
				if (!_videoDecoder->isPlaying())
					_videoDecoder->start();
				if (_videoDecoder->isPaused())
					_videoDecoder->pauseVideo(false);

				if (_currentPlayState != kMediaStatePlayingLastFrame)
					_currentPlayState = kMediaStatePlaying;
				checkContinuously = true;
			}
		} else {
			// Goal state is stopped
			if (_videoDecoder->isPlaying())
				_videoDecoder->stop();

			_currentPlayState = kMediaStateStopped;
		}

		const IntRange realRange = computeRealRange();

		uint32 minTS = realRange.min;
		uint32 maxTS = realRange.max;
		uint32 targetTS = _currentTimestamp;
		const bool hasFrames = _videoDecoder->getFrameCount() > 0;

		int framesDecodedThisFrame = 0;
		if (_currentPlayState == kMediaStatePlaying) {
			while (_videoDecoder->needsUpdate() && hasFrames) {
				if (_playEveryFrame && framesDecodedThisFrame > 0)
					break;

				if (_damagedFrames.size()) {
					bool frameIsDamaged = false;
					int thisFrameNumber = _videoDecoder->getCurFrame() + framesDecodedThisFrame + 1;

					for (int damagedFrame : _damagedFrames) {
						if (static_cast<int>(damagedFrame) == thisFrameNumber) {
							frameIsDamaged = true;
							break;
						}
					}

					if (frameIsDamaged)
						_videoDecoder->seekToFrame(thisFrameNumber + 1);
				}

				const Graphics::Surface *decodedFrame = _videoDecoder->decodeNextFrame();

				// QuickTimeDecoder doesn't return true for endOfVideo or false for needsUpdate until it
				// tries decoding past the end, so we're assuming that the decoded frame memory stays valid until we
				// actually have a new frame and continuing to use it.
				if (decodedFrame) {
					_contentsDirty = true;
					framesDecodedThisFrame++;
					_displayFrame = decodedFrame;
					_scaledFrame.reset();
					if (_playEveryFrame)
						break;
				}
			}

			if (_videoDecoder->endOfVideo())
				targetTS = _reversed ? realRange.min : realRange.max;
			else
				targetTS = (_videoDecoder->getTime() * _timeScale + 500) / 1000;
		}

		if (framesDecodedThisFrame > 1)
			debug(1, "Perf warning: %i video frames decoded in one frame", framesDecodedThisFrame);

		if (targetTS < minTS)
			targetTS = minTS;
		if (targetTS > maxTS)
			targetTS = maxTS;

		// Sync TS to the end of video if we hit the end

		bool triggerEndEvents = false;

		if (_currentPlayState == kMediaStatePlayingLastFrame)
			triggerEndEvents = true;

		if (targetTS != _currentTimestamp) {
			assert(!_paused);

			// Check media cues
			for (MediaCueState *mediaCue : _mediaCues)
				mediaCue->checkTimestampChange(runtime, _currentTimestamp, targetTS, checkContinuously, true);

			if (_subtitles)
				_subtitles->update(_currentTimestamp * 1000 / _timeScale, targetTS * 1000 / _timeScale);

			_currentTimestamp = targetTS;

			if (_currentTimestamp == maxTS) {
				if (maxTS == _maxTimestamp) {
					// If this play range plays through to the end, then delay end events 1 frame so it has a chance to render
					_currentPlayState = kMediaStatePlayingLastFrame;
				} else
					triggerEndEvents = true;
			}
		}

		if (triggerEndEvents) {
			if (!_loop) {
				_paused = true;
				stopSubtitles();

				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kPause, 0), DynamicValue(), getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
				runtime->queueMessage(dispatch);

				_currentPlayState = kMediaStateStopped;
			}

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kAtLastCel, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
			runtime->queueMessage(dispatch);

			// For some reason, At First Cel isn't fired for movies, even when they loop or are set to timevalue 0
			_videoDecoder->stop();
			_currentPlayState = kMediaStateStopped;

			if (_loop) {
				stopSubtitles();

				_needsReset = true;
				_currentTimestamp = _reversed ? realRange.max : realRange.min;
				_contentsDirty = true;
			}
		}
	}
}

void MovieElement::tryAutoSetName(Runtime *runtime, Project *project) {
	_name = project->getAssetNameByID(_assetID);
}

void MovieElement::setResizeFilter(const Common::SharedPtr<MovieResizeFilter> &filter) {
	_resizeFilter = filter;
}

Common::SharedPtr<Structural> MovieElement::shallowClone() const {
	error("Cloning movie elements is not currently supported");
	return nullptr;
}

void MovieElement::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	VisualElement::visitInternalReferences(visitor);

	error("Cloning movie elements is not currently supported");
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void MovieElement::debugSkipMovies() {
	if (_videoDecoder && !_videoDecoder->endOfVideo()) {
		const IntRange realRange = computeRealRange();

		_videoDecoder->seek(Audio::Timestamp(0, _timeScale).addFrames(_reversed ? realRange.min : realRange.max));
	}
}
#endif

void MovieElement::onSegmentUnloaded(int segmentIndex) {
	_videoDecoder.reset();
}

IntRange MovieElement::computeRealRange() const {
	// The default range for movies is 0..0, which is interpreted as unset
	if (_playRange.min == 0 && _playRange.max == 0)
		return IntRange(0, _maxTimestamp);
	return _playRange;
}

void MovieElement::stopSubtitles() {
	if (_subtitles)
		_subtitles->stop();
}

void MovieElement::initFallbackPalette() {
	if (!_fallbackPalette) {
		const Palette &globalPalette = getRuntime()->getGlobalPalette();
		_fallbackPalette = Common::ScopedPtr<Graphics::Palette>(new Graphics::Palette(globalPalette.getPalette(), globalPalette.kNumColors));
	}
}

void MovieElement::onPauseStateChanged() {
	VisualElement::onPauseStateChanged();

	if (_paused && _subtitles)
		_subtitles->stop();
}

MiniscriptInstructionOutcome MovieElement::scriptSetRange(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kIntegerRange) {
		thread->error("Wrong type for movie element range");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return scriptSetRangeTyped(thread, value.getIntRange());
}

MiniscriptInstructionOutcome MovieElement::scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Wrong type for movie element range");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (asInteger < 0)
		asInteger = 0;
	else if (asInteger > 100)
		asInteger = 100;

	_volume = asInteger;
	if (_videoDecoder)
		_videoDecoder->setVolume(_volume * 255 / 100);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MovieElement::scriptSetTimestamp(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Wrong type for movie element timevalue");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (asInteger < _playRange.min)
		asInteger = _playRange.min;
	else if (asInteger > _playRange.max)
		asInteger = _playRange.max;

	if (asInteger != (int32)_currentTimestamp) {
		thread->getRuntime()->getVThread().pushCoroutine<MovieElement::SeekToTimeCoroutine>(this, getRuntime(), asInteger);
		return kMiniscriptInstructionOutcomeYieldToVThread;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MovieElement::scriptSetRangeStart(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Couldn't set movie element range start");
		return kMiniscriptInstructionOutcomeFailed;
	}

	int32 rangeMax = _playRange.max;
	if (rangeMax < asInteger)
		rangeMax = asInteger;

	return scriptSetRangeTyped(thread, IntRange(asInteger, rangeMax));
}

MiniscriptInstructionOutcome MovieElement::scriptSetRangeEnd(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Couldn't set movie element range end");
		return kMiniscriptInstructionOutcomeFailed;
	}

	int32 rangeMin = _playRange.min;
	if (rangeMin > asInteger)
		rangeMin = asInteger;

	return scriptSetRangeTyped(thread, IntRange(rangeMin, asInteger));
}

MiniscriptInstructionOutcome MovieElement::scriptRangeWriteRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "start") {
		DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetRangeStart, true>::create(this, result);
		return kMiniscriptInstructionOutcomeFailed;
	}
	if (attrib == "end") {
		DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetRangeStart, true>::create(this, result);
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome MovieElement::scriptSetRangeTyped(MiniscriptThread *thread, const IntRange &range) {
	_playRange = range;

	if (_playRange.min < 0)
		_playRange.min = 0;

	if (_playRange.max > (int32)_maxTimestamp)
		_playRange.max = _maxTimestamp;

	// Tested that this is the correct logic for inverted ranges
	if (_playRange.max < _playRange.min)
		_playRange.min = _playRange.max;

	uint32 minTS = _playRange.min;
	uint32 maxTS = _playRange.max;
	uint32 targetTS = _currentTimestamp;

	// If the current timestamp is out of range then it goes to the start of the range
	// Obsidian needs this for the cube maze security booth
	if (targetTS < minTS || targetTS > maxTS)
		targetTS = _reversed ? maxTS : minTS;

	if (targetTS != _currentTimestamp) {
		thread->getRuntime()->getVThread().pushCoroutine<MovieElement::SeekToTimeCoroutine>(this, getRuntime(), targetTS);
		return kMiniscriptInstructionOutcomeYieldToVThread;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

CORO_BEGIN_DEFINITION(MovieElement::StartPlayingCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		MovieElement *self = params->self;

		if (self->_videoDecoder) {
			self->_videoDecoder->stop();
			self->_currentPlayState = kMediaStateStopped;
			self->_needsReset = true;
			self->_contentsDirty = true;
			self->_currentTimestamp = self->_reversed ? self->_playRange.max : self->_playRange.min;

			self->_shouldPlayIfNotPaused = true;
			self->_paused = false;

			self->stopSubtitles();
		}
	CORO_END_FUNCTION
CORO_END_DEFINITION


CORO_BEGIN_DEFINITION(MovieElement::SeekToTimeCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		MovieElement *self = params->self;

		uint32 minTS = self->_playRange.min;
		uint32 maxTS = self->_playRange.max;

		uint32 targetTS = params->timestamp;

		if (targetTS < minTS)
			targetTS = minTS;
		if (targetTS > maxTS)
			targetTS = maxTS;

		if (targetTS != self->_currentTimestamp) {
			self->_currentTimestamp = targetTS;
			if (self->_videoDecoder) {
				self->_videoDecoder->stop();
				self->_currentPlayState = kMediaStateStopped;
			}
			self->_needsReset = true;
			self->_contentsDirty = true;

			self->stopSubtitles();
		}
	CORO_END_FUNCTION
CORO_END_DEFINITION

ImageElement::ImageElement() : _cacheBitmap(false), _assetID(0) {
}

ImageElement::~ImageElement() {
}

bool ImageElement::load(ElementLoaderContext &context, const Data::ImageElement &data) {
	if (!VisualElement::loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_assetID = data.imageAssetID;

	return true;
}

bool ImageElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "text") {
		// Obsidian accesses this on an image element in the menus, and if it fails, the "save first" warning
		// prompt buttons aren't layered correctly?
		result.setString(_text);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome ImageElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "text") {
		DynamicValueWriteStringHelper::create(&_text, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "flushpriority") {
		DynamicValueWriteFuncHelper<ImageElement, &ImageElement::scriptSetFlushPriority, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

void ImageElement::activate() {
	Project *project = getRuntime()->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Image element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeImage) {
		warning("Image element assigned an asset that isn't an image");
		return;
	}

	_cachedImage = static_cast<ImageAsset *>(asset.get())->loadAndCacheImage(getRuntime());

	if (_name.empty())
		_name = project->getAssetNameByID(_assetID);
}

void ImageElement::deactivate() {
	_cachedImage.reset();
}

void ImageElement::tryAutoSetName(Runtime *runtime, Project *project) {
	_name = project->getAssetNameByID(_assetID);
}

void ImageElement::render(Window *window) {
	if (_cachedImage) {
		VisualElementRenderProperties::InkMode inkMode = _renderProps.getInkMode();

		if (inkMode == VisualElementRenderProperties::kInkModeInvisible)
			return;

		Common::SharedPtr<Graphics::ManagedSurface> optimized = _cachedImage->optimize(getRuntime());
		Common::Rect srcRect(optimized->w, optimized->h);
		Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.width(), _cachedAbsoluteOrigin.y + _rect.height());

		if (optimized->format.bytesPerPixel == 1) {
			// FIXME: Pass palette to blit functions instead
			if (_cachedImage->getOriginalColorDepth() == kColorDepthMode1Bit) {
				const uint8 bwPalette[2 * 3] = {
					255, 255, 255,
					0, 0, 0
				};
				optimized->setPalette(bwPalette, 0, 2);
			} else {
				const Palette *palette = getPalette().get();
				if (!palette)
					palette = &getRuntime()->getGlobalPalette();

				optimized->setPalette(palette->getPalette(), 0, 256);
			}
		}

		uint8 alpha = _transitionProps.getAlpha();

		Graphics::Surface *postShadingSource = optimized->surfacePtr();

		Graphics::Surface tempSurface;
		if (_interiorShading != 0 || (_bevelSize > 0 && (_bottomRightBevelShading != 0 || _topLeftBevelShading != 0))) {
			tempSurface.copyFrom(*postShadingSource);
			renderShading(tempSurface);
			postShadingSource = &tempSurface;
		}


		if (inkMode == VisualElementRenderProperties::kInkModeBackgroundMatte || inkMode == VisualElementRenderProperties::kInkModeBackgroundTransparent) {
			const ColorRGB8 transColorRGB8 = _renderProps.getBackColor();
			uint32 transColor = optimized->format.ARGBToColor(0, transColorRGB8.r, transColorRGB8.g, transColorRGB8.b);

			// Awful hack to work around transBlit not working with either 0 or -1
			if (transColor == 0)
				transColor = optimized->format.ARGBToColor(255, transColorRGB8.r, transColorRGB8.g, transColorRGB8.b);

			window->getSurface()->transBlitFrom(*optimized, srcRect, destRect, transColor, false, alpha);
		} else if (inkMode == VisualElementRenderProperties::kInkModeDefault || inkMode == VisualElementRenderProperties::kInkModeCopy) {
			if (alpha != 255) {
				warning("Alpha fade was applied to a default or copy image, this isn't supported yet");
				_transitionProps.setAlpha(255);
			}

			window->getSurface()->blitFrom(*optimized, srcRect, destRect);
		} else {
			warning("Unimplemented image ink mode");
		}
	}
}

Common::SharedPtr<Structural> ImageElement::shallowClone() const {
	return Common::SharedPtr<Structural>(new ImageElement(*this));
}

void ImageElement::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	VisualElement::visitInternalReferences(visitor);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ImageElement::debugInspect(IDebugInspectionReport *report) const {
	VisualElement::debugInspect(report);

	if (report->declareStatic("assetID"))
		report->declareStaticContents(Common::String::format("%i", static_cast<int>(_assetID)));
}
#endif

MiniscriptInstructionOutcome ImageElement::scriptSetFlushPriority(MiniscriptThread *thread, const DynamicValue &value) {
	// We don't support flushing media, and this value isn't readable, so just discard it
	return kMiniscriptInstructionOutcomeContinue;
}

MToonElement::MToonElement()
	: _cacheBitmap(false), _maintainRate(false), _assetID(0), _rateTimes100000(0), _flushPriority(0), _celStartTimeMSec(0),
	  _isPlaying(false), _isStopped(false), _renderedFrame(0), _playRange(IntRange(1, 1)), _cel(1), _hasIssuedRenderWarning(false) {
}

MToonElement::MToonElement(const MToonElement &other)
	: VisualElement(other), _cacheBitmap(other._cacheBitmap), _maintainRate(other._maintainRate), _assetID(other._assetID)
	, _rateTimes100000(other._rateTimes100000), _flushPriority(other._flushPriority), _celStartTimeMSec(other._celStartTimeMSec)
	, _isPlaying(other._isPlaying), _isStopped(other._isStopped), _renderSurface(nullptr), _renderedFrame(0), _metadata(other._metadata)
	, _cachedMToon(other._cachedMToon), _playMediaSignaller(nullptr), _playRange(other._playRange), _cel(other._cel), _hasIssuedRenderWarning(false) {
	_playMediaSignaller = other.getRuntime()->getProject()->notifyOnPlayMedia(this);
}

MToonElement::~MToonElement() {
	if (_playMediaSignaller)
		_playMediaSignaller->removeReceiver(this);
}

bool MToonElement::load(ElementLoaderContext &context, const Data::MToonElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_paused = ((data.elementFlags & Data::ElementFlags::kPaused) != 0);
	_loop = ((data.animationFlags & Data::AnimationFlags::kLoop) != 0);
	_maintainRate = ((data.elementFlags & Data::AnimationFlags::kPlayEveryFrame) == 0);	// NOTE: Inverted intentionally
	_assetID = data.assetID;
	_rateTimes100000 = data.rateTimes100000;

	return true;
}

bool MToonElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "cel") {
		result.setInt(_cel);
		return true;
	} else if (attrib == "flushpriority") {
		result.setInt(_flushPriority);
		return true;
	} else if (attrib == "rate") {
		result.setFloat(_rateTimes100000 / 100000.0);
		return true;
	} else if (attrib == "range") {
		result.setIntRange(_playRange);
		return true;
	} else if (attrib == "celcount") {
		if (_cachedMToon)
			result.setInt(_cachedMToon->getMetadata()->frames.size());
		else
			result.setInt(0);
		return true;
	} else if (attrib == "regpoint") {
		result.setPoint(_cachedMToon->getMetadata()->registrationPoint);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome MToonElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "cel") {
		DynamicValueWriteFuncHelper<MToonElement, &MToonElement::scriptSetCel, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "flushpriority") {
		DynamicValueWriteIntegerHelper<int32>::create(&_flushPriority, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "maintainrate") {
		DynamicValueWriteBoolHelper::create(&_maintainRate, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "rate") {
		DynamicValueWriteFuncHelper<MToonElement, &MToonElement::scriptSetRate, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "range") {
		DynamicValueWriteOrRefAttribFuncHelper<MToonElement, &MToonElement::scriptSetRange, &MToonElement::scriptRangeWriteRefAttribute>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, result, attrib);
}

CORO_BEGIN_DEFINITION(MToonElement::MToonConsumeCommandCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		CORO_IF (Event(EventIDs::kPlay, 0).respondsTo(params->msg->getEvent()))
			// If the range set fails, then the mToon should play anyway, so ignore the result
			CORO_AWAIT_MINISCRIPT(miniscriptIgnoreFailure(params->self->scriptSetRange(nullptr, params->msg->getValue())));

			MToonElement *self = params->self;

			if (self->_isStopped) {
				self->_isStopped = false;
				params->runtime->setSceneGraphDirty();
			}

			CORO_CALL(MToonElement::ChangeVisibilityCoroutine, params->self, params->runtime, true);

			CORO_CALL(MToonElement::StartPlayingCoroutine, params->self, params->runtime);

			CORO_RETURN;
		CORO_ELSE_IF(Event(EventIDs::kStop, 0).respondsTo(params->msg->getEvent()))
			// mTropolis 1.0 will not fire a Hidden event when an mToon is stopped even though it is hidden in the process.
			// MTI depends on this, otherwise 2 hints will play at once when clicking a song button on the piano.
			// This same bug does NOT apply to the "Shown" event firing on Play (as happens above).
			
			MToonElement *self = params->self;
			
			if (params->runtime->getProject()->getRuntimeVersion() <= kRuntimeVersion100)
				self->setVisible(params->runtime, false);

			CORO_CALL(MToonElement::StopPlayingCoroutine, params->self, params->runtime);

			CORO_IF (params->runtime->getProject()->getRuntimeVersion() > kRuntimeVersion100)
				CORO_CALL(MToonElement::ChangeVisibilityCoroutine, params->self, params->runtime, false);
			CORO_END_IF

			CORO_RETURN;
		CORO_END_IF

		CORO_CALL(VisualElement::VisualElementConsumeCommandCoroutine, params->self, params->runtime, params->msg);
	CORO_END_FUNCTION
CORO_END_DEFINITION

VThreadState MToonElement::asyncConsumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	runtime->getVThread().pushCoroutine<MToonElement::MToonConsumeCommandCoroutine>(this, runtime, msg);
	return kVThreadReturn;
}

void MToonElement::activate() {
	Project *project = getRuntime()->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("mToon element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeMToon) {
		warning("mToon element assigned an asset that isn't an mToon");
		return;
	}

	uint hackFlags = 0;

	_cachedMToon = static_cast<MToonAsset *>(asset.get())->loadAndCacheMToon(getRuntime(), hackFlags);
	_metadata = _cachedMToon->getMetadata();

	_playMediaSignaller = project->notifyOnPlayMedia(this);
	_playRange = IntRange(1, _metadata->frames.size());

	if (_name.empty())
		_name = project->getAssetNameByID(_assetID);

	if (_hooks)
		_hooks->onPostActivate(this);
}

void MToonElement::deactivate() {
	if (_playMediaSignaller) {
		_playMediaSignaller->removeReceiver(this);
		_playMediaSignaller.reset();
	}

	_renderSurface.reset();
}

void MToonElement::tryAutoSetName(Runtime *runtime, Project *project) {
	_name = project->getAssetNameByID(_assetID);
}

bool MToonElement::canAutoPlay() const {
	return _visible && !_paused;
}

void MToonElement::render(Window *window) {
	// Stopped mToons are not supposed to render
	// FIXME: Should this also disable mouse collision?  Should we detect ths somewhere else?
	if (_isStopped)
		return;

	if (_cachedMToon) {
		_cachedMToon->optimize(getRuntime());

		uint32 frame = _cel - 1;
		assert(frame < _metadata->frames.size());

		_cachedMToon->getOrRenderFrame(_renderedFrame, frame, _renderSurface);

		const Palette *palette = nullptr;
		if (_renderSurface->format.bytesPerPixel == 1) {
			palette = getPalette().get();
			if (!palette)
				palette = &getRuntime()->getGlobalPalette();

			// FIXME: Should support passing the palette to the blit function instead
			_renderSurface->setPalette(palette->getPalette(), 0, 256);
		}

		_renderedFrame = frame;

		// This is a bit suboptimal since we don't need to render the frame if invisible, but
		// we do need some things here to be up to date because isMouseCollisionAtPoint depends on
		// invisible mToon frames still being clickable.
		VisualElementRenderProperties::InkMode inkMode = _renderProps.getInkMode();

		if (inkMode == VisualElementRenderProperties::kInkModeInvisible)
			return;

		if (_renderSurface) {
			Common::Rect srcRect;
			Common::Rect destRect;

			Common::Rect frameRect = _metadata->frames[frame].rect;

			if (frameRect.width() == _renderSurface->w && frameRect.height() == _renderSurface->h) {
				// Frame rect is the size of the render surface, meaning the frame rect is an offset
				srcRect = Common::Rect(0, 0, frameRect.width(), frameRect.height());
			} else {
				// Frame rect is a sub-area of the rendered rect
				srcRect = Common::Rect(frameRect.left, frameRect.top, frameRect.right, frameRect.bottom);
			}
			destRect = Common::Rect(_cachedAbsoluteOrigin.x + frameRect.left, _cachedAbsoluteOrigin.y + frameRect.top, _cachedAbsoluteOrigin.x + frameRect.right, _cachedAbsoluteOrigin.y + frameRect.bottom);

			if (inkMode == VisualElementRenderProperties::kInkModeBackgroundMatte || inkMode == VisualElementRenderProperties::kInkModeBackgroundTransparent) {
				ColorRGB8 transColorRGB8 = _renderProps.getBackColor();
				uint32 transColor = 0;

				if (_renderSurface->format.bytesPerPixel == 1) {
					assert(palette);

					const byte *paletteData = palette->getPalette();
					bool foundColor = false;
					for (uint i = 0; i < Palette::kNumColors; i++) {
						if (transColorRGB8 == ColorRGB8(paletteData[i * 3 + 0], paletteData[i * 3 + 1], paletteData[i * 3 + 2])) {
							if (foundColor) {
								warning("mToon is rendered color key but has multiple palette entries matching the transparent color, this may not render correctly");
								_hasIssuedRenderWarning = true;
								break;
							} else {
								foundColor = true;
								transColor = i;

								if (_hasIssuedRenderWarning)
									break;
							}
						}
					}
				} else
					transColor = _renderSurface->format.ARGBToColor(255, transColorRGB8.r, transColorRGB8.g, transColorRGB8.b);

				window->getSurface()->transBlitFrom(*_renderSurface, srcRect, destRect, transColor);
			} else if (inkMode == VisualElementRenderProperties::kInkModeCopy || inkMode == VisualElementRenderProperties::kInkModeDefault) {
				window->getSurface()->blitFrom(*_renderSurface, srcRect, destRect);
			} else {
				warning("Unsupported mToon ink mode");
			}
		}
	}
}

bool MToonElement::isMouseCollisionAtPoint(int32 relativeX, int32 relativeY) const {
	relativeX -= _rect.left;
	relativeY -= _rect.top;

	if (_renderSurface) {
		Common::Rect frameRect = _metadata->frames[_renderedFrame].rect;

		if (relativeX < frameRect.left || relativeY < frameRect.top || relativeX >= frameRect.right || relativeY >= frameRect.bottom)
			return false;

		if (_renderProps.getInkMode() == VisualElementRenderProperties::kInkModeBackgroundMatte) {
			// TODO: This doesn't account for scaling
			ColorRGB8 transColorRGB8 = _renderProps.getBackColor();
			uint32 transColor = _renderSurface->format.ARGBToColor(255, transColorRGB8.r, transColorRGB8.g, transColorRGB8.b);

			if (frameRect.width() == _renderSurface->w && frameRect.height() == _renderSurface->h) {
				// Frame rect is the size of the render surface, meaning the frame is floating and we need to adjust to its coordinates
				relativeX -= frameRect.left;
				relativeY -= frameRect.top;
			}
			// ... otherwise it's a sub-area of the rendered rect, meaning we shouldn't adjust coordinates

			// Sanity-check
			if (relativeX < 0 || relativeY < 0 || relativeX >= _renderSurface->w || relativeY >= _renderSurface->h)
				return false;

			// Check if the pixel is transparent
			if (_renderSurface->getPixel(relativeX, relativeY) == transColor)
				return false;
		}

		return true;
	}

	return false;
}

Common::Rect MToonElement::getRelativeCollisionRect() const {
	Common::Rect colRect = _metadata->frames[_renderedFrame].rect;
	colRect.translate(_rect.left, _rect.top);
	return colRect;
}

Common::SharedPtr<Structural> MToonElement::shallowClone() const {
	return Common::SharedPtr<Structural>(new MToonElement(*this));
}

void MToonElement::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	VisualElement::visitInternalReferences(visitor);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void MToonElement::debugInspect(IDebugInspectionReport *report) const {
	VisualElement::debugInspect(report);

	report->declareDynamic("cel", Common::String::format("%i", static_cast<int>(_cel)));
	report->declareDynamic("assetID", Common::String::format("%i", static_cast<int>(_assetID)));
	report->declareDynamic("isPlaying", Common::String::format("%s", _isPlaying ? "true" : "false"));
	report->declareDynamic("renderedFrame", Common::String::format("%i", static_cast<int>(_renderedFrame)));
	report->declareDynamic("playRange", Common::String::format("%i-%i", static_cast<int>(_playRange.min), static_cast<int>(_playRange.max)));
}
#endif

CORO_BEGIN_DEFINITION(MToonElement::StartPlayingCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		MToonElement *self = params->self;

		if (self->_rateTimes100000 < 0)
			self->_cel = self->_playRange.max;
		else
			self->_cel = self->_playRange.min;

		self->_paused = false;
		self->_isPlaying = false; // Reset play state, it starts for real in playMedia

		self->_contentsDirty = true;

		Common::SharedPtr<MessageProperties> playMsgProps(new MessageProperties(Event(EventIDs::kPlay, 0), DynamicValue(), self->getSelfReference()));
		Common::SharedPtr<MessageDispatch> playDispatch(new MessageDispatch(playMsgProps, self, false, true, false));
		CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, playDispatch);

		Common::SharedPtr<MessageProperties> afcMsgProps(new MessageProperties(Event(EventIDs::kAtFirstCel, 0), DynamicValue(), params->self->getSelfReference()));
		Common::SharedPtr<MessageDispatch> afcDispatch(new MessageDispatch(afcMsgProps, params->self, false, true, false));
		CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, afcDispatch);
	CORO_END_FUNCTION
CORO_END_DEFINITION


CORO_BEGIN_DEFINITION(MToonElement::StopPlayingCoroutine)
	struct Locals {
		bool needsStop = false;
	};

	CORO_BEGIN_FUNCTION
		MToonElement *self = params->self;

		self->_contentsDirty = true;
		self->_isPlaying = false;

		locals->needsStop = !self->_isStopped;

		self->_isStopped = true;

		if (self->_hooks)
			self->_hooks->onStopPlayingMToon(self, self->_visible, self->_isStopped, self->_renderSurface.get());

		CORO_IF (locals->needsStop)
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kStop, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));
			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);
		CORO_END_IF
	CORO_END_FUNCTION
CORO_END_DEFINITION

void MToonElement::playMedia(Runtime *runtime, Project *project) {
	if (_paused)
		return;

	// TODO: This is semi-accurate: mTropolis Player will advance mToon time while
	// the element is hidden and can then fire a barrage of frame advances and events
	// if it's revealed again.  However, we're not fully handling that here yet,
	// and we actually miss events if frame advance overruns the last cel, which can
	// cause problems sometimes (e.g. lag in the Spider air puzzle in Obsidian when
	// the board is revealed)
	if (!_visible)
		return;

	int32 minCel = _playRange.min;
	int32 maxCel = _playRange.max;
	int32 sanitizeMaxCel = _metadata->frames.size();
	int32 targetCel = _cel;

	uint64 playTime = runtime->getPlayTime();
	if (!_isPlaying) {
		_isPlaying = true;
		_celStartTimeMSec = runtime->getPlayTime();
	}

	const bool isReversed = (_rateTimes100000 < 0);
	uint32 absRateTimes100000;
	if (isReversed)
		absRateTimes100000 = -_rateTimes100000;
	else
		absRateTimes100000 = _rateTimes100000;

	// Might be possible due to drift?
	if (playTime < _celStartTimeMSec)
		return;

	uint64 timeSinceCelStart = playTime - _celStartTimeMSec;
	uint64 framesAdvanced = timeSinceCelStart * static_cast<uint64>(absRateTimes100000) / static_cast<uint64>(100000000);

	if (framesAdvanced > 0) {
		// This needs to be handled correctly: Reaching the last frame triggers At Last Cel or At First Cel,
		// but going PAST the end frame triggers automatic stop and pause. The Obsidian bureau filing cabinets
		// depend on this, since they reset the cel when reaching the last cel but do not unpause.

		// There's actually some weird stuff we don't handle here where the play control range is invalid, in
		// which case the timing of "at last cel"/"at first cel" triggers based on where the timer would be
		// in the invalid range, so mTropolis Player apparently keeps a play cel independent of the actual
		// cel?

		bool ranPastEnd = false;

		size_t framesRemainingToOnePastEnd = isReversed ? (_cel - minCel + 1) : (maxCel + 1 - _cel);
		bool alreadyAtLastCel = (framesRemainingToOnePastEnd == 1);
		if (framesRemainingToOnePastEnd <= framesAdvanced) {
			ranPastEnd = true;
			if (_loop)
				targetCel = isReversed ? maxCel : minCel;
			else
				targetCel = isReversed ? minCel : maxCel;
		} else
			targetCel = isReversed ? (_cel - framesAdvanced) : (_cel + framesAdvanced);

		if (targetCel < 1)
			targetCel = 1;
		if (targetCel > sanitizeMaxCel)
			targetCel = sanitizeMaxCel;

		if (_cel != targetCel) {
			_cel = targetCel;
			_contentsDirty = true;
		}

		// Events play control events even if no cel advance occurs
		bool atFirstCel = (targetCel == (isReversed ? maxCel : minCel));
		bool atLastCel = (targetCel == (isReversed ? minCel : maxCel)) && !(ranPastEnd && alreadyAtLastCel);

		if (atFirstCel) {
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kAtFirstCel, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
			runtime->queueMessage(dispatch);
		} else if (atLastCel) {		// These can not fire from the same frame transition (see notes)
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kAtLastCel, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
			runtime->queueMessage(dispatch);
		}

		if (ranPastEnd && !_loop) {
			_paused = true;

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kPause, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
			runtime->queueMessage(dispatch);
		}

		if (_maintainRate && !runtime->getHacks().ignoreMToonMaintainRateFlag)
			_celStartTimeMSec = playTime;
		else
			_celStartTimeMSec += (static_cast<uint64>(100000000) * framesAdvanced) / absRateTimes100000;
	}
}

MiniscriptInstructionOutcome MToonElement::scriptSetCel(MiniscriptThread *thread, const DynamicValue &value) {
	int32 newCel = 0;
	if (!value.roundToInt(newCel)) {
		thread->error("Attempted to set mToon cel to an invalid value");
		return kMiniscriptInstructionOutcomeFailed;
	}

	int32 maxCel = _metadata->frames.size();

	// Intentionally ignore play range.  The cel may be set to an out-of-range cel here and will
	// in fact play from that cel even if it's out of range.  The mariachi hint room near the
	// Bureau booths in Obsidian depends on this behavior, since it sets the mToon cel and then
	// sets the range based on the cel value.
	//
	// We also need to loop around to 1 (exactly) if the range is exceeded.  MTI depends on this
	// in the piano to loop the sound bank display mToon.
	if (newCel < 1 || newCel > maxCel)
		newCel = 1;

	if (newCel != _cel) {
		_cel = newCel;
		_contentsDirty = true;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MToonElement::scriptSetRange(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kIntegerRange)
		return scriptSetRangeTyped(thread, value.getIntRange());
	if (value.getType() == DynamicValueTypes::kPoint)
		return scriptSetRangeTyped(thread, value.getPoint());
	if (value.getType() == DynamicValueTypes::kLabel)
		return scriptSetRangeTyped(thread, value.getLabel());

	if (thread)
		thread->error("Invalid type for mToon range");

	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome MToonElement::scriptSetRangeStart(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Invalid type for mToon range start");
		return kMiniscriptInstructionOutcomeFailed;
	}

	IntRange range = _playRange;
	range.min = asInteger;
	return scriptSetRangeTyped(thread, range);
}

MiniscriptInstructionOutcome MToonElement::scriptSetRangeEnd(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Invalid type for mToon range start");
		return kMiniscriptInstructionOutcomeFailed;
	}

	IntRange range = _playRange;
	range.max = asInteger;
	return scriptSetRangeTyped(thread, range);
}

MiniscriptInstructionOutcome MToonElement::scriptRangeWriteRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "start") {
		DynamicValueWriteFuncHelper<MToonElement, &MToonElement::scriptSetRangeStart, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "end") {
		DynamicValueWriteFuncHelper<MToonElement, &MToonElement::scriptSetRangeEnd, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome MToonElement::scriptSetRangeTyped(MiniscriptThread *thread, const IntRange &intRangeRef) {
	IntRange intRange = intRangeRef;

	int32 maxFrame = _metadata->frames.size();

	// Intentionally buggy sanitization, see notes.
	const bool isInvertedRange = (intRange.min > intRange.max);

	if (intRange.min < 1)
		intRange.min = 1;
	if (intRange.max > maxFrame)
		intRange.max = maxFrame;

	if (isInvertedRange) {
		// coverity[swapped_arguments]
		_playRange = IntRange(intRange.max, intRange.min);
		if (_rateTimes100000 > 0)
			_rateTimes100000 = -_rateTimes100000;
	} else {
		_playRange = intRange;
		if (_rateTimes100000 < 0)
			_rateTimes100000 = -_rateTimes100000;
	}

	int32 newCel = _cel;
	if (newCel < intRange.min || newCel > intRange.max)
		newCel = intRange.min;

	if (newCel < 1 || newCel > maxFrame)
		newCel = maxFrame;

	if (newCel != _cel) {
		_cel = newCel;
		_contentsDirty = true;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MToonElement::scriptSetRangeTyped(MiniscriptThread *thread, const Common::Point &pointRef) {
	IntRange intRange(pointRef.x, pointRef.y);
	return scriptSetRangeTyped(thread, intRange);
}

MiniscriptInstructionOutcome MToonElement::scriptSetRangeTyped(MiniscriptThread *thread, const Label &label) {
	const Common::String *nameStrPtr = getRuntime()->getProject()->findNameOfLabel(label);
	if (!nameStrPtr) {
		if (thread)
			thread->error("mToon range label wasn't found");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (!_metadata) {
		if (thread)
			thread->error("mToon range couldn't be resolved because the metadata wasn't loaded yet");
		return kMiniscriptInstructionOutcomeFailed;
	}

	for (const MToonMetadata::FrameRangeDef &frameRange : _metadata->frameRanges) {
		if (caseInsensitiveEqual(frameRange.name, *nameStrPtr)) {
			// Frame ranges in the metadata are 0-based, but setting the range is 1-based, so add 1
			return scriptSetRangeTyped(thread, IntRange(frameRange.startFrame + 1, frameRange.endFrame + 1));
		}
	}

	if (thread)
		thread->error("mToon range was assigned to a label but the label doesn't exist in the mToon data");

	return kMiniscriptInstructionOutcomeFailed;
}

void MToonElement::onPauseStateChanged() {
	_celStartTimeMSec = getRuntime()->getPlayTime();
}

MiniscriptInstructionOutcome MToonElement::scriptSetRate(MiniscriptThread *thread, const DynamicValue &value) {
	switch (value.getType()) {
	case DynamicValueTypes::kFloat:
		_rateTimes100000 = static_cast<int32>(round(value.getFloat()) * 100000.0);
		break;
	case DynamicValueTypes::kInteger:
		_rateTimes100000 = value.getInt() * 100000;
		break;
	default:
		thread->error("Invalid type for Miniscript rate");
		return kMiniscriptInstructionOutcomeFailed;
	}
	_celStartTimeMSec = thread->getRuntime()->getPlayTime();

	return kMiniscriptInstructionOutcomeContinue;
}


TextLabelElement::TextLabelElement()
	: _cacheBitmap(false), _needsRender(false), _isBitmap(true), _assetID(0),
	  _macFontID(0), _size(12), _alignment(kTextAlignmentLeft) {
}

TextLabelElement::TextLabelElement(const TextLabelElement &other)
	: VisualElement(other), _cacheBitmap(other._cacheBitmap), _needsRender(other._needsRender), _isBitmap(other._isBitmap)
	, _assetID(other._assetID), _text(other._text), _macFontID(other._macFontID), _fontFamilyName(other._fontFamilyName)
	, _size(other._size), _alignment(other._alignment), _styleFlags(other._styleFlags), _macFormattingSpans(other._macFormattingSpans)
	, _renderedText(nullptr) {

	if (other._isBitmap)
		_renderedText = other._renderedText;
}

TextLabelElement::~TextLabelElement() {
}

bool TextLabelElement::isTextLabel() const {
	return true;
}

bool TextLabelElement::load(ElementLoaderContext &context, const Data::TextLabelElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, 0, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_assetID = data.assetID;

	return true;
}

bool TextLabelElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "text") {
		result.setString(_text);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

bool TextLabelElement::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "line") {
		int32 asInteger = 0;
		if (!index.roundToInt(asInteger) || asInteger < 1) {
			thread->error("Invalid text label line index");
			return false;
		}

		size_t lineIndex = static_cast<size_t>(asInteger) - 1;
		uint32 startPos;
		uint32 endPos;
		if (findLineRange(lineIndex, startPos, endPos))
			result.setString(_text.substr(startPos, endPos - startPos));
		else
			result.setString("");

		return true;
	}

	return VisualElement::readAttributeIndexed(thread, result, attrib, index);
}

MiniscriptInstructionOutcome TextLabelElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "text") {
		DynamicValueWriteFuncHelper<TextLabelElement, &TextLabelElement::scriptSetText, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

MiniscriptInstructionOutcome TextLabelElement::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "line") {
		int32 asInteger = 0;
		if (!index.roundToInt(asInteger) || asInteger < 1) {
			thread->error("Invalid text label line set index");
			return kMiniscriptInstructionOutcomeFailed;
		}

		writeProxy.pod.ifc = DynamicValueWriteInterfaceGlue<TextLabelLineWriteInterface>::getInstance();
		writeProxy.pod.objectRef = this;
		writeProxy.pod.ptrOrOffset = static_cast<uintptr>(asInteger) - 1;
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttributeIndexed(thread, writeProxy, attrib, index);
}

void TextLabelElement::activate() {
	Project *project = getRuntime()->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Text element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeText) {
		warning("Text element assigned an asset that isn't text");
		return;
	}

	TextAsset *textAsset = static_cast<TextAsset *>(asset.get());

	if (textAsset->isBitmap()) {
		_renderedText = textAsset->getBitmapSurface();
		_needsRender = false;
		_isBitmap = true;
	} else {
		_needsRender = true;
		_isBitmap = false;
		_text = textAsset->getString();
		_macFormattingSpans = textAsset->getMacFormattingSpans();
	}
}

void TextLabelElement::deactivate() {
}

void TextLabelElement::render(Window *window) {
	if (!_visible)
		return;

	int renderWidth = _rect.width();
	int renderHeight = _rect.height();
	if (_renderedText) {
		if (renderWidth != _renderedText->w || renderHeight != _renderedText->h)
			_needsRender = true;
	}

	if (_needsRender) {
		_needsRender = false;
		_isBitmap = false;

		_renderedText.reset();
		_renderedText.reset(new Graphics::ManagedSurface());

		_renderedText->create(renderWidth, renderHeight, Graphics::PixelFormat::createFormatCLUT8());
		_renderedText->fillRect(Common::Rect(0, 0, renderWidth, renderHeight), 0);

		const Graphics::Font *font = nullptr;
		if (_fontFamilyName.size() > 0) {
			font = FontMan.getFontByName(_fontFamilyName.c_str());
			if (!font)
				font = FontMan.getFontByUsage(getDefaultUsageForNamedFont(_fontFamilyName, _size));
		} else if (_macFontID != 0) {
			// TODO: Formatting spans
			int slant = 0;
			// FIXME/HACK: These aren't public...
			if (_styleFlags.bold)
				slant |= 1;
			if (_styleFlags.italic)
				slant |= 2;
			if (_styleFlags.underline)
				slant |= 4;
			if (_styleFlags.outline)
				slant |= 8;
			if (_styleFlags.shadow)
				slant |= 16;
			if (_styleFlags.condensed)
				slant |= 32;
			if (_styleFlags.expanded)
				slant |= 64;

			const Graphics::FontManager::FontUsage defaultUsage = getDefaultUsageForMacFont(_macFontID, _size);
			const Graphics::Font *fallback = FontMan.getFontByUsage(defaultUsage);

			Graphics::MacFont macFont(_macFontID, _size, slant);
			macFont.setFallback(fallback);

			font = getRuntime()->getMacFontManager()->getFont(macFont);
		}

		// Some weird cases (like the Immediate Action entryway in Obsidian) have no font info at all
		if (!font)
			font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);

		int height = font->getFontHeight();
		int ascent = font->getFontAscent();

		Graphics::TextAlign textAlign = Graphics::kTextAlignLeft;
		switch (_alignment) {
		case kTextAlignmentLeft:
			textAlign = Graphics::kTextAlignLeft;
			break;
		case kTextAlignmentCenter:
			textAlign = Graphics::kTextAlignCenter;
			break;
		case kTextAlignmentRight:
			textAlign = Graphics::kTextAlignRight;
			break;
		default:
			break;
		}

		int line = 0;
		uint32 lineStart = 0;
		while (lineStart < _text.size()) {
			bool noMoreLines = false;
			uint32 lineEndPos = _text.find('\r', lineStart);
			if (lineEndPos == Common::String::npos) {
				lineEndPos = _text.size();
				noMoreLines = true;
			}

			Common::String lineStr;
			if (lineStart != 0 || lineEndPos != _text.size())
				lineStr = _text.substr(lineStart, lineEndPos - lineStart);
			else
				lineStr = _text;

			// Split the line into sublines
			while (lineStr.size() > 0) {
				size_t lineCommitted = 0;
				bool prevWasWhitespace = true;
				for (size_t i = 0; i <= lineStr.size(); i++) {
					bool isWhitespace = (i == lineStr.size() || lineStr[i] < ' ');

					if (isWhitespace) {
						if (!prevWasWhitespace) {
							int width = font->getStringWidth(lineStr.substr(0, i));
							if (width > renderWidth)
								break;
						}
						lineCommitted = i + 1;
					}

					prevWasWhitespace = isWhitespace;
				}

				if (lineCommitted > lineStr.size())
					lineCommitted = lineStr.size();

				// Too little space for anything
				if (lineCommitted == 0) {
					lineCommitted = 1;
					for (size_t i = 2; i <= lineStr.size(); i++) {
						int width = font->getStringWidth(lineStr.substr(0, i));
						if (width > renderWidth)
							break;
						lineCommitted = i;
					}
				}

				font->drawString(_renderedText.get(), lineStr.substr(0, lineCommitted), 0, line * height + (height - ascent) / 2, renderWidth, 1, textAlign, 0, false);

				if (lineCommitted == lineStr.size())
					lineStr.clear();
				else {
					lineStr = lineStr.substr(lineCommitted);
					line++;
				}
			}

			if (noMoreLines)
				break;

			line++;
			lineStart = lineEndPos + 1;
		}
	}

	Graphics::ManagedSurface *target = window->getSurface().get();
	Common::Rect srcRect(0, 0, renderWidth, renderHeight);
	Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.width(), _cachedAbsoluteOrigin.y + _rect.height());

	// TODO: Need to handle more modes
	const ColorRGB8 &color = _renderProps.getForeColor();
	const uint8 drawPalette[2 * 3] = {
		0, 0, 0,
		color.r, color.g, color.b
	};

	if (_renderedText) {
		_renderedText->setPalette(drawPalette, 0, 2);
		target->transBlitFrom(*_renderedText.get(), srcRect, destRect, 0);
	}
}

void TextLabelElement::setTextStyle(uint16 macFontID, const Common::String &fontFamilyName, uint size, TextAlignment alignment, const TextStyleFlags &styleFlags) {
	if (!_text.empty()) {
		_needsRender = true;
		_isBitmap = false;
		_contentsDirty = true;
	}

	_macFontID = macFontID;
	_fontFamilyName = fontFamilyName;
	_size = size;
	_alignment = alignment;
	_styleFlags = styleFlags;
}

Graphics::FontManager::FontUsage TextLabelElement::getDefaultUsageForMacFont(uint16 macFontID, uint size) {
	switch (macFontID) {
	case Graphics::kMacFontCourier:
		return Graphics::FontManager::kConsoleFont;
	default:
		break;
	}

	warning("Unhandled font ID %i default, this might not render well", static_cast<int>(macFontID));
	return Graphics::FontManager::kGUIFont;
}

Graphics::FontManager::FontUsage TextLabelElement::getDefaultUsageForNamedFont(const Common::String &fontFamilyName, uint size) {
	if (fontFamilyName == "Courier New") {
		if (size == 8)
			return Graphics::FontManager::kConsoleFont;
	} else if (fontFamilyName == "Arial") {
		if (size == 10)
			return Graphics::FontManager::kGUIFont;
		if (size == 14)
			return Graphics::FontManager::kBigGUIFont;
	}

	warning("Unhandled font name '%s' default, this might not render well", fontFamilyName.c_str());
	return Graphics::FontManager::kGUIFont;
}

Common::SharedPtr<Structural> TextLabelElement::shallowClone() const {
	return Common::SharedPtr<Structural>(new TextLabelElement(*this));
}

void TextLabelElement::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	VisualElement::visitInternalReferences(visitor);
}

MiniscriptInstructionOutcome TextLabelElement::scriptSetText(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kString) {
		thread->error("Tried to set a text label element's text to something that wasn't a string");
		return kMiniscriptInstructionOutcomeFailed;
	}

	_text = value.getString();
	_needsRender = true;
	_isBitmap = false;
	_contentsDirty = true;
	_macFormattingSpans.clear();

	return kMiniscriptInstructionOutcomeContinue;
}


MiniscriptInstructionOutcome TextLabelElement::scriptSetLine(MiniscriptThread *thread, size_t lineIndex, const DynamicValue &value) {
	DynamicValue derefValue = value.dereference();

	if (derefValue.getType() != DynamicValueTypes::kString) {
		thread->error("Tried to set a text label element's text to something that wasn't a string");
		return kMiniscriptInstructionOutcomeFailed;
	}

	uint32 startPos;
	uint32 endPos;
	if (findLineRange(lineIndex, startPos, endPos))
		_text = _text.substr(0, startPos) + derefValue.getString() + _text.substr(endPos, _text.size() - endPos);
	else {
		size_t numLines = countLines();
		while (numLines <= lineIndex) {
			_text += '\r';
			numLines++;
		}
		_text += derefValue.getString();
	}

	_needsRender = true;
	_isBitmap = false;
	_contentsDirty = true;
	_macFormattingSpans.clear();

	return kMiniscriptInstructionOutcomeContinue;
}

bool TextLabelElement::findLineRange(size_t lineIndex, uint32 &outStartPos, uint32 &outEndPos) const {
	uint32 lineStart = 0;
	uint32 lineEnd = _text.size();
	size_t linesToScan = lineIndex + 1;

	while (linesToScan > 0) {
		linesToScan--;

		lineEnd = _text.find('\r', lineStart);
		if (lineEnd == Common::String::npos) {
			lineEnd = _text.size();
			break;
		}
	}

	if (linesToScan > 0)
		return false;

	outStartPos = lineStart;
	outEndPos = lineEnd;

	return true;
}

size_t TextLabelElement::countLines() const {
	size_t numLines = 1;
	for (char c : _text)
		if (c == '\r')
			numLines++;

	return numLines;
}

MiniscriptInstructionOutcome TextLabelElement::TextLabelLineWriteInterface::write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset) {
	return static_cast<TextLabelElement *>(objectRef)->scriptSetLine(thread, ptrOrOffset, dest);
}

MiniscriptInstructionOutcome TextLabelElement::TextLabelLineWriteInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome TextLabelElement::TextLabelLineWriteInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return kMiniscriptInstructionOutcomeFailed;
}

SoundElement::SoundElement()
	: _leftVolume(0), _rightVolume(0), _balance(0), _assetID(0), _startTime(0), _finishTime(0), _cueCheckTime(0),
	  _startTimestamp(0), _shouldPlayIfNotPaused(true), _needsReset(true) {
}

SoundElement::SoundElement(const SoundElement &other)
	: NonVisualElement(other), _leftVolume(other._leftVolume), _rightVolume(other._rightVolume), _balance(other._balance)
	, _assetID(other._assetID), _cachedAudio(other._cachedAudio), _metadata(other._metadata), _player(nullptr)
	, _startTime(other._startTime), _finishTime(other._finishTime), _startTimestamp(other._startTimestamp)
	, _cueCheckTime(other._cueCheckTime), _shouldPlayIfNotPaused(other._shouldPlayIfNotPaused), _needsReset(true)
	, _playMediaSignaller(nullptr), _subtitlePlayer(nullptr) {

	_playMediaSignaller = other.getRuntime()->getProject()->notifyOnPlayMedia(this);

	initSubtitles();
}

SoundElement::~SoundElement() {
	if (_playMediaSignaller)
		_playMediaSignaller->removeReceiver(this);
}

bool SoundElement::load(ElementLoaderContext &context, const Data::SoundElement &data) {
	if (!NonVisualElement::loadCommon(data.name, data.guid, data.elementFlags))
		return false;

	_paused = ((data.soundFlags & Data::SoundElement::kPaused) != 0);
	_loop = ((data.soundFlags & Data::SoundElement::kLoop) != 0);
	_leftVolume = data.leftVolume;
	_rightVolume = data.rightVolume;
	_balance = data.balance;
	_assetID = data.assetID;

	return true;
}

bool SoundElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "loop") {
		result.setBool(_loop);
		return true;
	} else if (attrib == "volume") {
		result.setInt((_leftVolume + _rightVolume) / 2);
		return true;
	}

	return NonVisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome SoundElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "loop") {
		DynamicValueWriteFuncHelper<SoundElement, &SoundElement::scriptSetLoop, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "volume") {
		DynamicValueWriteFuncHelper<SoundElement, &SoundElement::scriptSetVolume, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "balance") {
		DynamicValueWriteFuncHelper<SoundElement, &SoundElement::scriptSetBalance, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "asset") {
		DynamicValueWriteFuncHelper<SoundElement, &SoundElement::scriptSetAsset, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return NonVisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

CORO_BEGIN_DEFINITION(SoundElement::SoundElementConsumeCommandCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		CORO_IF (Event(EventIDs::kPlay, 0).respondsTo(params->msg->getEvent()))
			CORO_CALL(SoundElement::StartPlayingCoroutine, params->self, params->runtime);
			CORO_RETURN
		CORO_ELSE_IF(Event(EventIDs::kStop, 0).respondsTo(params->msg->getEvent()))
			CORO_CALL(SoundElement::StopPlayingCoroutine, params->self, params->runtime);
			CORO_RETURN;
		CORO_END_IF

		CORO_CALL(NonVisualElement::NonVisualElementConsumeCommandCoroutine, params->self, params->runtime, params->msg);
	CORO_END_FUNCTION
CORO_END_DEFINITION

VThreadState SoundElement::asyncConsumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	runtime->getVThread().pushCoroutine<SoundElementConsumeCommandCoroutine>(this, runtime, msg);
	return kVThreadReturn;
}

void SoundElement::initSubtitles() {
	Project *project = getRuntime()->getProject();

	const SubtitleTables &subTables = project->getSubtitles();
	if (subTables.assetMapping) {
		const Common::String *subtitleSetIDPtr = subTables.assetMapping->findSubtitleSetForAssetID(_assetID);
		if (!subtitleSetIDPtr) {
			Common::String assetName = project->getAssetNameByID(_assetID);
			if (assetName.size() > 0)
				subtitleSetIDPtr = subTables.assetMapping->findSubtitleSetForAssetName(assetName);
		}

		if (subtitleSetIDPtr)
			_subtitlePlayer.reset(new SubtitlePlayer(getRuntime(), *subtitleSetIDPtr, subTables));
	}
}

void SoundElement::activate() {
	Project *project = getRuntime()->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Sound element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeAudio) {
		warning("Sound element assigned an asset that isn't audio");
		return;
	}

	_cachedAudio = static_cast<AudioAsset *>(asset.get())->loadAndCacheAudio(getRuntime());
	_metadata = static_cast<AudioAsset *>(asset.get())->getMetadata();

	_playMediaSignaller = project->notifyOnPlayMedia(this);

	if (_name.empty())
		_name = project->getAssetNameByID(_assetID);

	initSubtitles();
}


void SoundElement::deactivate() {
	if (_playMediaSignaller) {
		_playMediaSignaller->removeReceiver(this);
		_playMediaSignaller.reset();
	}

	_metadata.reset();
	_cachedAudio.reset();
	_player.reset();
}

bool SoundElement::canAutoPlay() const {
	return !_paused;
}

void SoundElement::playMedia(Runtime *runtime, Project *project) {
	if (_shouldPlayIfNotPaused) {
		if (_paused) {
			// Goal state is paused
			// TODO: Track pause time
			stopPlayer();
		} else {
			// Goal state is playing
			if (_needsReset) {
				// TODO: Reset to start time
				stopPlayer();
				_needsReset = false;
			}

			if (!_player) {
				_finishTime = getRuntime()->getPlayTime() + _metadata->durationMSec;

				int normalizedVolume = (_leftVolume + _rightVolume) * 255 / 200;
				int normalizedBalance = _balance * 127 / 100;

				// TODO: Support ranges
				size_t numSamples = _cachedAudio->getNumSamples(*_metadata);
				_player.reset(new AudioPlayer(getRuntime()->getAudioMixer(), normalizedVolume, normalizedBalance, _metadata, _cachedAudio, _loop, 0, 0, numSamples));

				_startTime = runtime->getPlayTime();
				_cueCheckTime = _startTime;
				_startTimestamp = 0;
			}

			uint64 newTime = getRuntime()->getPlayTime();
			if (newTime > _cueCheckTime) {
				uint64 oldTimeRelative = _cueCheckTime - _startTime + _startTimestamp;
				uint64 newTimeRelative = newTime - _startTime + _startTimestamp;

				if (_subtitlePlayer)
					_subtitlePlayer->update(oldTimeRelative, newTimeRelative);

				for (MediaCueState *mediaCue : _mediaCues)
					mediaCue->checkTimestampChange(runtime, oldTimeRelative * _metadata->sampleRate / 1000u, newTimeRelative * _metadata->sampleRate / 1000u, true, true);

				_cueCheckTime = newTime;
			}

			if (!_loop && newTime >= _finishTime) {
				// Don't throw out the handle - It can still be playing but we just treat it like it's not.
				// If it has anything left, then we let it finish and avoid clipping the sound, but we need
				// to know that the handle is still here so we can actually stop it if the element is
				// destroyed, since the stream is tied to the CachedAudio.

				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kStop, 0), DynamicValue(), getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
				runtime->queueMessage(dispatch);

				_shouldPlayIfNotPaused = false;
				if (_subtitlePlayer)
					_subtitlePlayer->stop();
			}
		}
	} else {
		// Goal state is stopped
		stopPlayer();
	}
}

void SoundElement::tryAutoSetName(Runtime *runtime, Project *project) {
	_name = project->getAssetNameByID(_assetID);
}

bool SoundElement::resolveMediaMarkerLabel(const Label &label, int32 &outResolution) const {
	if (_metadata) {
		for (const AudioMetadata::CuePoint &cuePoint : _metadata->cuePoints) {
			if (cuePoint.cuePointID == label.id) {
				outResolution = cuePoint.position;
				return true;
			}
		}
	}

	return false;
}

Common::SharedPtr<Structural> SoundElement::shallowClone() const {
	return Common::SharedPtr<Structural>(new SoundElement(*this));
}

void SoundElement::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	NonVisualElement::visitInternalReferences(visitor);
}

void SoundElement::stopPlayer() {
	_player.reset();
	if (_subtitlePlayer)
		_subtitlePlayer->stop();
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void SoundElement::debugInspect(IDebugInspectionReport *report) const {
	NonVisualElement::debugInspect(report);

	report->declareDynamic("leftVol", Common::String::format("%i", _leftVolume));
	report->declareDynamic("rightVol", Common::String::format("%i", _rightVolume));
	report->declareDynamic("balance", Common::String::format("%i", _balance));
	report->declareDynamic("asset", Common::String::format("%i", _assetID));

	AudioMetadata *metadata = _metadata.get();
	report->declareDynamic("duration", metadata ? Common::String::format("%i", metadata->durationMSec) : Common::String("Unknown"));
	report->declareDynamic("finishTime", Common::String::format("%i", static_cast<int>(_finishTime)));
	report->declareDynamic("shouldPlayIfNotPaused", _shouldPlayIfNotPaused ? "true" : "false");
	report->declareDynamic("paused", _paused ? "true" : "false");
	report->declareDynamic("needsReset", _needsReset ? "true" : "false");
}
#endif

MiniscriptInstructionOutcome SoundElement::scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	setLoop(value.getBool());
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SoundElement::scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 0)
		asInteger = 0;
	else if (asInteger > 100)
		asInteger = 100;

	setVolume(asInteger);
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SoundElement::scriptSetBalance(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < -100)
		asInteger = -100;
	else if (asInteger > 100)
		asInteger = 100;

	setBalance(asInteger);
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SoundElement::scriptSetAsset(MiniscriptThread *thread, const DynamicValue &value) {
	DynamicValue derefValue = value.dereference();

	if (derefValue.getType() != DynamicValueTypes::kString) {
		thread->error("Tried to set a sound element's asset to something that wasn't a string");
		return kMiniscriptInstructionOutcomeFailed;
	}

	stopPlayer();

	Project *project = thread->getRuntime()->getProject();

	uint32 assetID = 0;
	if (!project->getAssetIDByName(derefValue.getString(), assetID)) {
		warning("Sound element references asset '%s' but the asset ID couldn't be resolved", derefValue.getString().c_str());
		return kMiniscriptInstructionOutcomeFailed;
	}

	Common::Array<Common::SharedPtr<Asset> > forceLoadedAssets;

	Common::SharedPtr<Asset> asset = project->getAssetByID(assetID).lock();

	if (!asset) {
		if (thread->getRuntime()->getHacks().allowAssetsFromOtherScenes) {
			project->forceLoadAsset(assetID, forceLoadedAssets);
			asset = project->getAssetByID(assetID).lock();
		}
	}

	if (!asset) {
		warning("Sound element references asset '%s' but the asset isn't loaded!", derefValue.getString().c_str());
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (asset->getAssetType() != kAssetTypeAudio) {
		warning("Sound element assigned an asset that isn't audio");
		return kMiniscriptInstructionOutcomeFailed;
	}

	_cachedAudio = static_cast<AudioAsset *>(asset.get())->loadAndCacheAudio(getRuntime());
	_metadata = static_cast<AudioAsset *>(asset.get())->getMetadata();
	_assetID = asset->getAssetID();

	return kMiniscriptInstructionOutcomeContinue;

}

void SoundElement::setLoop(bool loop) {
	_loop = loop;
}

void SoundElement::setVolume(uint16 volume) {
	uint16 fullVolumeLeft = 100 - _balance;

	// Weird math to ensure _leftVolume + _rightVolume stays divisible by 2
	_leftVolume = (volume * fullVolumeLeft + 50) / 100;
	_rightVolume = volume * 2 - _leftVolume;
}

void SoundElement::setBalance(int16 balance) {
	_balance = balance;
	setVolume((_leftVolume + _rightVolume) / 2);
}

CORO_BEGIN_DEFINITION(SoundElement::StartPlayingCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		// Unpaused -> Played

		params->self->_shouldPlayIfNotPaused = true;
		params->self->_needsReset = true;

		CORO_IF (params->self->_paused)
			params->self->_paused = false;
		
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kUnpause, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);
		CORO_END_IF

		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kPlay, 0), DynamicValue(), params->self->getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

		CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);
	CORO_END_FUNCTION
CORO_END_DEFINITION

CORO_BEGIN_DEFINITION(SoundElement::StopPlayingCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		CORO_IF (params->self->_shouldPlayIfNotPaused)
			params->self->_shouldPlayIfNotPaused = false;
			params->self->_needsReset = true;

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kStop, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);
		CORO_END_IF
	CORO_END_FUNCTION
CORO_END_DEFINITION

} // End of namespace MTropolis
