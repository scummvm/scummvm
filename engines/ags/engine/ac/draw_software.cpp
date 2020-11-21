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
// Software drawing component. Optimizes drawing for software renderer using
// dirty rectangles technique.
//
// TODO: do research/profiling to find out if this dirty rectangles thing
// is still giving ANY notable perfomance boost at all.
//
// TODO: would that give any benefit to reorganize the code and move dirty
// rectangles into SoftwareGraphicDriver?
// Alternatively: we could pass dirty rects struct pointer and room background
// DDB when calling BeginSpriteBatch(). Driver itself could be calling
// update_invalid_region(). That will keep gfx driver's changes to minimum.
//
// NOTE: this code, including structs and functions, has underwent several
// iterations of changes. Originally it was meant to perform full transform
// of dirty rects right away, but later I realized it won't work that way
// because a) Allegro does not support scaling bitmaps over destination with
// different colour depth (which may be a case when running 16-bit game),
// and b) Allegro does not support scaling and rotating of sprites with
// blending and lighting at the same time which means that room objects have
// to be drawn upon non-scaled background first. Possibly some of the code
// below may be therefore simplified.
//
//=============================================================================

#include <string.h>
#include <vector>
#include "ac/draw_software.h"
#include "gfx/bitmap.h"
#include "util/scaling.h"

using namespace AGS::Common;
using namespace AGS::Engine;

// TODO: choose these values depending on game resolution?
#define MAXDIRTYREGIONS 25
#define WHOLESCREENDIRTY (MAXDIRTYREGIONS + 5)
#define MAX_SPANS_PER_ROW 4

// Dirty rects store coordinate values in the coordinate system of a camera surface,
// where coords always span from 0,0 to surface width,height.
// Converting from room to dirty rects would require subtracting room camera offsets.
struct IRSpan
{
    int x1, x2;
    int mergeSpan(int tx1, int tx2);

    IRSpan();
};

struct IRRow
{
    IRSpan span[MAX_SPANS_PER_ROW];
    int numSpans;

    IRRow();
};

struct DirtyRects
{
    // Size of the surface managed by this dirty rects object
    Size SurfaceSize;
    // Where the surface is rendered on screen
    Rect Viewport;
    // Room -> screen coordinate transformation
    PlaneScaling Room2Screen;
    // Screen -> dirty surface rect
    // The dirty rects are saved in coordinates limited to (0,0)->(camera size) rather than room or screen coords
    PlaneScaling Screen2DirtySurf;

    std::vector<IRRow> DirtyRows;
    Rect DirtyRegions[MAXDIRTYREGIONS];
    size_t NumDirtyRegions;

    DirtyRects();
    bool IsInit() const;
    // Initialize dirty rects for the given surface size
    void Init(const Size &surf_size, const Rect &viewport);
    void SetSurfaceOffsets(int x, int y);
    // Delete dirty rects
    void Destroy();
    // Mark all surface as tidy
    void Reset();
};


IRSpan::IRSpan()
    : x1(0), x2(0)
{
}

IRRow::IRRow()
    : numSpans(0)
{
}

int IRSpan::mergeSpan(int tx1, int tx2)
{
    if ((tx1 > x2) || (tx2 < x1))
        return 0;
    // overlapping, increase the span
    if (tx1 < x1)
        x1 = tx1;
    if (tx2 > x2)
        x2 = tx2;
    return 1;
}

DirtyRects::DirtyRects()
    : NumDirtyRegions(0)
{
}

bool DirtyRects::IsInit() const
{
    return DirtyRows.size() > 0;
}

void DirtyRects::Init(const Size &surf_size, const Rect &viewport)
{
    int height = surf_size.Height;
    if (SurfaceSize != surf_size)
    {
        Destroy();
        SurfaceSize = surf_size;
        DirtyRows.resize(height);

        NumDirtyRegions = WHOLESCREENDIRTY;
        for (int i = 0; i < height; ++i)
            DirtyRows[i].numSpans = 0;
    }

    Viewport = viewport;
    Room2Screen.Init(surf_size, viewport);
    Screen2DirtySurf.Init(viewport, RectWH(0, 0, surf_size.Width, surf_size.Height));
}

void DirtyRects::SetSurfaceOffsets(int x, int y)
{
    Room2Screen.SetSrcOffsets(x, y);
}

