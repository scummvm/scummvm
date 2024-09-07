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

#include "common/system.h"
#include "common/hashmap.h"

#include "graphics/managed_surface.h"
#include "graphics/surface.h"

#include "mtropolis/assets.h"
#include "mtropolis/detection.h"
#include "mtropolis/elements.h"
#include "mtropolis/hacks.h"
#include "mtropolis/runtime.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/saveload.h"

namespace MTropolis {

Hacks::Hacks() {
	ignoreMismatchedProjectNameInObjectLookups = false;
	removeQuickTimeEdits = false;
	midiVolumeScale = 256;
	minTransitionDuration = 0;
	ignoreMToonMaintainRateFlag = false;
	allowAssetsFromOtherScenes = false;
	mtiVariableReferencesHack = false;
	mtiSceneReturnHack = false;
	mtiHispaniolaDamagedStringHack = false;
	ignoreSceneUnloads = false;
}

Hacks::~Hacks() {
}

void Hacks::addStructuralHooks(uint32 guid, const Common::SharedPtr<StructuralHooks> &hooks) {
	structuralHooks[guid] = hooks;
}

void Hacks::addModifierHooks(uint32 guid, const Common::SharedPtr<ModifierHooks> &hooks) {
	modifierHooks[guid] = hooks;
}

void Hacks::addAssetHooks(const Common::SharedPtr<AssetHooks> &hooks) {
	assetHooks.push_back(hooks);
}

void Hacks::addSceneTransitionHooks(const Common::SharedPtr<SceneTransitionHooks> &hooks) {
	sceneTransitionHooks.push_back(hooks);
}

void Hacks::addSaveLoadHooks(const Common::SharedPtr<SaveLoadHooks> &hooks) {
	saveLoadHooks.push_back(hooks);
}

void Hacks::addSaveLoadMechanismHooks(const Common::SharedPtr<SaveLoadMechanismHooks> &hooks) {
	saveLoadMechanismHooks.push_back(hooks);
}

namespace HackSuites {

class ObsidianCorruptedAirTowerTransitionFix : public AssetHooks {
public:
	void onLoaded(Asset *asset, const Common::String &name) override;
};

void ObsidianCorruptedAirTowerTransitionFix::onLoaded(Asset *asset, const Common::String &name) {
	if (asset->getAssetType() == kAssetTypeMovie && name == "A105_132.01Fxx.trn") {
		static_cast<MovieAsset *>(asset)->addDamagedFrame(35);
	}
}

class ObsidianInventoryWindscreenHooks : public StructuralHooks {
public:
	void onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) override;
};

void ObsidianInventoryWindscreenHooks::onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) {
	if (pt.y < 480) {
		// Set direct to screen so it draws over cinematics
		static_cast<VisualElement *>(structural)->setDirectToScreen(true);

		// Move in-bounds
		pt.y -= 60;
	}
}

class ObsidianSecurityFormWidescreenHooks : public StructuralHooks {
public:
	void onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) override;

private:
	Common::Array<uint32> _hiddenCards;
};

void ObsidianSecurityFormWidescreenHooks::onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) {
	bool cardVisibility = (pt.y > 480);

	// Originally tried manipulating layer order but that's actually not a good solution because
	// the form graphic is not actually dismissed until the cinematic completes.  It's normally not
	// visible because the cinematic is drawn over it, but managing that vis-a-vis the cards is a mess,
	// and the form graphic actually includes a bit of area to the left due to the vidbot's arm being
	// on the desk, which partially overlaps the cards, but not completely.
	Structural *subsection = structural->getParent()->getParent();
	assert(subsection->isSubsection());

	Structural *sharedScene = subsection->getChildren()[0].get();
	assert(sharedScene);

	Structural *cards = nullptr;
	for (const Common::SharedPtr<Structural> &child : sharedScene->getChildren()) {
		if (child->getName() == "Inventory Cards") {
			cards = child.get();
			break;
		}
	}

	if (!cardVisibility)
		_hiddenCards.clear();

	if (cards) {
		for (const Common::SharedPtr<Structural> &child : cards->getChildren()) {
			assert(child->isElement() && static_cast<Element *>(child.get())->isVisual());

			VisualElement *card = static_cast<VisualElement *>(child.get());

			if (cardVisibility) {
				if (Common::find(_hiddenCards.begin(), _hiddenCards.end(), card->getStaticGUID()) != _hiddenCards.end())
					card->setVisible(runtime, true);
			} else {
				if (card->isVisible()) {
					_hiddenCards.push_back(card->getStaticGUID());
					card->setVisible(runtime, false);
				}
			}
		}
	}

	if (cardVisibility)
		_hiddenCards.clear();
}

class ObsidianRSGLogoAnamorphicFilter : public MovieResizeFilter {
public:
	ObsidianRSGLogoAnamorphicFilter();

	Common::SharedPtr<Graphics::ManagedSurface> scaleFrame(const Graphics::Surface &surface, uint32 timestamp) const override;

private:
	template<class TPixel>
	void anamorphicScaleFrameTyped(const Graphics::Surface &src, Graphics::ManagedSurface &dest) const;

	static double anamorphicCurve(double d);
	static double inverseAnamorphicCurve(double d);

	template<class TPixel>
	void halveWidthTyped(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dest) const;

	template<class TPixel>
	void halveHeightTyped(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dest) const;

	Common::Array<uint> _xCoordinates;
	Common::Array<uint> _yCoordinates;
};

