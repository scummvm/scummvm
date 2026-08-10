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
#include "common/hashmap.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/movie.h"
#include "director/debugger/dt-internal.h"
#include "director/lingo/lingo-profiler.h"

namespace Director {
namespace DT {

static bool openHandlerScript(const Common::String &handlerName) {
	Movie *movie = g_director ? g_director->getCurrentMovie() : nullptr;
	if (!movie || handlerName.empty())
		return false;

	ScriptContext *ctx = nullptr;
	for (auto &it : *movie->getCasts()) {
		ctx = resolveHandlerContext(-1, CastMemberID(0, it._key), handlerName);
		if (ctx)
			break;
	}
	if (!ctx)
		ctx = resolveHandlerContext(-1, CastMemberID(0, SHARED_CAST_LIB), handlerName);
	if (!ctx)
		return false;

	int castLibID = getCastLibIDForContext(ctx);
	Common::String moviePath = movie->getArchive()->getPathName().toString();
	addToOpenHandlers(buildImGuiHandlerScript(ctx, castLibID, handlerName, moviePath));
	return true;
}

// A resolved begin/end pair with its nested render depth.
struct ProfZone {
	uint32 startSeq;
	uint32 endSeq;
	uint32 startTs;
	uint32 endTs;
	uint32 childTs;		// ms spent in nested calls, for self-time
	uint32 nameId;
	uint32 movieId;
	uint32 startFrame;
	uint32 endFrame;
	uint16 depth;
};

struct StatRow {
	uint32 nameId;
	uint32 count;
	uint64 totalTs;		// inclusive ms across all calls
	uint64 selfTs;		// ms excluding nested calls
};

struct DrawnRect {
	float x0, y0, x1, y1;
	uint32 nameId;
};

static bool statGreater(const StatRow &a, const StatRow &b) {
	if (a.totalTs != b.totalTs)
		return a.totalTs > b.totalTs;
	return a.count > b.count;
}

// Deterministic pastel keyed by handler name, so a handler keeps one color.
static ImU32 nameColor(uint32 id) {
	uint32 h = (id + 1) * 2654435761u;
	int r = 130 + (int)((h >> 1) & 0x4F);
	int g = 130 + (int)((h >> 9) & 0x4F);
	int b = 130 + (int)((h >> 17) & 0x4F);
	return IM_COL32(r, g, b, 255);
}

static inline float fabsff(float f) { return f < 0.0f ? -f : f; }

// Resolve the raw event stream into completed begin/end zones with nesting.
static void rebuildZones(const Common::Array<ProfilerEvent> &events, Common::Array<ProfZone> &zones,
		uint16 &maxRow, uint32 &maxSeq, uint32 &maxTs) {
	zones.clear();
	maxRow = 0;

	Common::Array<uint> openStack;
	Common::Array<uint16> offStack;
	uint16 depthOff = 0;

	for (uint i = 0; i < events.size(); i++) {
		const ProfilerEvent &e = events[i];
		switch (e.type) {
		case kProfBegin: {
			ProfZone z;
			z.startSeq = e.seq;
			z.endSeq = e.seq;
			z.startTs = e.ts;
			z.endTs = e.ts;
			z.childTs = 0;
			z.nameId = e.nameId;
			z.movieId = e.movieId;
			z.startFrame = e.frame;
			z.endFrame = e.frame;
			z.depth = e.depth + depthOff;
			if (z.depth > maxRow)
				maxRow = z.depth;
			zones.push_back(z);
			openStack.push_back(zones.size() - 1);
			break;
		}
		case kProfEnd:
			if (!openStack.empty()) {
				uint idx = openStack.back();
				openStack.pop_back();
				zones[idx].endSeq = e.seq;
				zones[idx].endTs = e.ts;
				zones[idx].endFrame = e.frame;
				// Credit inclusive time to the parent for its self-time.
				if (!openStack.empty())
					zones[openStack.back()].childTs += zones[idx].endTs - zones[idx].startTs;
			}
			break;
		case kProfFreeze:
			offStack.push_back(depthOff);
			depthOff += e.depth;
			break;
		case kProfThaw:
			if (!offStack.empty()) {
				depthOff = offStack.back();
				offStack.pop_back();
			}
			break;
		default:
			break;
		}
	}

	maxSeq = events.empty() ? 0 : events.back().seq;
	maxTs = events.empty() ? 0 : events.back().ts;
	for (uint i = 0; i < openStack.size(); i++) {
		zones[openStack[i]].endSeq = maxSeq + 1;
		zones[openStack[i]].endTs = maxTs;
	}
}

// Per-handler aggregate table (inclusive and self time), most costly first.
static void drawStatistics(LingoProfiler *prof, const Common::Array<ProfZone> &zones) {
	if (!ImGui::CollapsingHeader("Statistics"))
		return;

	Common::HashMap<uint32, uint> idxByName;
	Common::Array<StatRow> rows;
	for (uint i = 0; i < zones.size(); i++) {
		const ProfZone &z = zones[i];
		uint32 total = z.endTs > z.startTs ? z.endTs - z.startTs : 0;
		uint32 self = total > z.childTs ? total - z.childTs : 0;
		Common::HashMap<uint32, uint>::iterator it = idxByName.find(z.nameId);
		if (it == idxByName.end()) {
			StatRow r;
			r.nameId = z.nameId;
			r.count = 1;
			r.totalTs = total;
			r.selfTs = self;
			idxByName[z.nameId] = rows.size();
			rows.push_back(r);
		} else {
			rows[it->_value].count++;
			rows[it->_value].totalTs += total;
			rows[it->_value].selfTs += self;
		}
	}
	Common::sort(rows.begin(), rows.end(), statGreater);

	ImGui::BeginChild("##stats", ImVec2(0, 160.0f), ImGuiChildFlags_Borders);
	ImGui::Text("%8s  %10s  %10s  %s", "calls", "total(ms)", "self(ms)", "handler (click to open)");
	uint shown = rows.size() < 50 ? rows.size() : 50;
	for (uint i = 0; i < shown; i++) {
		Common::String row = Common::String::format("%8u  %10.0f  %10.0f  %s", rows[i].count,
			(double)rows[i].totalTs, (double)rows[i].selfTs, prof->internedName(rows[i].nameId).c_str());
		ImGui::PushID((int)i);
		if (ImGui::Selectable(row.c_str()))
			openHandlerScript(prof->internedName(rows[i].nameId));
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void showProfiler() {
	if (!_state->_w.profiler)
		return;

	ImGui::SetNextWindowSize(ImVec2(1000, 640), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Profiler", &_state->_w.profiler)) {
		ImGui::End();
		return;
	}

	LingoProfiler *prof = g_director ? g_director->_lingoProfiler : nullptr;
	if (!prof) {
		ImGui::TextUnformatted("Profiler unavailable");
		ImGui::End();
		return;
	}

	const DebuggerTheme &theme = *_state->theme;

	static Common::Array<ProfZone> zones;
	static uint builtCount = 0xFFFFFFFFu;
	static uint16 maxRow = 0;
	static uint32 maxSeq = 0;
	static uint32 maxTs = 0;
	static float viewStart = 0.0f;
	static float viewSpan = 200.0f;
	static bool follow = true;
	static int selected = -1;

	// Eased zoom animation (Fit / double-click a zone).
	static bool animActive = false;
	static float animT = 0.0f;
	static float aFromStart = 0.0f, aToStart = 0.0f, aFromSpan = 0.0f, aToSpan = 0.0f;

	bool capturing = prof->isEnabled();
	if (ImGui::Checkbox("Capture", &capturing))
		prof->setEnabled(capturing);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Record Lingo handler calls, freeze/thaw and frames.\nOff by default so it costs nothing.");

	ImGui::SameLine();
	if (ImGui::Button("Clear"))
		prof->clear();

	ImGui::SameLine();
	ImGui::Checkbox("Follow live", &follow);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Keep the newest event (red line) at the right edge.\nAny zoom/pan turns this off.");

	// Rebuild the zone cache when the trace changed. Zones keep their indices
	// across rebuilds (events only ever append), so the selection survives.
	const Common::Array<ProfilerEvent> &events = prof->events();
	if (builtCount != events.size()) {
		builtCount = events.size();
		rebuildZones(events, zones, maxRow, maxSeq, maxTs);
		if (selected >= (int)zones.size())
			selected = -1;
		if (viewSpan < 4.0f)
			viewSpan = 200.0f;
	}

	ImGui::SameLine();
	if (ImGui::Button("Fit")) {
		aFromStart = viewStart;
		aFromSpan = viewSpan;
		aToStart = 0.0f;
		aToSpan = (maxSeq > 10) ? (float)maxSeq : 200.0f;
		animT = 0.0f;
		animActive = true;
		follow = false;
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Zoom out to the whole trace.");

	ImGui::SameLine();
	ImGui::Text("%u events%s  |  wheel: zoom  drag: pan  dbl-click: zoom to call",
		(unsigned)events.size(), prof->isFull() ? "  (full)" : "");

	if (selected >= 0 && (uint)selected < zones.size()) {
		const ProfZone &z = zones[selected];
		if (ImGui::SmallButton("Open script"))
			openHandlerScript(prof->internedName(z.nameId));
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Open this handler's script in the Scripts window.");
		ImGui::SameLine();
		uint32 total = z.endTs > z.startTs ? z.endTs - z.startTs : 0;
		uint32 self = total > z.childTs ? total - z.childTs : 0;
		ImGui::Text("selected: %s   frames %u-%u   time %ums (self %ums)   depth %u   movie %s",
			prof->internedName(z.nameId).c_str(), z.startFrame, z.endFrame,
			total, self, z.depth, prof->internedName(z.movieId).c_str());
	} else {
		ImGui::TextUnformatted("selected: (none)   -- click a call to select, double-click to zoom, Open script for its code");
	}

	drawStatistics(prof, zones);

	if (animActive) {
		animT += ImGui::GetIO().DeltaTime * 4.0f;
		float t = animT >= 1.0f ? 1.0f : animT;
		float v = t * t * (3.0f - 2.0f * t);
		viewStart = aFromStart + (aToStart - aFromStart) * v;
		viewSpan = aFromSpan + (aToSpan - aFromSpan) * v;
		if (animT >= 1.0f)
			animActive = false;
	}

	const float rowH = 18.0f;
	const float rulerH = 22.0f;
	ImGui::BeginChild("##timeline", ImVec2(0, 0), ImGuiChildFlags_Borders);

	const float viewW = ImGui::GetContentRegionAvail().x;
	float availH = ImGui::GetContentRegionAvail().y;

	if (viewSpan < 4.0f)
		viewSpan = 4.0f;
	if (follow && !animActive)
		viewStart = (float)maxSeq - viewSpan * 0.5f;

	const ImVec2 origin = ImGui::GetCursorScreenPos();
	float canvasH = (float)(maxRow + 3) * rowH + 20.0f;
	if (canvasH < availH)
		canvasH = availH;

	ImGui::InvisibleButton("##canvas", ImVec2(viewW, canvasH));
	const bool itemHovered = ImGui::IsItemHovered();
	const bool itemActive = ImGui::IsItemActive();
	const ImVec2 mouse = ImGui::GetIO().MousePos;

	float pxPerUnit = viewW / viewSpan;

	// Wheel zooms around the cursor, drag pans (both cancel follow/anim).
	if (itemHovered) {
		float wheel = ImGui::GetIO().MouseWheel;
		if (wheel != 0.0f) {
			float cursorSeq = viewStart + (mouse.x - origin.x) / pxPerUnit;
			float factor = 1.0f - wheel * 0.15f;
			if (factor < 0.2f)
				factor = 0.2f;
			viewSpan *= factor;
			if (viewSpan < 4.0f)
				viewSpan = 4.0f;
			if (viewSpan > (float)(maxSeq + 50))
				viewSpan = (float)(maxSeq + 50);
			pxPerUnit = viewW / viewSpan;
			viewStart = cursorSeq - (mouse.x - origin.x) / pxPerUnit;
			follow = false;
			animActive = false;
		}
	}
	if (itemActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		viewStart -= ImGui::GetIO().MouseDelta.x / pxPerUnit;
		follow = false;
		animActive = false;
	}

	const ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
	const bool clicked = itemHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
		fabsff(dragDelta.x) < 3.0f && fabsff(dragDelta.y) < 3.0f;
	const bool dblClicked = itemHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

	ImDrawList *dl = ImGui::GetWindowDrawList();
	const float baseY = origin.y + rulerH;
	const ImU32 frameLineCol = ImGui::GetColorU32(theme.line_color);
	const ImU32 rulerBgCol = ImGui::GetColorU32(theme.prof_ruler_bg);
	const ImU32 gridLineCol = ImGui::GetColorU32(theme.prof_grid_line);
	const ImU32 freezeCol = ImGui::GetColorU32(theme.prof_freeze);
	const ImU32 thawCol = ImGui::GetColorU32(theme.prof_thaw);
	const ImU32 zoneBorderCol = ImGui::GetColorU32(theme.prof_zone_border);
	const ImU32 zoneTextCol = ImGui::GetColorU32(theme.prof_zone_text);
	const ImU32 selectedCol = ImGui::GetColorU32(theme.prof_selected);
	const ImU32 highlightCol = ImGui::GetColorU32(theme.prof_highlight);
	const ImU32 liveEdgeCol = ImGui::GetColorU32(theme.prof_live_edge);
	const ImU32 crosshairCol = ImGui::GetColorU32(theme.prof_crosshair);

	const float visStartSeq = viewStart - 2.0f;
	const float visEndSeq = viewStart + viewSpan + 2.0f;
	const uint kMaxSlices = 6000;
	uint drawn = 0;
	int hoveredZone = -1;

	Common::Array<float> rowRight;
	rowRight.resize((uint)maxRow + 2);
	for (uint i = 0; i < rowRight.size(); i++)
		rowRight[i] = -1.0e9f;

	// Frames and freeze/thaw live in a thin ruler strip at the top.
	dl->AddRectFilled(ImVec2(origin.x, origin.y), ImVec2(origin.x + viewW, origin.y + rulerH), rulerBgCol);
	dl->AddLine(ImVec2(origin.x, origin.y + rulerH), ImVec2(origin.x + viewW, origin.y + rulerH), frameLineCol, 1.0f);

	float lastFrameLabelX = -1.0e9f;
	float lastFreezeX = -1.0e9f;
	float lastThawX = -1.0e9f;
	for (uint i = 0; i < events.size(); i++) {
		const ProfilerEvent &e = events[i];
		if (e.type != kProfFrame && e.type != kProfFreeze && e.type != kProfThaw)
			continue;
		if ((float)e.seq < visStartSeq || (float)e.seq > visEndSeq)
			continue;
		const float x = origin.x + ((float)e.seq - viewStart) * pxPerUnit;
		if (e.type == kProfFrame) {
			if (x - lastFrameLabelX < 44.0f)
				continue;
			lastFrameLabelX = x;
			dl->AddLine(ImVec2(x, origin.y + rulerH - 6.0f), ImVec2(x, origin.y + rulerH), frameLineCol, 1.0f);
			dl->AddLine(ImVec2(x, origin.y + rulerH), ImVec2(x, origin.y + canvasH), gridLineCol, 1.0f);
			Common::String lbl = Common::String::format("f%u", e.frame);
			dl->AddText(ImVec2(x + 2.0f, origin.y + 3.0f), theme.gridTextColor, lbl.c_str());
		} else if (e.type == kProfFreeze) {
			if (x - lastFreezeX < 4.0f)
				continue;
			lastFreezeX = x;
			dl->AddTriangleFilled(ImVec2(x - 3.0f, origin.y + 2.0f), ImVec2(x + 3.0f, origin.y + 2.0f),
				ImVec2(x, origin.y + 8.0f), freezeCol);
		} else { // kProfThaw
			if (x - lastThawX < 4.0f)
				continue;
			lastThawX = x;
			dl->AddTriangleFilled(ImVec2(x - 3.0f, origin.y + rulerH - 2.0f), ImVec2(x + 3.0f, origin.y + rulerH - 2.0f),
				ImVec2(x, origin.y + rulerH - 8.0f), thawCol);
		}
	}

	// Zones. Cull to the visible window and coalesce sub-pixel slices.
	Common::Array<DrawnRect> drawnRects;
	for (uint i = 0; i < zones.size(); i++) {
		const ProfZone &z = zones[i];
		if ((float)z.endSeq < visStartSeq || (float)z.startSeq > visEndSeq)
			continue;

		float x0 = origin.x + ((float)z.startSeq - viewStart) * pxPerUnit;
		float x1 = origin.x + ((float)z.endSeq - viewStart) * pxPerUnit;
		if (x1 < x0 + 1.0f)
			x1 = x0 + 1.0f;

		const uint rr = z.depth < rowRight.size() ? z.depth : rowRight.size() - 1;
		if (x1 - x0 < 1.5f && x0 < rowRight[rr] + 1.0f)
			continue;
		rowRight[rr] = x1;

		if (drawn >= kMaxSlices)
			break;
		drawn++;

		const float y0 = baseY + (float)z.depth * rowH;
		const float y1 = y0 + rowH - 1.0f;

		dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), nameColor(z.nameId));
		if (x1 - x0 > 3.0f)
			dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), zoneBorderCol);

		if (x1 - x0 > 24.0f) {
			const Common::String &nm = prof->internedName(z.nameId);
			dl->PushClipRect(ImVec2(x0 + 2.0f, y0), ImVec2(x1 - 2.0f, y1), true);
			dl->AddText(ImVec2(x0 + 3.0f, y0 + 2.0f), zoneTextCol, nm.c_str());
			dl->PopClipRect();
		}

		DrawnRect dr;
		dr.x0 = x0; dr.y0 = y0; dr.x1 = x1; dr.y1 = y1; dr.nameId = z.nameId;
		drawnRects.push_back(dr);

		if (itemHovered && mouse.x >= x0 && mouse.x <= x1 && mouse.y >= y0 && mouse.y <= y1)
			hoveredZone = (int)i;

		if ((int)i == selected)
			dl->AddRect(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), selectedCol, 0.0f, 0, 2.0f);
	}