void DirtyRects::Destroy()
{
    DirtyRows.clear();
    NumDirtyRegions = 0;
}

void DirtyRects::Reset()
{
    NumDirtyRegions = 0;

    for (size_t i = 0; i < DirtyRows.size(); ++i)
        DirtyRows[i].numSpans = 0;
}

// Dirty rects for the main viewport background (black screen);
// these are used when the room viewport does not cover whole screen,
// so that we know when to paint black after mouse cursor and gui.
DirtyRects BlackRects;
// Dirty rects object for the single room camera
std::vector<DirtyRects> RoomCamRects;
// Saved room camera offsets to know if we must invalidate whole surface.
// TODO: if we support rotation then we also need to compare full transform!
std::vector<std::pair<int, int>> RoomCamPositions;


void dispose_invalid_regions(bool /* room_only */)
{
    RoomCamRects.clear();
    RoomCamPositions.clear();
}

void init_invalid_regions(int view_index, const Size &surf_size, const Rect &viewport)
{
    if (view_index < 0)
    {
        BlackRects.Init(surf_size, viewport);
    }
    else
    {
        if (RoomCamRects.size() <= (size_t)view_index)
        {
            RoomCamRects.resize(view_index + 1);
            RoomCamPositions.resize(view_index + 1);
        }
        RoomCamRects[view_index].Init(surf_size, viewport);
        RoomCamPositions[view_index] = std::make_pair(-1000, -1000);
    }
}

void delete_invalid_regions(int view_index)
{
    if (view_index >= 0)
    {
        RoomCamRects.erase(RoomCamRects.begin() + view_index);
        RoomCamPositions.erase(RoomCamPositions.begin() + view_index);
    }
}

void set_invalidrects_cameraoffs(int view_index, int x, int y)
{
    if (view_index < 0)
    {
        BlackRects.SetSurfaceOffsets(x, y);
        return;
    }
    else
    {
        RoomCamRects[view_index].SetSurfaceOffsets(x, y);
    }

    int &posxwas = RoomCamPositions[view_index].first;
    int &posywas = RoomCamPositions[view_index].second;
    if ((x != posxwas) || (y != posywas))
    {
        invalidate_all_camera_rects(view_index);
        posxwas = x;
        posywas = y;
    }
}

void invalidate_all_rects()
{
    for (auto &rects : RoomCamRects)
    {
        if (!IsRectInsideRect(rects.Viewport, BlackRects.Viewport))
            BlackRects.NumDirtyRegions = WHOLESCREENDIRTY;
        rects.NumDirtyRegions = WHOLESCREENDIRTY;
    }
}

void invalidate_all_camera_rects(int view_index)
{
    if (view_index < 0)
        return;
    RoomCamRects[view_index].NumDirtyRegions = WHOLESCREENDIRTY;
}