ObsidianRSGLogoAnamorphicFilter::ObsidianRSGLogoAnamorphicFilter() {
	// Anamorphic rescale, keeps the RSG logo proportional but preserves the vertical spacing!
	// We use an anamorphic curve of y=x+x^2 which ensures the derivative at 0 is 1, meaning
	// the rate of change stays constant at with the unfiltered pixels at the edge of the filter.
	const uint unscaledWidth = 640;
	const uint unscaledHeight = 480;

	const uint scaledWidth = 1280;
	const uint scaledHeight = 720;

	_xCoordinates.resize(scaledWidth);
	_yCoordinates.resize(scaledHeight);

	// Margin in pixels on the side of the original image to apply filter
	const double scalingFactor = static_cast<double>(scaledHeight) / static_cast<double>(unscaledHeight);
	const double invScalingFactor = 1.0 / scalingFactor;
	const double sideMarginInOriginalImage = 90.0;

	const double sideMarginInScaledImage = (static_cast<double>(scaledWidth) - ((static_cast<double>(unscaledWidth) - sideMarginInOriginalImage * 2.0) * scalingFactor)) * 0.5;

	const double originalMarginHeightFraction = sideMarginInOriginalImage / static_cast<double>(unscaledHeight);
	const double scaledMarginHeightFraction = sideMarginInScaledImage / static_cast<double>(scaledHeight);

	const double targetCurveRatio = scaledMarginHeightFraction / originalMarginHeightFraction;

	// (x + x^2) / x = targetCurveRatio
	// (x + x^2) = targetCurveRatio * x
	// 1 + x = targetCurveRatio

	const double xCurveRatio = targetCurveRatio - 1.0;
	const double yCurveRatio = anamorphicCurve(xCurveRatio);

	const double rightMarginStart = static_cast<double>(scaledWidth) - sideMarginInScaledImage;

	for (uint i = 0; i < scaledWidth; i++) {
		double pixelCenterX = static_cast<double>(i) + 0.5;
		double originalImagePixelCenter = 0.0;
		if (pixelCenterX < sideMarginInScaledImage) {
			double marginFraction = 1.0 - pixelCenterX / sideMarginInScaledImage;
			double marginCurveY = marginFraction * yCurveRatio;
			double marginCurveX = inverseAnamorphicCurve(marginCurveY);
			double multiplier = 1.0 - marginCurveX / xCurveRatio;
			originalImagePixelCenter = multiplier * sideMarginInOriginalImage;
		} else if (pixelCenterX > rightMarginStart) {
			double marginFraction = 1.0 - (static_cast<double>(scaledWidth) - pixelCenterX) / sideMarginInScaledImage;
			double marginCurveY = marginFraction * yCurveRatio;
			double marginCurveX = inverseAnamorphicCurve(marginCurveY);
			double multiplier = 1.0 - marginCurveX / xCurveRatio;
			originalImagePixelCenter = static_cast<double>(unscaledWidth) - multiplier * sideMarginInOriginalImage;
		} else {
			double offsetFromCenter = pixelCenterX - (static_cast<double>(scaledWidth) * 0.5);
			double offsetFromCenterInOriginalImage = offsetFromCenter * invScalingFactor;
			originalImagePixelCenter = static_cast<double>(unscaledWidth) * 0.5 + offsetFromCenterInOriginalImage;
		}

		double srcPixelX = floor(originalImagePixelCenter);
		if (srcPixelX < 0.0)
			srcPixelX = 0.0;
		else if (srcPixelX >= static_cast<double>(unscaledWidth))
			srcPixelX = static_cast<double>(unscaledWidth - 1);

		_xCoordinates[i] = static_cast<uint>(srcPixelX);
	}

	for (uint i = 0; i < scaledHeight; i++)
		_yCoordinates[i] = (2 * i + 1) * unscaledHeight / (scaledHeight * 2);
}

template<class TPixel>
void ObsidianRSGLogoAnamorphicFilter::anamorphicScaleFrameTyped(const Graphics::Surface &src, Graphics::ManagedSurface &dest) const {
	const uint width = _xCoordinates.size();
	const uint height = _yCoordinates.size();

	const uint *xCoordinates = &_xCoordinates[0];
	const uint *yCoordinates = &_yCoordinates[0];

	assert(width == static_cast<uint>(dest.w));
	assert(height == static_cast<uint>(dest.h));

	for (uint row = 0; row < height; row++) {
		const TPixel *srcRow = static_cast<const TPixel *>(src.getBasePtr(0, yCoordinates[row]));
		TPixel *destRow = static_cast<TPixel *>(dest.getBasePtr(0, row));

		for (uint col = 0; col < width; col++)
			destRow[col] = srcRow[xCoordinates[col]];
	}
}

double ObsidianRSGLogoAnamorphicFilter::anamorphicCurve(double d) {
	return d + d * d;
}

double ObsidianRSGLogoAnamorphicFilter::inverseAnamorphicCurve(double d) {
	return -0.5 + sqrt(0.25 + d);
}

