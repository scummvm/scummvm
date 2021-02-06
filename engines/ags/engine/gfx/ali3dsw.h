//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// Software graphics factory, based on Allegro
//
//=============================================================================

#ifndef __AGS_EE_GFX__ALI3DSW_H
#define __AGS_EE_GFX__ALI3DSW_H

#include <memory>

#include "core/platform.h"
#define AGS_DDRAW_GAMMA_CONTROL (AGS_PLATFORM_OS_WINDOWS)

#include <allegro.h>

#if AGS_DDRAW_GAMMA_CONTROL
#include <winalleg.h>
#include <ddraw.h>
#endif

#include "gfx/bitmap.h"
#include "gfx/ddb.h"
#include "gfx/gfxdriverfactorybase.h"
#include "gfx/gfxdriverbase.h"

namespace AGS
{
namespace Engine
{
namespace ALSW
{

class AllegroGfxFilter;
using AGS::Common::Bitmap;

class ALSoftwareBitmap : public IDriverDependantBitmap
{
public:
    // NOTE by CJ:
    // Transparency is a bit counter-intuitive
    // 0=not transparent, 255=invisible, 1..254 barely visible .. mostly visible
    void SetTransparency(int transparency) override { _transparency = transparency; }
    void SetFlippedLeftRight(bool isFlipped) override { _flipped = isFlipped; }
    void SetStretch(int width, int height, bool useResampler = true) override 
    {
        _stretchToWidth = width;
        _stretchToHeight = height;
    }
    int GetWidth() override { return _width; }
    int GetHeight() override { return _height; }
    int GetColorDepth() override { return _colDepth; }
    void SetLightLevel(int lightLevel) override  { }
    void SetTint(int red, int green, int blue, int tintSaturation) override { }

    Bitmap *_bmp;
    int _width, _height;
    int _colDepth;
    bool _flipped;
    int _stretchToWidth, _stretchToHeight;
    bool _opaque; // no mask color
    bool _hasAlpha;
    int _transparency;

    ALSoftwareBitmap(Bitmap *bmp, bool opaque, bool hasAlpha)
    {
        _bmp = bmp;
        _width = bmp->GetWidth();
        _height = bmp->GetHeight();
        _colDepth = bmp->GetColorDepth();
        _flipped = false;
        _stretchToWidth = 0;
        _stretchToHeight = 0;
        _transparency = 0;
        _opaque = opaque;
        _hasAlpha = hasAlpha;
    }

    int GetWidthToRender() { return (_stretchToWidth > 0) ? _stretchToWidth : _width; }
    int GetHeightToRender() { return (_stretchToHeight > 0) ? _stretchToHeight : _height; }

    void Dispose()
    {
        // do we want to free the bitmap?
    }

    ~ALSoftwareBitmap() override
    {
        Dispose();
    }
};


class ALSoftwareGfxModeList : public IGfxModeList
{
public:
    ALSoftwareGfxModeList(GFX_MODE_LIST *alsw_gfx_mode_list)
        : _gfxModeList(alsw_gfx_mode_list)
    {
    }

    int GetModeCount() const override
    {
        return _gfxModeList ? _gfxModeList->num_modes : 0;
    }

    bool GetMode(int index, DisplayMode &mode) const override;

private:
    GFX_MODE_LIST *_gfxModeList;
};


typedef SpriteDrawListEntry<ALSoftwareBitmap> ALDrawListEntry;
// Software renderer's sprite batch
struct ALSpriteBatch
{
    // List of sprites to render
    std::vector<ALDrawListEntry> List;
    // Intermediate surface which will be drawn upon and transformed if necessary
    std::shared_ptr<Bitmap>      Surface;
    // Whether surface is a virtual screen's region
    bool                         IsVirtualScreen;
    // Tells whether the surface is treated as opaque or transparent
    bool                         Opaque;
};
typedef std::vector<ALSpriteBatch> ALSpriteBatches;


class ALSoftwareGraphicsDriver : public GraphicsDriverBase
{
public:
    ALSoftwareGraphicsDriver();

    const char*GetDriverName() override { return "Software renderer"; }
    const char*GetDriverID() override { return "Software"; }
    void SetTintMethod(TintMethod method) override;
    bool SetDisplayMode(const DisplayMode &mode, volatile int *loopTimer) override;
    bool SetNativeSize(const Size &src_size) override;
    bool SetRenderFrame(const Rect &dst_rect) override;
    bool IsModeSupported(const DisplayMode &mode) override;
    int  GetDisplayDepthForNativeDepth(int native_color_depth) const override;
    IGfxModeList *GetSupportedModeList(int color_depth) override;
    PGfxFilter GetGraphicsFilter() const override;
    void UnInit();
    // Clears the screen rectangle. The coordinates are expected in the **native game resolution**.
    void ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse) override;
    int  GetCompatibleBitmapFormat(int color_depth) override;
    IDriverDependantBitmap* CreateDDBFromBitmap(Bitmap *bitmap, bool hasAlpha, bool opaque) override;
    void UpdateDDBFromBitmap(IDriverDependantBitmap* bitmapToUpdate, Bitmap *bitmap, bool hasAlpha) override;
    void DestroyDDB(IDriverDependantBitmap* bitmap) override;