void invalidate_rect_on_surf(int x1, int y1, int x2, int y2, DirtyRects &rects)
{
    if (rects.DirtyRows.size() == 0)
        return;
    if (rects.NumDirtyRegions >= MAXDIRTYREGIONS) {
        // too many invalid rectangles, just mark the whole thing dirty
        rects.NumDirtyRegions = WHOLESCREENDIRTY;
        return;
    }

    int a;

    const Size &surfsz = rects.SurfaceSize;
    if (x1 >= surfsz.Width) x1 = surfsz.Width - 1;
    if (y1 >= surfsz.Height) y1 = surfsz.Height - 1;
    if (x2 >= surfsz.Width) x2 = surfsz.Width - 1;
    if (y2 >= surfsz.Height) y2 = surfsz.Height - 1;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 < 0) x2 = 0;
    if (y2 < 0) y2 = 0;
    rects.NumDirtyRegions++;

    // ** Span code
    std::vector<IRRow> &dirtyRow = rects.DirtyRows;
    int s, foundOne;
    // add this rect to the list for this row
    for (a = y1; a <= y2; a++) {
        foundOne = 0;
        for (s = 0; s < dirtyRow[a].numSpans; s++) {
            if (dirtyRow[a].span[s].mergeSpan(x1, x2)) {
                foundOne = 1;
                break;
            }
        }
        if (foundOne) {
            // we were merged into a span, so we're ok
            int t;
            // check whether now two of the spans overlap each other
            // in which case merge them
            for (s = 0; s < dirtyRow[a].numSpans; s++) {
                for (t = s + 1; t < dirtyRow[a].numSpans; t++) {
                    if (dirtyRow[a].span[s].mergeSpan(dirtyRow[a].span[t].x1, dirtyRow[a].span[t].x2)) {
                        dirtyRow[a].numSpans--;
                        for (int u = t; u < dirtyRow[a].numSpans; u++)
                            dirtyRow[a].span[u] = dirtyRow[a].span[u + 1];
                        break;
                    }
                }
            }
        }
        else if (dirtyRow[a].numSpans < MAX_SPANS_PER_ROW) {
            dirtyRow[a].span[dirtyRow[a].numSpans].x1 = x1;
            dirtyRow[a].span[dirtyRow[a].numSpans].x2 = x2;
            dirtyRow[a].numSpans++;
        }
        else {
            // didn't fit in an existing span, and there are none spare
            int nearestDist = 99999, nearestWas = -1, extendLeft;
            int tleft, tright;
            // find the nearest span, and enlarge that to include this rect
            for (s = 0; s < dirtyRow[a].numSpans; s++) {
                tleft = dirtyRow[a].span[s].x1 - x2;
                if ((tleft > 0) && (tleft < nearestDist)) {
                    nearestDist = tleft;
                    nearestWas = s;
                    extendLeft = 1;
                }
                tright = x1 - dirtyRow[a].span[s].x2;
                if ((tright > 0) && (tright < nearestDist)) {
                    nearestDist = tright;
                    nearestWas = s;
                    extendLeft = 0;
                }
            }
            if (extendLeft)
                dirtyRow[a].span[nearestWas].x1 = x1;
            else
                dirtyRow[a].span[nearestWas].x2 = x2;
        }
    }
    // ** End span code
    //}
}

void invalidate_rect_ds(DirtyRects &rects, int x1, int y1, int x2, int y2, bool in_room)
{
    if (!in_room)
    {
        // TODO: for most opimisation (esp. with multiple viewports) should perhaps
        // split/cut parts of the original rectangle which overlap room viewport(s).
        Rect r(x1, y1, x2, y2);
        // If overlay is NOT completely over the room, then invalidate black rect
        if (!IsRectInsideRect(rects.Viewport, r))
            invalidate_rect_on_surf(x1, y1, x2, y2, BlackRects);
        // If overlay is NOT intersecting room viewport at all, then stop
        if (!AreRectsIntersecting(rects.Viewport, r))
            return;

        // Transform from screen to room coordinates through the known viewport
        x1 = rects.Screen2DirtySurf.X.ScalePt(x1);
        x2 = rects.Screen2DirtySurf.X.ScalePt(x2);
        y1 = rects.Screen2DirtySurf.Y.ScalePt(y1);
        y2 = rects.Screen2DirtySurf.Y.ScalePt(y2);
    }
    else
    {
        x1 -= rects.Room2Screen.X.GetSrcOffset();
        y1 -= rects.Room2Screen.Y.GetSrcOffset();
        x2 -= rects.Room2Screen.X.GetSrcOffset();
        y2 -= rects.Room2Screen.Y.GetSrcOffset();
    }

    invalidate_rect_on_surf(x1, y1, x2, y2, rects);
}

void invalidate_rect_ds(int x1, int y1, int x2, int y2, bool in_room)
{
    for (auto &rects : RoomCamRects)
        invalidate_rect_ds(rects, x1, y1, x2, y2, in_room);
}