template<class TPixel>
void ObsidianRSGLogoAnamorphicFilter::halveWidthTyped(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dest) const {
	const uint widthHigh = src.w;
	const uint widthLow = dest.w;
	const uint height = src.h;

	assert(widthLow * 2 == widthHigh);
	assert(dest.h == src.h);

	const Graphics::PixelFormat fmt = src.format;

	for (uint row = 0; row < height; row++) {
		const TPixel *srcRow = static_cast<const TPixel *>(src.getBasePtr(0, row));
		TPixel *destRow = static_cast<TPixel *>(dest.getBasePtr(0, row));

		for (uint col = 0; col < widthLow; col++) {
			uint32 col1 = srcRow[col * 2];
			uint32 col2 = srcRow[col * 2 + 1];

			uint8 r1, g1, b1;
			fmt.colorToRGB(col1, r1, g1, b1);

			uint8 r2, g2, b2;
			fmt.colorToRGB(col2, r2, g2, b2);

			destRow[col] = fmt.RGBToColor((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1);
		}
	}
}

template<class TPixel>
void ObsidianRSGLogoAnamorphicFilter::halveHeightTyped(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dest) const {
	const uint heightHigh = src.h;
	const uint heightLow = dest.h;
	const uint width = src.w;

	assert(heightLow * 2 == heightHigh);
	assert(dest.w == src.w);

	const Graphics::PixelFormat fmt = src.format;

	for (uint row = 0; row < heightLow; row++) {
		const TPixel *srcRow1 = static_cast<const TPixel *>(src.getBasePtr(0, row * 2));
		const TPixel *srcRow2 = static_cast<const TPixel *>(src.getBasePtr(0, row * 2 + 1));
		TPixel *destRow = static_cast<TPixel *>(dest.getBasePtr(0, row));

		for (uint col = 0; col < width; col++) {
			uint32 col1 = srcRow1[col];
			uint32 col2 = srcRow2[col];

			uint8 r1, g1, b1;
			fmt.colorToRGB(col1, r1, g1, b1);

			uint8 r2, g2, b2;
			fmt.colorToRGB(col2, r2, g2, b2);

			destRow[col] = fmt.RGBToColor((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1);
		}
	}
}

Common::SharedPtr<Graphics::ManagedSurface> ObsidianRSGLogoAnamorphicFilter::scaleFrame(const Graphics::Surface &surface, uint32 timestamp) const {
	Common::SharedPtr<Graphics::ManagedSurface> result(new Graphics::ManagedSurface());
	result->create(_xCoordinates.size() / 2, _yCoordinates.size() / 2, surface.format);

	Common::SharedPtr<Graphics::ManagedSurface> temp1(new Graphics::ManagedSurface());
	Common::SharedPtr<Graphics::ManagedSurface> temp2(new Graphics::ManagedSurface());

	temp1->create(_xCoordinates.size(), _yCoordinates.size(), surface.format);
	temp2->create(_xCoordinates.size() / 2, _yCoordinates.size(), surface.format);

	if (surface.format.bytesPerPixel == 1) {
		anamorphicScaleFrameTyped<uint8>(surface, *temp1);
		halveWidthTyped<uint8>(*temp1, *temp2);
		halveHeightTyped<uint8>(*temp2, *result);
	} else if (surface.format.bytesPerPixel == 2) {
		anamorphicScaleFrameTyped<uint16>(surface, *temp1);
		halveWidthTyped<uint16>(*temp1, *temp2);
		halveHeightTyped<uint16>(*temp2, *result);
	} else if (surface.format.bytesPerPixel == 4) {
		anamorphicScaleFrameTyped<uint32>(surface, *temp1);
		halveWidthTyped<uint32>(*temp1, *temp2);
		halveHeightTyped<uint32>(*temp2, *result);
	}

	return result;
}

class ObsidianRSGLogoWidescreenHooks : public StructuralHooks {
public:
	void onCreate(Structural *structural) override;
};

void ObsidianRSGLogoWidescreenHooks::onCreate(Structural *structural) {
	MovieElement *movie = static_cast<MovieElement *>(structural);
	movie->setRelativeRect(Common::Rect(0, 60, 640, 420));
	movie->setResizeFilter(Common::SharedPtr<MovieResizeFilter>(new ObsidianRSGLogoAnamorphicFilter()));
}

class ObsidianSaveScreenshotHooks : public SceneTransitionHooks {
public:
	void onSceneTransitionSetup(Runtime *runtime, const Common::WeakPtr<Structural> &oldScene, const Common::WeakPtr<Structural> &newScene) override;
};

void ObsidianSaveScreenshotHooks::onSceneTransitionSetup(Runtime *runtime, const Common::WeakPtr<Structural> &oldScene, const Common::WeakPtr<Structural> &newScene) {
	Structural *newScenePtr = newScene.lock().get();

	if (!newScenePtr)
		return;

	if (newScenePtr->getName() == "Game_Screen") {
		Window *mainWindow = runtime->getMainWindow().lock().get();
		if (mainWindow) {
			Common::SharedPtr<Graphics::ManagedSurface> mainWindowSurface = mainWindow->getSurface();
			Common::SharedPtr<Graphics::ManagedSurface> screenshot(new Graphics::ManagedSurface());
			screenshot->copyFrom(*mainWindowSurface);

			runtime->setSaveScreenshotOverride(screenshot);
		}
	} else {
		runtime->setSaveScreenshotOverride(Common::SharedPtr<Graphics::ManagedSurface>());
	}
}

void addObsidianQuirks(const MTropolisGameDescription &desc, Hacks &hacks) {
	// Add screenshot hook to store savegame screenshot prior to going to the menu
	hacks.addSceneTransitionHooks(Common::SharedPtr<SceneTransitionHooks>(new ObsidianSaveScreenshotHooks()));

	// Strip edit lists from QuickTime movies to work around audio popping problem.
	// For some reason, some vidbots (like the cube maze entry bot) have edit lists
	// in the audio track full of half-second edits, but the edit offsets are spaced
	// 22080 samples apart instead of 22050, which causes it to skip 30 audio samples
	// every half-second.
	hacks.removeQuickTimeEdits = true;
}

void addObsidianBugFixes(const MTropolisGameDescription &desc, Hacks &hacks) {
	// Workaround for bug in Obsidian:
	// When opening the journal in the intro, a script checks if cGSt.cfst.binjournal is false and if so,
	// sets cGSt.cfst.binjournal to true and then sets including setting cJournalConst.aksjournpath to the
	// main journal scene path.  That scene path is used to resolve the scene to go to after clicking
	// the "Continue" button on the warning that pops up.
	//
	// The problem is that cJournalConst uses a project name that doesn't match the retail data, and
	// cJournalConst is unloaded if the player leaves the journal.  This causes a progression blocker if
	// the player leaves the journal without clicking Continue.
	hacks.ignoreMismatchedProjectNameInObjectLookups = true;

	// Fix for corrupted frame in transition from the outer edge in Spider to the air puzzle tower.
	// The data is corrupted in both Mac and Win retail versions.
	hacks.addAssetHooks(Common::SharedPtr<AssetHooks>(new ObsidianCorruptedAirTowerTransitionFix()));
}

void addObsidianImprovedWidescreen(const MTropolisGameDescription &desc, Hacks &hacks) {
	if ((desc.desc.flags & ADGF_DEMO) == 0) {
		const uint32 inventoryItemGUIDs[] = {
			0x4dfa22, // Obsidian Data > Labyrinth1 > 100 > Untitled Shared Scene > L200.0L.63Fxx.mtn (Limitation Document, Atlas Face)
			0x4e2d9e, // Obsidian Data > Labyrinth1 > 100 > Untitled Shared Scene > L200.0L.64Fxx.mtn (Tradesman Document, Atlas Face)
			0x4e2555, // Obsidian Data > Labyrinth1 > 199 > > L200.0L.63Fxx.mtn (Limitation Document, Glyph Chamber)
			0x4de654, // Obsidian Data > Labyrinth1 > 199 > > L200.0L.64Fxx.mtn (Tradesman Document, Glyph Chamber)
			0x4c83d2, // Obsidian Data > Labyrinth1 > 200 > Untitled Shared Scene > L200.0L.63Fxx.mtn (Limitation Document, Records Face)
			0x4c5802, // Obsidian Data > Labyrinth1 > 200 > Untitled Shared Scene > L200.0L.64Fxx.mtn (Tradesman Document, Records Face)
			0x178d5c, // Obsidian Data > Labyrinth1 > 299 > Untitled Shared Scene > L200.0L.63Fxx.mtn (Limitation Document, Cloud Ring)
			0x177754, // Obsidian Data > Labyrinth1 > 299 > Untitled Shared Scene > L200.0L.64Fxx.mtn (Tradesman Document, Cloud Ring)
			0x9914fb, // Obsidian Data > Labyrinth1 > 800 > > L200.0L.63Fxx.mtn (Limitation Document, Sunken Library)
			0x990f1f, // Obsidian Data > Labyrinth1 > 800 > > L200.0L.64Fxx.mtn (Tradesman Document, Sunken Library)
			0x4e0f86, // Obsidian Data > Labyrinth2 > 300 > Untitled Shared Scene > L200.0L.63Fxx.mtn (Limitation Document, Nexus Face)
			0x4e5107, // Obsidian Data > Labyrinth2 > 300 > Untitled Shared Scene > L200.0L.64Fxx.mtn (Tradesman Document, Nexus Face)
			0x4e5528, // Obsidian Data > Labyrinth2 > 400 > Untitled Shared Scene > L200.0L.63Fxx.mtn (Limitation Document, Security Face)
			0x4e55cc, // Obsidian Data > Labyrinth2 > 400 > Untitled Shared Scene > L200.0L.64Fxx.mtn (Tradesman Document, Security Face)
			0x6035f, // Obsidian Data > Labyrinth2 > 400 > Untitled Shared Scene > Inventory Cards > L400.0L.04Fxx.mtn - Black (Cards, Security Face)
			0x62e24, // Obsidian Data > Labyrinth2 > 400 > Untitled Shared Scene > Inventory Cards > L400.0L.04Fxx.mtn - Blue (Cards, Security Face)
			0x58d7f, // Obsidian Data > Labyrinth2 > 400 > Untitled Shared Scene > Inventory Cards > L400.0L.04Fxx.mtn - Yellow (Cards, Security Face)
			0x58212, // Obsidian Data > Labyrinth2 > 400 > Untitled Shared Scene > Inventory Cards > L400.0L.04Fxx.mtn - Red (Cards, Security Face)
			0x4e2e7b, // Obsidian Data > Labyrinth2 > 500 > Untitled Shared Scene > L200.0L.63Fxx.mtn (Limitation Document, Time Face)
			0x4e0710, // Obsidian Data > Labyrinth2 > 500 > Untitled Shared Scene > L200.0L.64Fxx.mtn (Tradesman Document, Time Face)
			0x12fa7,  // Obsidian Data > Abraxas1 > 400 > Untitled Shared Scene > Inventory Beaker (Inventory Beaker, Metal Balcony)
			0x5f02e6, // Obsidian Data > Bismuth1 > 100 > Untitled Shared Scene > B201.2L.02Fxx.spr (Inspiration Chip, Junkyard)
			0x9bd5fc, // Obsidian Data > Bismuth1 > 200 > Untitled Shared Scene > B201.2L.02Fxx.spr (Inspiration Chip, Plane)
			0x5ef979, // Obsidian Data > Bismuth1 > 300 > Untitled Shared Scene > B201.2L.02Fxx.spr (Inspiration Chip, Piazza)
			0x5ecdee, // Obsidian Data > Bismuth1 > 500 > Untitled Shared Scene > B201.2L.02Fxx.spr (Inspiration Chip, Statue)
			0xed9a8f, // Obsidian Data > Bismuth2 > 400 > Untitled Shared Scene > B201.2L.02Fxx.spr (Inspiration Chip, Church of the Machine)
		};

		const uint32 cubeMazeSecurityFormGUID = 0x9602ec; // Obsidian Data > Labyrinth2 > 400 > L437.2L.02Fxx.spr (Security Form, Securty Booth)
		const uint32 rsgIntroMovieGUID = 0x2fc101; // Obsidian Data > Start Obsidian > SplashScreen > RSG Splash > rssplash.F11.cut (Rocket Science Games Logo)

		Common::SharedPtr<StructuralHooks> invItemHooks(new ObsidianInventoryWindscreenHooks());

		for (uint32 guid : inventoryItemGUIDs)
			hacks.addStructuralHooks(guid, invItemHooks);

		hacks.addStructuralHooks(cubeMazeSecurityFormGUID, Common::SharedPtr<StructuralHooks>(new ObsidianSecurityFormWidescreenHooks()));
		hacks.addStructuralHooks(rsgIntroMovieGUID, Common::SharedPtr<StructuralHooks>(new ObsidianRSGLogoWidescreenHooks()));
	}
}

// Auto-save triggers for Obsidian.  Basically, we auto-save on reaching specific scenes when conditions are met.
// There are two types of condition: One is the player reaches the scene from a one-way scene that can not be
// revisited, such as a chapter transition.
//
// The other is a variable latch, which happens if the variable became true since the last time the reset scene
// (the opening credits) was reached, or since the last time the game was loaded.
//
// Variable latches don't work if the latch becomes true and the player saves+reloads their game before they hit
// the auto-save checkpoint, but that's okay.

struct ObsidianAutoSaveTrigger {
	const char *sceneName;
	const char *priorSceneName; // If set, only save when transitioning from this scene
	const char *varTrueLatch;   // If set, only save when this variable was set to
};

static ObsidianAutoSaveTrigger kObsidianAutoSaveTriggers[] = {
	// Arrive at campsite
	{
		"103.2L",
		"102_103_Credits",
		nullptr,
	},
	// Bureau start
	{
		"102.0L",
		nullptr,
		"cgst.clst.cl100st.bbossspoken"
	},
	// Win cube maze from side room
	{
		"445.2L",
		nullptr,
		"cgst.clst.cl400st.bcubiclewon",
	},
	// Win cube maze from back room
	{
		"445.0L",
		nullptr,
		"cgst.clst.cl400st.bcubiclewon",
	},
	// Stamp document and back away
	{
		"445.2L",
		nullptr,
		"cgst.clst.cinv.bstampedsd",
	},
	// Get repair document from cabinet + back away (English version)
	{
		"218.4L",
		nullptr,
		"cgst.clst.cinv.bhavesd",
	},
	// Get repair document from Bridge Repair + back away (non-English versions)
	{
		"109.6L",
		nullptr,
		"cgst.clst.cinv.bhavesd",
	},
	// Fix Immediate Action clock + exit left side
	{
		"504.0L",
		nullptr,
		"cgst.clst.cl500st.bwonl505",
	},
	// Fix Immediate Action clock + exit right side
	{
		"504.4L",
		nullptr,
		"cgst.clst.cl500st.bwonl505",
	},
	// Give document to Immediate Action + return to light
	{
		"306.6L",
		nullptr,
		"cgst.clst.cinv.bgavesd",
	},
	// Get rebel document + leave cabinet (English version)
	{
		"227.4L",
		nullptr,
		"cgst.clst.cinv.bhaveom",
	},
	// Solve dial puzzle
	{
		"699.0L",
		nullptr,
		"cgst.clst.cl308st.bwon",
	},

	// Spider start
	{
		"101.2L",
		"710.0L",
		nullptr,
	},
	// Leave elevator after completing any Spider puzzle
	{
		"121.0L",
		"118.0L_121.0L-s1",
		nullptr,
	},

	// Inspiration start
	{
		"101.4L",
		"121.0L_Lunch_Time",
		nullptr,
	},
	// Complete propulsion puzzle and leave engine room
	{
		"201.4L",
		nullptr,
		"cgst.cbst.cb2st.bengineon",
	},
	// Complete Church puzzle and leave robot
	{
		"412.4L",
		nullptr,
		"cgst.cbst.cb4st.bwon",
	},
	// Complete statue canvas puzzle
	{
		"523.2L_from_521.2L-s2",
		nullptr,
		"cgst.cbst.cb5st.bpaintedblank",
	},

	// Conductor start
	{
		"101.6L",
		"203.4L_WIN",
		nullptr,
	},
	// Freed Max
	{
		"104.0L_Max_Freed",
		nullptr,
		"cgst.ccst.cc1st.bmaxfreed",
	},
};

class ObsidianAutoSaveVarsState {
public:
	ObsidianAutoSaveVarsState();

	static const VariableModifier *findVar(Runtime *runtime, const Common::String &str);

	bool getVarState(const Common::String &varName) const;
	void resyncAllVars(Runtime *runtime);

private:
	Common::HashMap<Common::String, bool> _varState;
};

ObsidianAutoSaveVarsState::ObsidianAutoSaveVarsState() {
	for (const ObsidianAutoSaveTrigger &trigger : kObsidianAutoSaveTriggers) {
		if (trigger.varTrueLatch)
			_varState[trigger.varTrueLatch] = false;
	}
}

const VariableModifier *ObsidianAutoSaveVarsState::findVar(Runtime *runtime, const Common::String &str) {
	size_t scanStartPos = 0;

	const Modifier *modifierScan = nullptr;
	const IModifierContainer *container = runtime->getProject();

	for (;;) {
		size_t dotPos = str.findFirstOf('.', scanStartPos);
		if (dotPos == Common::String::npos)
			dotPos = str.size();

		Common::String childName = str.substr(scanStartPos, dotPos - scanStartPos);
		if (!container)
			return nullptr;

		modifierScan = nullptr;
		for (const Common::SharedPtr<Modifier> &modifier : container->getModifiers()) {
			if (caseInsensitiveEqual(childName, modifier->getName())) {
				modifierScan = modifier.get();
				break;
			}
		}

		if (!modifierScan)
			return nullptr;

		if (modifierScan->isCompoundVariable())
			container = static_cast<const CompoundVariableModifier *>(modifierScan);

		if (dotPos == str.size())
			break;

		scanStartPos = dotPos + 1;
	}

	if (modifierScan && modifierScan->isVariable())
		return static_cast<const VariableModifier *>(modifierScan);

	return nullptr;
}

bool ObsidianAutoSaveVarsState::getVarState(const Common::String &varName) const {
	Common::HashMap<Common::String, bool>::const_iterator it = _varState.find(varName);
	if (it == _varState.end())
		return false;
	return it->_value;
}

void ObsidianAutoSaveVarsState::resyncAllVars(Runtime *runtime) {
	for (Common::HashMap<Common::String, bool>::iterator it = _varState.begin(), itEnd = _varState.end(); it != itEnd; ++it) {
		const VariableModifier *var = findVar(runtime, it->_key);
		if (var) {
			DynamicValue varValue;
			var->varGetValue(varValue);
			assert(varValue.getType() == DynamicValueTypes::kBoolean);

			it->_value = varValue.getBool();
		}
	}
}

class ObsidianAutoSaveSceneTransitionHooks : public SceneTransitionHooks {
public:
	explicit ObsidianAutoSaveSceneTransitionHooks(const Common::SharedPtr<ObsidianAutoSaveVarsState> &vars, IAutoSaveProvider *autoSaveProvider);

	void onSceneTransitionEnded(Runtime *runtime, const Common::WeakPtr<Structural> &newScene) override;

private:
	Common::SharedPtr<ObsidianAutoSaveVarsState> _varsState;
	IAutoSaveProvider *_autoSaveProvider;

	Common::String _currentSceneName;
	Common::String _prevSceneName;

	Common::String _resetSceneName;
	Common::String _saveVarName;
};

ObsidianAutoSaveSceneTransitionHooks::ObsidianAutoSaveSceneTransitionHooks(const Common::SharedPtr<ObsidianAutoSaveVarsState> &vars, IAutoSaveProvider *autoSaveProvider)
	: _varsState(vars), _autoSaveProvider(autoSaveProvider) {

	_resetSceneName = Common::String("101_102_Credits");
	_saveVarName = Common::String("cgst");
}

void ObsidianAutoSaveSceneTransitionHooks::onSceneTransitionEnded(Runtime *runtime, const Common::WeakPtr<Structural> &newScene) {
	bool triggerAutoSave = false;

	if (newScene.expired())
		return;

	_prevSceneName = _currentSceneName;
	_currentSceneName = newScene.lock()->getName();

	for (const ObsidianAutoSaveTrigger &trigger : kObsidianAutoSaveTriggers) {
		Common::String triggerSceneName(trigger.sceneName);

		if (!caseInsensitiveEqual(triggerSceneName, _currentSceneName))
			continue;

		if (trigger.priorSceneName && !caseInsensitiveEqual(trigger.priorSceneName, _prevSceneName))
			continue;

		if (trigger.varTrueLatch) {
			Common::String varName(trigger.varTrueLatch);

			// Variable must must have been false since the last game load or reset
			if (_varsState->getVarState(varName))
				continue;

			bool passedLatchTest = false;

			const VariableModifier *var = _varsState->findVar(runtime, varName);
			if (var) {
				DynamicValue varValue;
				var->varGetValue(varValue);
				assert(varValue.getType() == DynamicValueTypes::kBoolean);

				passedLatchTest = varValue.getBool();
			}

			if (!passedLatchTest)
				continue;
		}

		triggerAutoSave = true;
		break;
	}

	if (triggerAutoSave) {
		Common::SharedPtr<Modifier> saveVar;

		for (const Common::SharedPtr<Modifier> &child : runtime->getProject()->getModifiers()) {
			if (caseInsensitiveEqual(child->getName(), _saveVarName)) {
				saveVar = child;
				break;
			}
		}

		if (saveVar && saveVar->isModifier()) {
			Modifier *modifier = static_cast<Modifier *>(saveVar.get());
			Common::SharedPtr<ModifierSaveLoad> saveLoad = modifier->getSaveLoad(runtime);

			if (saveLoad) {
				CompoundVarSaver saver(runtime, saveVar.get());
				_autoSaveProvider->autoSave(&saver);

				_varsState->resyncAllVars(runtime);
			}
		}
	}

	if (caseInsensitiveEqual(_currentSceneName, _resetSceneName))
		_varsState->resyncAllVars(runtime);
}

class ObsidianAutoSaveSaveLoadHooks : public SaveLoadHooks {
public:
	explicit ObsidianAutoSaveSaveLoadHooks(const Common::SharedPtr<ObsidianAutoSaveVarsState> &vars);

	void onSave(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) override;
	void onLoad(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) override;

private:
	Common::SharedPtr<ObsidianAutoSaveVarsState> _varsState;
};


ObsidianAutoSaveSaveLoadHooks::ObsidianAutoSaveSaveLoadHooks(const Common::SharedPtr<ObsidianAutoSaveVarsState> &vars) : _varsState(vars) {
}

void ObsidianAutoSaveSaveLoadHooks::onSave(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) {
	// Reset all variable latches on save
	_varsState->resyncAllVars(runtime);
}

void ObsidianAutoSaveSaveLoadHooks::onLoad(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) {
	// Reset all variable latches on load
	_varsState->resyncAllVars(runtime);
}

void addObsidianAutoSaves(const MTropolisGameDescription &desc, Hacks &hacks, IAutoSaveProvider *autoSaveProvider) {
	Common::SharedPtr<ObsidianAutoSaveVarsState> varsState(new ObsidianAutoSaveVarsState());
	hacks.addSceneTransitionHooks(Common::SharedPtr<SceneTransitionHooks>(new ObsidianAutoSaveSceneTransitionHooks(varsState, autoSaveProvider)));
	hacks.addSaveLoadHooks(Common::SharedPtr<SaveLoadHooks>(new ObsidianAutoSaveSaveLoadHooks(varsState)));
}

class ObsidianSaveLoadMechanism : public SaveLoadMechanismHooks {
public:
	bool canSaveNow(Runtime *runtime) override;
	Common::SharedPtr<ISaveWriter> createSaveWriter(Runtime *runtime) override;
};

bool ObsidianSaveLoadMechanism::canSaveNow(Runtime *runtime) {
	Project *project = runtime->getProject();

	// Check that we're in a game section
	Structural *mainScene = runtime->getActiveMainScene().get();

	if (!mainScene)
		return false;

	const Common::String disallowedSections[] = {
		Common::String("Start Obsidian"),	// Intro videos/screens
		Common::String("End Obsidian"),		// Credits
		Common::String("GUI"),				// Menus
	};

	Common::String sectionName = mainScene->getParent()->getParent()->getName();

	for (const Common::String &disallowedSection : disallowedSections) {
		if (caseInsensitiveEqual(disallowedSection, sectionName))
			return false;
	}

	// Check that the g.bESC flag is set, meaning we can go to the menu
	Common::String gName("g");
	Common::String bEscName("bESC");

	Modifier *gCompoundVar = nullptr;
	for (const Common::SharedPtr<Modifier> &child : project->getModifiers()) {
		if (caseInsensitiveEqual(child->getName(), gName)) {
			gCompoundVar = child.get();
			break;
		}
	}

	if (!gCompoundVar)
		return false;

	IModifierContainer *container = gCompoundVar->getChildContainer();
	if (!container)
		return false;

	Modifier *bEscVar = nullptr;
	for (const Common::SharedPtr<Modifier> &child : container->getModifiers()) {
		if (caseInsensitiveEqual(child->getName(), bEscName)) {
			bEscVar = child.get();
			break;
		}
	}

	if (!bEscVar || !bEscVar->isVariable())
		return false;

	DynamicValue bEscValue;
	static_cast<VariableModifier *>(bEscVar)->varGetValue(bEscValue);

	if (bEscValue.getType() != DynamicValueTypes::kBoolean || !bEscValue.getBool())
		return false;

	return true;
}

Common::SharedPtr<ISaveWriter> ObsidianSaveLoadMechanism::createSaveWriter(Runtime *runtime) {
	Project *project = runtime->getProject();

	Common::String cgstName("cGSt");

	Modifier *cgstCompoundVar = nullptr;
	for (const Common::SharedPtr<Modifier> &child : project->getModifiers()) {
		if (caseInsensitiveEqual(child->getName(), cgstName)) {
			cgstCompoundVar = child.get();
			break;
		}
	}

	if (!cgstCompoundVar)
		return nullptr;

	if (cgstCompoundVar->getSaveLoad(runtime))
		return Common::SharedPtr<CompoundVarSaver>(new CompoundVarSaver(runtime, cgstCompoundVar));

	return nullptr;
}

void addObsidianSaveMechanism(const MTropolisGameDescription &desc, Hacks &hacks) {
	Common::SharedPtr<ObsidianSaveLoadMechanism> mechanism(new ObsidianSaveLoadMechanism());
	hacks.addSaveLoadMechanismHooks(mechanism);
}

class MTIMolassesHandler : public IPostEffect {
public:
	void setFullScreenSurface(const Graphics::ManagedSurface &srcSurf);
	void wipeRect(const Common::Rect &rect);
	void setInitialRect(const Common::Rect &rect);

	void release();

	void renderPostEffect(Graphics::ManagedSurface &surface) const override;

private:
	Graphics::ManagedSurface _surf;
	Graphics::ManagedSurface _mask;
	Common::Rect _initialRect;
};

void MTIMolassesHandler::setFullScreenSurface(const Graphics::ManagedSurface &srcSurf) {
	_surf.copyFrom(srcSurf);
	_mask.create(srcSurf.w, srcSurf.h, Graphics::PixelFormat::createFormatCLUT8());
	_mask.fillRect(Common::Rect(0, 0, srcSurf.w, srcSurf.h), 0xff);

	wipeRect(_initialRect);
}

void MTIMolassesHandler::wipeRect(const Common::Rect &rect) {
	_mask.fillRect(rect, 0);
}

void MTIMolassesHandler::setInitialRect(const Common::Rect &rect) {
	_initialRect = rect;
}

void MTIMolassesHandler::release() {
	_surf.free();
	_mask.free();
}

void MTIMolassesHandler::renderPostEffect(Graphics::ManagedSurface &surface) const {
	surface.transBlitFrom(_surf, Common::Point(0, 0), _mask);
}

class MTIMolassesFullscreenHooks : public StructuralHooks {
public:
	explicit MTIMolassesFullscreenHooks(const Common::SharedPtr<MTIMolassesHandler> &molassesHandler);

	void onStopPlayingMToon(Structural *structural, bool &hide, bool &stopped, Graphics::ManagedSurface *lastSurf) override;

private:
	Common::SharedPtr<MTIMolassesHandler> _molassesHandler;
};

MTIMolassesFullscreenHooks::MTIMolassesFullscreenHooks(const Common::SharedPtr<MTIMolassesHandler> &molassesHandler) : _molassesHandler(molassesHandler) {
}

void MTIMolassesFullscreenHooks::onStopPlayingMToon(Structural *structural, bool &hide, bool &stopped, Graphics::ManagedSurface *lastSurf) {
	_molassesHandler->setFullScreenSurface(*lastSurf);
}

class MTIMolassesSpongeHooks : public StructuralHooks {
public:
	explicit MTIMolassesSpongeHooks(const Common::SharedPtr<MTIMolassesHandler> &molassesHandler);

	void onPostActivate(Structural *structural) override;
	void onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) override;

private:
	Common::SharedPtr<MTIMolassesHandler> _molassesHandler;
};

MTIMolassesSpongeHooks::MTIMolassesSpongeHooks(const Common::SharedPtr<MTIMolassesHandler> &molassesHandler) : _molassesHandler(molassesHandler) {
}

void MTIMolassesSpongeHooks::onPostActivate(Structural *structural) {
	VisualElement *visual = static_cast<VisualElement *>(structural);

	_molassesHandler->setInitialRect(visual->getRelativeRect());
}

void MTIMolassesSpongeHooks::onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) {
	const Common::Rect relRect = static_cast<VisualElement *>(structural)->getRelativeRect();

	int16 w = relRect.width();
	int16 h = relRect.height();

	Common::Rect wipeRect = Common::Rect(pt.x, pt.y, pt.x + w, pt.y + h);

	_molassesHandler->wipeRect(wipeRect);
}

