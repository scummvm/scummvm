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
#include "common/path.h"

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

struct ProfZone {
	uint32 startSeq;
	uint32 endSeq;
	uint32 nameId;
	uint32 movieId;
	uint32 startFrame;
	uint32 endFrame;
	uint16 depth;
};

struct StatRow {
	uint32 nameId;
	uint32 count;
	uint64 totalSpan;
};

struct DrawnRect {
	float x0, y0, x1, y1;
	uint32 nameId;
};

static bool statGreater(const StatRow &a, const StatRow &b) {
	return a.count > b.count;
}

static ImU32 nameColor(uint32 id) {
	uint32 h = (id + 1) * 2654435761u;
	int r = 130 + (int)((h >> 1) & 0x4F);
	int g = 130 + (int)((h >> 9) & 0x4F);
	int b = 130 + (int)((h >> 17) & 0x4F);
	return IM_COL32(r, g, b, 255);
}

static inline float fabsff(float f) { return f < 0.0f ? -f : f; }

void showProfiler() {
	if (!_state->_w.profiler)
		return;

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

	static Common::Array<ProfZone> zones;
	static uint builtCount = 0xFFFFFFFFu;
	static uint16 maxRow = 0;
	static uint32 maxSeq = 0;
	static float viewStart = 0.0f;
	static float viewSpan = 200.0f;
	static bool follow = true;
	static int selected = -1;

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
	static Common::String status;
	if (ImGui::Button("Export JSON")) {
		Common::Path path("lingo-trace.json");
		status = prof->exportChromeTrace(path) ? Common::String("wrote lingo-trace.json") : Common::String("export failed");
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Write a Chrome/Perfetto trace to lingo-trace.json.");

	ImGui::SameLine();
	ImGui::Checkbox("Follow live", &follow);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Keep the newest event (red line) at the right edge.\nAny zoom/pan turns this off.");

	const Common::Array<ProfilerEvent> &events = prof->events();
	if (builtCount != events.size()) {
		builtCount = events.size();
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
					zones[idx].endFrame = e.frame;
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
		for (uint i = 0; i < openStack.size(); i++)
			zones[openStack[i]].endSeq = maxSeq + 1;

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
		ImGui::Text("selected: %s   frames %u-%u   span %u   depth %u   movie %s",
			prof->internedName(z.nameId).c_str(), z.startFrame, z.endFrame,
			z.endSeq - z.startSeq, z.depth, prof->internedName(z.movieId).c_str());
	} else {
		ImGui::TextUnformatted("selected: (none)   -- click a call to select, double-click to zoom, Open script for its code");
	}

	if (ImGui::CollapsingHeader("Statistics")) {
		Common::HashMap<uint32, uint> idxByName;
		Common::Array<StatRow> rows;
		for (uint i = 0; i < zones.size(); i++) {
			const ProfZone &z = zones[i];
			uint32 span = z.endSeq > z.startSeq ? z.endSeq - z.startSeq : 0;
			Common::HashMap<uint32, uint>::iterator it = idxByName.find(z.nameId);
			if (it == idxByName.end()) {
				StatRow r;
				r.nameId = z.nameId;
				r.count = 1;
				r.totalSpan = span;
				idxByName[z.nameId] = rows.size();
				rows.push_back(r);
			} else {
				rows[it->_value].count++;
				rows[it->_value].totalSpan += span;
			}
		}
		Common::sort(rows.begin(), rows.end(), statGreater);

		ImGui::BeginChild("##stats", ImVec2(0, 160.0f), ImGuiChildFlags_Borders);
		ImGui::Text("%8s  %10s  %s", "calls", "total(seq)", "handler (click to open)");
		uint shown = rows.size() < 50 ? rows.size() : 50;
		for (uint i = 0; i < shown; i++) {
			Common::String row = Common::String::format("%8u  %10.0f  %s", rows[i].count,
				(double)rows[i].totalSpan, prof->internedName(rows[i].nameId).c_str());
			ImGui::PushID((int)i);
			if (ImGui::Selectable(row.c_str()))
				openHandlerScript(prof->internedName(rows[i].nameId));
			ImGui::PopID();
		}
		ImGui::EndChild();
	}

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
	const ImU32 frameLineCol = ImGui::GetColorU32(_state->theme->line_color);

	const float visStartSeq = viewStart - 2.0f;
	const float visEndSeq = viewStart + viewSpan + 2.0f;
	const uint kMaxSlices = 6000;
	uint drawn = 0;
	int hoveredZone = -1;

	Common::Array<float> rowRight;
	rowRight.resize((uint)maxRow + 2);
	for (uint i = 0; i < rowRight.size(); i++)
		rowRight[i] = -1.0e9f;

	dl->AddRectFilled(ImVec2(origin.x, origin.y), ImVec2(origin.x + viewW, origin.y + rulerH), IM_COL32(0, 0, 0, 40));
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
			dl->AddLine(ImVec2(x, origin.y + rulerH), ImVec2(x, origin.y + canvasH), IM_COL32(140, 140, 140, 22), 1.0f);
			Common::String lbl = Common::String::format("f%u", e.frame);
			dl->AddText(ImVec2(x + 2.0f, origin.y + 3.0f), _state->theme->gridTextColor, lbl.c_str());
		} else if (e.type == kProfFreeze) {
			if (x - lastFreezeX < 4.0f)
				continue;
			lastFreezeX = x;
			dl->AddTriangleFilled(ImVec2(x - 3.0f, origin.y + 2.0f), ImVec2(x + 3.0f, origin.y + 2.0f),
				ImVec2(x, origin.y + 8.0f), IM_COL32(230, 120, 60, 255));
		} else {
			if (x - lastThawX < 4.0f)
				continue;
			lastThawX = x;
			dl->AddTriangleFilled(ImVec2(x - 3.0f, origin.y + rulerH - 2.0f), ImVec2(x + 3.0f, origin.y + rulerH - 2.0f),
				ImVec2(x, origin.y + rulerH - 8.0f), IM_COL32(90, 200, 120, 255));
		}
	}

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
			dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(0, 0, 0, 100));

		if (x1 - x0 > 24.0f) {
			const Common::String &nm = prof->internedName(z.nameId);
			dl->PushClipRect(ImVec2(x0 + 2.0f, y0), ImVec2(x1 - 2.0f, y1), true);
			dl->AddText(ImVec2(x0 + 3.0f, y0 + 2.0f), IM_COL32(20, 20, 20, 255), nm.c_str());
			dl->PopClipRect();
		}

		DrawnRect dr;
		dr.x0 = x0; dr.y0 = y0; dr.x1 = x1; dr.y1 = y1; dr.nameId = z.nameId;
		drawnRects.push_back(dr);

		if (itemHovered && mouse.x >= x0 && mouse.x <= x1 && mouse.y >= y0 && mouse.y <= y1)
			hoveredZone = (int)i;

		if ((int)i == selected)
			dl->AddRect(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), IM_COL32(255, 220, 40, 255), 0.0f, 0, 2.0f);
	}

	if (hoveredZone >= 0 && (uint)hoveredZone < zones.size()) {
		uint32 hn = zones[hoveredZone].nameId;
		for (uint i = 0; i < drawnRects.size(); i++) {
			const DrawnRect &dr = drawnRects[i];
			if (dr.nameId == hn)
				dl->AddRect(ImVec2(dr.x0, dr.y0), ImVec2(dr.x1, dr.y1), IM_COL32(255, 255, 180, 200), 0.0f, 0, 1.5f);
		}
	}

	{
		const float px = origin.x + ((float)maxSeq - viewStart) * pxPerUnit;
		if (px >= origin.x && px <= origin.x + viewW)
			dl->AddLine(ImVec2(px, origin.y), ImVec2(px, origin.y + canvasH), IM_COL32(230, 40, 40, 255), 2.0f);
	}

	if (itemHovered)
		dl->AddLine(ImVec2(mouse.x, origin.y), ImVec2(mouse.x, origin.y + canvasH), IM_COL32(150, 150, 150, 90), 1.0f);

	if (zones.empty())
		dl->AddText(ImVec2(origin.x + 12.0f, origin.y + 12.0f), _state->theme->gridTextColor,
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
		ImGui::Text("span: %u   depth: %u", z.endSeq - z.startSeq, z.depth);
		ImGui::EndTooltip();
	}

	ImGui::End();
}

} // End of namespace DT
} // End of namespace Director
