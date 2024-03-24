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

#include "twp/twp.h"
#include "twp/object.h"
#include "twp/room.h"
#include "twp/lighting.h"
#include "twp/walkboxnode.h"

namespace Twp {

WalkboxNode::WalkboxNode() : Node("Walkbox") {
	_zOrder = -1000;
	_mode = WalkboxMode::None;
}

void WalkboxNode::drawCore(const Math::Matrix4& trsf) {
	if (g_twp->_room) {
		Color white;
		Color red(1.f, 0.f, 0.f);
		Color green(0.f, 1.f, 0.f);
		Color yellow(1.f, 1.f, 0.f);
		Common::Array<Walkbox> walkboxes = g_twp->_room ? g_twp->_room->_pathFinder.getWalkboxes() : Common::Array<Walkbox>();

		switch (_mode) {
		case WalkboxMode::All: {
			Math::Matrix4 transf;
			// cancel camera pos
			Math::Vector2d pos = g_twp->getGfx().cameraPos();
			transf.translate(Math::Vector3d(-pos.getX(), -pos.getY(), 0.f));
			for (uint i = 0; i < walkboxes.size(); i++) {
				const Walkbox &wb = walkboxes[i];
				const Color color = wb.isVisible() ? green : red;
				Common::Array<Vertex> vertices;
				const Common::Array<Vector2i> &points = wb.getPoints();
				for (uint j = 0; j < points.size(); j++) {
					Vector2i pInt = points[j];
					Math::Vector2d p = (Math::Vector2d)pInt;
					vertices.push_back(Vertex((Math::Vector2d)p, color));

					Math::Matrix4 t(transf);
					p -= Math::Vector2d(1.f, 1.f);
					t.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
				}
				g_twp->getGfx().drawLinesLoop(vertices.data(), vertices.size(), transf);
			}
		} break;
		case WalkboxMode::Merged: {
			Math::Matrix4 transf;
			Math::Vector2d pos = g_twp->getGfx().cameraPos();
			// cancel camera pos
			transf.translate(Math::Vector3d(-pos.getX(), -pos.getY(), 0.f));
			for (uint i = 0; i < walkboxes.size(); i++) {
				const Walkbox &wb = walkboxes[i];
				const Color color = i == 0 ? green : red;
				Common::Array<Vertex> vertices;
				const Common::Array<Vector2i> &points = wb.getPoints();
				for (uint j = 0; j < points.size(); j++) {
					Vector2i pInt = points[j];
					Math::Vector2d p = (Math::Vector2d)pInt;
					vertices.push_back(Vertex(p, color));

					Math::Matrix4 t(transf);
					p -= Math::Vector2d(1.f, 1.f);
					t.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
					// if (wb.concave(j) == (i == 0))
					// 	g_twp->getGfx().drawQuad(Math::Vector2d(3.f, 3.f), yellow, t);
				}
				g_twp->getGfx().drawLinesLoop(vertices.data(), vertices.size(), transf);
			}
		} break;
		default:
			break;
		}
	}
}

PathNode::PathNode() : Node("Path") {
	_zOrder = -1000;
}

Math::Vector2d PathNode::fixPos(const Math::Vector2d &pos) {
	for (size_t i = 0; i < g_twp->_room->_mergedPolygon.size(); i++) {
		Walkbox &wb = g_twp->_room->_mergedPolygon[i];
		if (!wb.isVisible() && wb.contains(pos)) {
			return wb.getClosestPointOnEdge(pos);
		}
	}
	//   for wb in gEngine.room.mergedPolygon:
	//     if wb.visible and not wb.contains(pos):
	//       return wb.getClosestPointOnEdge(pos)
	return pos;
}

void PathNode::drawCore(const Math::Matrix4& trsf) {
	if (!g_twp->_room)
		return;

	const Color green(0.f, 1.f, 0.f);
	const Color red(1.f, 0.f, 0.f);
	const Color yellow(1.f, 1.f, 0.f);
	const Color blue(0.f, 0.f, 1.f);
	const Common::SharedPtr<Object> actor = g_twp->_actor;

	// draw actor path
	if (((_mode == PathMode::GraphMode) || (_mode == PathMode::All)) && actor && actor->getWalkTo()) {
		const WalkTo *walkTo = (WalkTo *)actor->getWalkTo().get();
		const Common::Array<Math::Vector2d> &path = walkTo->getPath();
		if (path.size() > 0) {
			Common::Array<Vertex> vertices;
			vertices.push_back(Vertex(g_twp->roomToScreen(actor->_node->getPos()), yellow));
			for (uint i = 0; i < path.size(); i++) {
				Math::Vector2d p = g_twp->roomToScreen((Math::Vector2d)path[i]);
				vertices.push_back(Vertex(p, yellow));

				Math::Matrix4 t;
				p -= Math::Vector2d(2.f, 2.f);
				t.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
				g_twp->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), yellow, t);
			}
			g_twp->getGfx().drawLines(vertices.data(), vertices.size());
		}
	}

	// draw graph nodes
	const Twp::Graph &graph = g_twp->_room->_pathFinder.getGraph();
	if (((_mode == PathMode::GraphMode) || (_mode == PathMode::All))) {
		for (uint i = 0; i < graph._concaveVertices.size(); i++) {
			const Math::Vector2d p = g_twp->roomToScreen((Math::Vector2d)graph._concaveVertices[i]) - Math::Vector2d(2.f, 2.f);
			Math::Matrix4 t;
			t.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
			g_twp->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), yellow, t);
		}

		if (_mode == PathMode::All) {
			for (uint i = 0; i < graph._edges.size(); i++) {
				const Common::Array<GraphEdge> &edges = graph._edges[i];
				for (uint j = 0; j < edges.size(); j++) {
					const GraphEdge &edge = edges[j];
					const Math::Vector2d p1 = g_twp->roomToScreen((Math::Vector2d)graph._nodes[edge.start]);
					const Math::Vector2d p2 = g_twp->roomToScreen((Math::Vector2d)graph._nodes[edge.to]);
					Vertex vertices[] = {Vertex(p1), Vertex(p2)};
					g_twp->getGfx().drawLines(&vertices[0], 2);
				}
			}
		}
	}

	// draw path from actor to mouse position
	if (((_mode == PathMode::GraphMode) || (_mode == PathMode::All)) && actor) {
		Math::Vector2d pos = g_twp->roomToScreen(actor->_node->getPos()) - Math::Vector2d(2.f, 2.f);
		Math::Matrix4 t;
		t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
		g_twp->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), yellow, t);

		const Math::Vector2d scrPos = g_twp->winToScreen(g_twp->_cursor.pos);
		const Math::Vector2d roomPos = g_twp->screenToRoom(scrPos);
		Math::Vector2d p = fixPos(roomPos);
		t = Math::Matrix4();
		pos = g_twp->roomToScreen((Math::Vector2d)p) - Math::Vector2d(4.f, 4.f);
		t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
		g_twp->getGfx().drawQuad(Math::Vector2d(8.f, 8.f), yellow, t);

		Common::SharedPtr<Object> obj = g_twp->objAt(roomPos);
		if (obj) {
			t = Math::Matrix4();
			pos = g_twp->roomToScreen(obj->getUsePos()) - Math::Vector2d(4.f, 4.f);
			t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
			g_twp->getGfx().drawQuad(Math::Vector2d(8.f, 8.f), red, t);
		}

		const Common::Array<Math::Vector2d> path = g_twp->_room->calculatePath(fixPos(actor->_node->getPos()), p);
		Common::Array<Vertex> vertices;
		for (uint i = 0; i < path.size(); i++) {
			vertices.push_back(Vertex(g_twp->roomToScreen((Math::Vector2d)path[i]), yellow));
		}
		if (vertices.size() > 0) {
			g_twp->getGfx().drawLines(vertices.data(), vertices.size());
		}

		// draw a green square if inside walkbox, red if not
		Common::Array<Walkbox> walkboxes = g_twp->_room ? g_twp->_room->_pathFinder.getWalkboxes() : Common::Array<Walkbox>();
		if (walkboxes.empty())
			return;

		const bool inside = (walkboxes.size() > 0) && walkboxes[0].contains(roomPos);
		pos = scrPos - Math::Vector2d(4.f, 4.f);
		t = Math::Matrix4();
		t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
		g_twp->getGfx().drawQuad(Math::Vector2d(8.f, 8.f), inside ? green : red, t);

		// draw a blue square on the closest point
		pos = g_twp->roomToScreen((Math::Vector2d)walkboxes[0].getClosestPointOnEdge(roomPos));
		t = Math::Matrix4();
		t.translate(Math::Vector3d(pos.getX() - 2.f, pos.getY() - 2.f, 0.f));
		g_twp->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), blue, t);
	}
}

LightingNode::LightingNode() : Node("Lighting") {
	_visible = false;
}

void LightingNode::drawCore(const Math::Matrix4& trsf) {
	if (!g_twp->_room)
		return;

	const float size = 6.0f;
	for (int i = 0; i < MAX_LIGHTS; i++) {
		float *pos = &g_twp->_lighting->u_lightPos[i * 3];
		float *color = &g_twp->_lighting->u_lightColor[i * 3];
		Math::Vector2d p = g_twp->roomToScreen(Math::Vector2d(pos[0], pos[1]));
		Math::Matrix4 t;
		t.translate(Math::Vector3d(p.getX() - size / 2.f, p.getY() - size / 2.f, 0.f));
		g_twp->getGfx().drawQuad(Math::Vector2d(size, size), Color(color[0], color[1], color[2]), t);
	}
}

} // namespace Twp