class MTIMolassesSceneTransitionHooks : public SceneTransitionHooks {
public:
	explicit MTIMolassesSceneTransitionHooks(const Common::SharedPtr<MTIMolassesHandler> &molassesHandler);

	void onSceneTransitionSetup(Runtime *runtime, const Common::WeakPtr<Structural> &oldScene, const Common::WeakPtr<Structural> &newScene) override;

private:
	Common::SharedPtr<MTIMolassesHandler> _molassesHandler;
};

MTIMolassesSceneTransitionHooks::MTIMolassesSceneTransitionHooks(const Common::SharedPtr<MTIMolassesHandler> &molassesHandler) : _molassesHandler(molassesHandler) {
}

void MTIMolassesSceneTransitionHooks::onSceneTransitionSetup(Runtime *runtime, const Common::WeakPtr<Structural> &oldScene, const Common::WeakPtr<Structural> &newScene) {
	Structural *oldScenePtr = oldScene.lock().get();
	Structural *newScenePtr = newScene.lock().get();

	const char *molassesSceneName = "B01c: Molasses";

	if (oldScenePtr && oldScenePtr->getName() == molassesSceneName)
		runtime->removePostEffect(_molassesHandler.get());
	else if (newScenePtr && newScenePtr->getName() == molassesSceneName)
		runtime->addPostEffect(_molassesHandler.get());
}