// Note that this function is denied to perform any kind of scaling or other transformation
// other than blitting with offset. This is mainly because destination could be a 32-bit virtual screen
// while room background was 16-bit and Allegro lib does not support stretching between colour depths.
// The no_transform flag here means essentially "no offset", and indicates that the function
// must blit src on ds at 0;0. Otherwise, actual Viewport offset is used.
void update_invalid_region(Bitmap *ds, Bitmap *src, const DirtyRects &rects, bool no_transform)
{
    if (rects.NumDirtyRegions == 0)
        return;

    if (!no_transform)
        ds->SetClip(rects.Viewport);

    const int src_x = rects.Room2Screen.X.GetSrcOffset();
    const int src_y = rects.Room2Screen.Y.GetSrcOffset();
    const int dst_x = no_transform ? 0 : rects.Viewport.Left;
    const int dst_y = no_transform ? 0 : rects.Viewport.Top;

    if (rects.NumDirtyRegions == WHOLESCREENDIRTY)
    {
        ds->Blit(src, src_x, src_y, dst_x, dst_y, rects.SurfaceSize.Width, rects.SurfaceSize.Height);
    }
    else
    {
        const std::vector<IRRow> &dirtyRow = rects.DirtyRows;
        const int surf_height = rects.SurfaceSize.Height;
        // TODO: is this IsMemoryBitmap check is still relevant?
        // If bitmaps properties match and no transform required other than linear offset
        if ((src->GetColorDepth() == ds->GetColorDepth()) && (ds->IsMemoryBitmap()))
        {
            const int bypp = src->GetBPP();
            // do the fast memory copy
            for (int i = 0; i < surf_height; i++)
            {
                const uint8_t *src_scanline = src->GetScanLine(i + src_y);
                uint8_t *dst_scanline = ds->GetScanLineForWriting(i + dst_y);
                const IRRow &dirty_row = dirtyRow[i];
                for (int k = 0; k < dirty_row.numSpans; k++)
                {
                    int tx1 = dirty_row.span[k].x1;
                    int tx2 = dirty_row.span[k].x2;
                    memcpy(&dst_scanline[(tx1 + dst_x) * bypp], &src_scanline[(tx1 + src_x) * bypp], ((tx2 - tx1) + 1) * bypp);
                }
            }
        }
        // If has to use Blit, but still must draw with no transform but offset
        else
        {
            // do fast copy without transform
            for (int i = 0, rowsInOne = 1; i < surf_height; i += rowsInOne, rowsInOne = 1)
            {
                // if there are rows with identical masks, do them all in one go
                // TODO: what is this for? may this be done at the invalidate_rect merge step?
                while ((i + rowsInOne < surf_height) && (memcmp(&dirtyRow[i], &dirtyRow[i + rowsInOne], sizeof(IRRow)) == 0))
                    rowsInOne++;

                const IRRow &dirty_row = dirtyRow[i];
                for (int k = 0; k < dirty_row.numSpans; k++)
                {
                    int tx1 = dirty_row.span[k].x1;
                    int tx2 = dirty_row.span[k].x2;
                    ds->Blit(src, tx1 + src_x, i + src_y, tx1 + dst_x, i + dst_y, (tx2 - tx1) + 1, rowsInOne);
                }
            }
        }
    }
}

void update_invalid_region(Bitmap *ds, color_t fill_color, const DirtyRects &rects)
{
    ds->SetClip(rects.Viewport);

    if (rects.NumDirtyRegions == WHOLESCREENDIRTY)
    {
        ds->FillRect(rects.Viewport, fill_color);
    }
    else
    {
        const std::vector<IRRow> &dirtyRow = rects.DirtyRows;
        const int surf_height = rects.SurfaceSize.Height;
        {
            const PlaneScaling &tf = rects.Room2Screen;
            for (int i = 0, rowsInOne = 1; i < surf_height; i += rowsInOne, rowsInOne = 1)
            {
                // if there are rows with identical masks, do them all in one go
                // TODO: what is this for? may this be done at the invalidate_rect merge step?
                while ((i + rowsInOne < surf_height) && (memcmp(&dirtyRow[i], &dirtyRow[i + rowsInOne], sizeof(IRRow)) == 0))
                    rowsInOne++;

                const IRRow &dirty_row = dirtyRow[i];
                for (int k = 0; k < dirty_row.numSpans; k++)
                {
                    Rect src_r(dirty_row.span[k].x1, i, dirty_row.span[k].x2, i + rowsInOne - 1);
                    Rect dst_r = tf.ScaleRange(src_r);
                    ds->FillRect(dst_r, fill_color);
                }
            }
        }
    }
}

void update_black_invreg_and_reset(Bitmap *ds)
{
    if (!BlackRects.IsInit())
        return;
    update_invalid_region(ds, (color_t)0, BlackRects);
    BlackRects.Reset();
}

void update_room_invreg_and_reset(int view_index, Bitmap *ds, Bitmap *src, bool no_transform)
{
    if (view_index < 0 || RoomCamRects.size() == 0)
        return;
    
    update_invalid_region(ds, src, RoomCamRects[view_index], no_transform);
    RoomCamRects[view_index].Reset();
}