    void DrawSprite(int x, int y, IDriverDependantBitmap* bitmap) override;
    void SetScreenFade(int red, int green, int blue) override;
    void SetScreenTint(int red, int green, int blue) override;

    void RenderToBackBuffer() override;
    void Render() override;
    void Render(int xoff, int yoff, GlobalFlipType flip) override;
    bool GetCopyOfScreenIntoBitmap(Bitmap *destination, bool at_native_res, GraphicResolution *want_fmt) override;
    void FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) override;
    void FadeIn(int speed, PALETTE pal, int targetColourRed, int targetColourGreen, int targetColourBlue) override;
    void BoxOutEffect(bool blackingOut, int speed, int delay) override;
#ifndef AGS_NO_VIDEO_PLAYER
    bool PlayVideo(const char *filename, bool useAVISound, VideoSkipType skipType, bool stretchToFullScreen) override;
#endif
    bool SupportsGammaControl() override ;
    void SetGamma(int newGamma) override;
    void UseSmoothScaling(bool enabled) override { }
    void EnableVsyncBeforeRender(bool enabled) override { _autoVsync = enabled; }
    void Vsync() override;
    void RenderSpritesAtScreenResolution(bool enabled, int supersampling) override { }
    bool RequiresFullRedrawEachFrame() override { return false; }
    bool HasAcceleratedTransform() override { return false; }
    bool UsesMemoryBackBuffer() override { return true; }
    Bitmap *GetMemoryBackBuffer() override;
    void SetMemoryBackBuffer(Bitmap *backBuffer) override;
    Bitmap *GetStageBackBuffer() override;
    ~ALSoftwareGraphicsDriver() override;

    typedef std::shared_ptr<AllegroGfxFilter> PALSWFilter;

    void SetGraphicsFilter(PALSWFilter filter);

private:
    PALSWFilter _filter;

    bool _autoVsync;
    Bitmap *_allegroScreenWrapper;
    // Virtual screen bitmap is either a wrapper over Allegro's real screen
    // bitmap, or bitmap provided by the graphics filter. It should not be
    // disposed by the renderer: it is up to filter object to manage it.
    Bitmap *_origVirtualScreen;
    // Current virtual screen bitmap; may be provided either by graphics
    // filter or by external user. It should not be disposed by the renderer.
    Bitmap *virtualScreen;
    // Stage screen meant for particular rendering stages, may be referencing
    // actual virtual screen or separate bitmap of different size that is
    // blitted to virtual screen at the stage finalization.
    Bitmap *_stageVirtualScreen;
    //Bitmap *_spareTintingScreen;
    int _tint_red, _tint_green, _tint_blue;

    ALSpriteBatches _spriteBatches;
    GFX_MODE_LIST *_gfxModeList;

#if AGS_DDRAW_GAMMA_CONTROL
    IDirectDrawGammaControl* dxGammaControl;
    // The gamma ramp is a lookup table for each possible R, G and B value
    // in 32-bit colour (from 0-255) it maps them to a brightness value
    // from 0-65535. The default gamma ramp just multiplies each value by 256
    DDGAMMARAMP gammaRamp;
    DDGAMMARAMP defaultGammaRamp;
    DDCAPS ddrawCaps;
#endif

    void InitSpriteBatch(size_t index, const SpriteBatchDesc &desc) override;
    void ResetAllBatches() override;

    // Use gfx filter to create a new virtual screen
    void CreateVirtualScreen();
    void DestroyVirtualScreen();
    // Unset parameters and release resources related to the display mode
    void ReleaseDisplayMode();
    // Renders single sprite batch on the precreated surface
    void RenderSpriteBatch(const ALSpriteBatch &batch, Common::Bitmap *surface, int surf_offx, int surf_offy);

    void highcolor_fade_in(Bitmap *vs, void(*draw_callback)(), int offx, int offy, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue);
    void highcolor_fade_out(Bitmap *vs, void(*draw_callback)(), int offx, int offy, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue);
    void __fade_from_range(PALETTE source, PALETTE dest, int speed, int from, int to) ;
    void __fade_out_range(int speed, int from, int to, int targetColourRed, int targetColourGreen, int targetColourBlue) ;
    int  GetAllegroGfxDriverID(bool windowed);
};


class ALSWGraphicsFactory : public GfxDriverFactoryBase<ALSoftwareGraphicsDriver, AllegroGfxFilter>
{
public:
    ~ALSWGraphicsFactory() override;

    size_t               GetFilterCount() const override;
    const GfxFilterInfo *GetFilterInfo(size_t index) const override;
    String               GetDefaultFilterID() const override;

    static  ALSWGraphicsFactory *GetFactory();

private:
    ALSoftwareGraphicsDriver *EnsureDriverCreated() override;
    AllegroGfxFilter         *CreateFilter(const String &id) override;

    static ALSWGraphicsFactory *_factory;
};

} // namespace ALSW
} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__ALI3DSW_H