class MTIBuggyAnimationHooks : public StructuralHooks {
public:
	void onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) override;
	void onStopPlayingMToon(Structural *structural, bool &hide, bool &stopped, Graphics::ManagedSurface *lastSurf) override;
	void onHidden(Structural *structural, bool &visible) override;
};

void MTIBuggyAnimationHooks::onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) {
	// Cancel out off-screen translation
	if (pt.x < 0)
		pt = oldPt;
}

void MTIBuggyAnimationHooks::onStopPlayingMToon(Structural *structural, bool &visible, bool &stopped, Graphics::ManagedSurface *lastSurf) {
	// Un-stop
	visible = true;
	stopped = false;
}

void MTIBuggyAnimationHooks::onHidden(Structural *structural, bool &visible) {
	// Un-hide
	visible = true;
}

class MTIStructuralHooks : public StructuralHooks {
public:
	explicit MTIStructuralHooks(const Common::SharedPtr<MTIMolassesHandler> &molassesHandler);

	void onPostActivate(Structural *structural) override;

private:
	Common::SharedPtr<MTIMolassesHandler> _molassesHandler;
};

MTIStructuralHooks::MTIStructuralHooks(const Common::SharedPtr<MTIMolassesHandler> &molassesHandler) : _molassesHandler(molassesHandler) {
}