	// Highlight every visible call sharing the hovered call's name.
	if (hoveredZone >= 0 && (uint)hoveredZone < zones.size()) {
		uint32 hn = zones[hoveredZone].nameId;
		for (uint i = 0; i < drawnRects.size(); i++) {
			const DrawnRect &dr = drawnRects[i];
			if (dr.nameId == hn)
				dl->AddRect(ImVec2(dr.x0, dr.y0), ImVec2(dr.x1, dr.y1), highlightCol, 0.0f, 0, 1.5f);
		}
	}

	// Live edge: the newest recorded event ("now").
	{
		const float px = origin.x + ((float)maxSeq - viewStart) * pxPerUnit;
		if (px >= origin.x && px <= origin.x + viewW)
			dl->AddLine(ImVec2(px, origin.y), ImVec2(px, origin.y + canvasH), liveEdgeCol, 2.0f);
	}

	if (itemHovered)
		dl->AddLine(ImVec2(mouse.x, origin.y), ImVec2(mouse.x, origin.y + canvasH), crosshairCol, 1.0f);

	if (zones.empty())
		dl->AddText(ImVec2(origin.x + 12.0f, origin.y + 12.0f), theme.gridTextColor,
			"Tick Capture, then interact with the game to record Lingo execution.");

	if (dblClicked && hoveredZone >= 0 && (uint)hoveredZone < zones.size()) {
		const ProfZone &z = zones[hoveredZone];
		float span = (float)(z.endSeq - z.startSeq);
		if (span < 1.0f)
			span = 1.0f;
		float pad = span * 0.15f + 1.0f;
		aFromStart = viewStart;
		aFromSpan = viewSpan;
		aToStart = (float)z.startSeq - pad;
		aToSpan = span + 2.0f * pad;
		animT = 0.0f;
		animActive = true;
		follow = false;
		selected = hoveredZone;
	} else if (clicked) {
		selected = hoveredZone;
	}

	ImGui::EndChild();

	if (hoveredZone >= 0 && (uint)hoveredZone < zones.size()) {
		const ProfZone &z = zones[hoveredZone];
		ImGui::BeginTooltip();
		ImGui::Text("%s", prof->internedName(z.nameId).c_str());
		ImGui::Text("movie: %s", prof->internedName(z.movieId).c_str());
		if (z.startFrame == z.endFrame)
			ImGui::Text("frame %u", z.startFrame);
		else
			ImGui::Text("frames %u - %u", z.startFrame, z.endFrame);
		uint32 total = z.endTs > z.startTs ? z.endTs - z.startTs : 0;
		uint32 self = total > z.childTs ? total - z.childTs : 0;
		ImGui::Text("time: %ums (self %ums)   depth: %u", total, self, z.depth);
		ImGui::EndTooltip();
	}

	ImGui::End();
}

} // End of namespace DT
} // End of namespace Director