void MTIStructuralHooks::onPostActivate(Structural *structural) {
	const Common::String &name = structural->getName();

	if (name == "D15_0003.tun") {
		// Fix for intro not playing in Piggy's secret room.  D15_0003.tun is on layer 7 but treasure layers are on top of it.
		if (structural->isElement() && static_cast<Element *>(structural)->isVisual())
			static_cast<VisualElement *>(structural)->setLayer(20);
	} else if (name == "C01c0005.tun" || name == "C01c0005a.tun" || name == "A06_Xspot.tun" || name == "A08agp01.tun") {
		// Several animations stop (which hides) and are moved off-screen when they stop, yet for some reason are supposed to
		// continue to draw.
		//
		// Known cases:
		// - Treasure map in Benbow (A06_Xspot.tun)
		// - Molasses when leaving Benbow (A08agp01.tun)
		// - Long John Silver in the life boat when disembarking the Hispaniola (C01c0005.tun and C01c0005a.tun)
		structural->setHooks(Common::SharedPtr<StructuralHooks>(new MTIBuggyAnimationHooks()));
	} else if (name == "B01cgp01.tun") {
		structural->setHooks(Common::SharedPtr<StructuralHooks>(new MTIMolassesFullscreenHooks(_molassesHandler)));
	} else if (name == "B01c_newsponge.tun") {
		structural->setHooks(Common::SharedPtr<StructuralHooks>(new MTIMolassesSpongeHooks(_molassesHandler)));
		structural->getHooks()->onPostActivate(structural);
	} else if (name == "E01_Beakerfly.tun") {
		// Beaker flyby in the intro is on layer 4, but the door is on layer 11
		static_cast<VisualElement *>(structural)->setLayer(12);
	}
}

void addMTIQuirks(const MTropolisGameDescription &desc, Hacks &hacks) {
	// MTI uses a lot of "maintain rate" mToons at 10Hz.  This means their frame timer resets on every frame advance, and
	// is supposed to ensure that the mToon plays back at a smooth rate regardless of clock jitter.  Unfortunately, it
	// does this with mToons that are synchronized to sounds, which is bad!  Presumably the reason this wasn't a problem
	// is because MacOS runs with a 60Hz tick clock so it always divides evenly into the frame rate, and Windows... not sure.
	//
	// Anyway, there are two possible solutions to this: Lock the clock to 60Hz, or ignore the flag.
	// Given that the flag should not be set, we ignore the flag.
	hacks.ignoreMToonMaintainRateFlag = true;

	// Needed for piano, the sound assets are in B10 : Notes but the piano is in B10 : Piano
	hacks.allowAssetsFromOtherScenes = true;

	// MTI initializes variables in a way that doesn't seem to match mTropolis behavior in any explicable way:
	//
	// For example, 0010cb0e "Scene Started => init Benbow" looks like this internally, decompiled:
	// set local:a.billystate to 0
	//
	// In this case "a" is a compound variable and "billyState" is a NON-ALIASED integer variable contained in
	// the compound.  Later, 0009fc9a "Scene Started => play intro" checks local 00007f83 00 'billyState'
	// to determine if the Benbow intro needs to be played.  Since the GUID doesn't match (?) we check by name,
	// which resolves to the GUID-less (?) alias in the Benbow subsection, which references 00097cf4, a different
	// variable also named "billyState"
	//
	// Haven't figured out anything that would explain why it would reference the variables in the compound
	// modifier.  Probably some quirk of early-version mTropolis.
	hacks.mtiVariableReferencesHack = true;

	// MTI returns from the menu by transitioning to a "return" scene that sends a return message to the target
	// scene, which is supposed to activate a scene transition modifier in the scene that transitions to itself.
	// This doesn't work because the modifier is gone when the scene is unloaded.
	hacks.mtiSceneReturnHack = true;

	// Modifier "Scene Started => helmSteered? => Play Character, else Play Char" has a non-null-terminated name,
	// which causes an integrity check failure when disembarking the Hispaniola.
	hacks.mtiHispaniolaDamagedStringHack = true;

	Common::SharedPtr<MTIMolassesHandler> molassesHandler(new MTIMolassesHandler());

	hacks.defaultStructuralHooks.reset(new MTIStructuralHooks(molassesHandler));
	hacks.addSceneTransitionHooks(Common::SharedPtr<SceneTransitionHooks>(new MTIMolassesSceneTransitionHooks(molassesHandler)));
}

} // End of namespace HackSuites

} // End of namespace MTropolis
