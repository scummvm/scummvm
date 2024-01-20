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
#include "twp/walkboxnode.h"

namespace Twp {

WalkboxNode::WalkboxNode() : Node("Walkbox") {
	_zOrder = -1000;
	_mode = WalkboxMode::None;
}

void WalkboxNode::drawCore(Math::Matrix4 trsf) {
	if (g_engine->_room) {
		Color white;
		Color red(1.f, 0.f, 0.f);
		Color green(0.f, 1.f, 0.f);
		Color yellow(1.f, 1.f, 0.f);
		switch (_mode) {
		case WalkboxMode::All: {
			Math::Matrix4 transf;
			// cancel camera pos
			Math::Vector2d pos = g_engine->getGfx().cameraPos();
			transf.translate(Math::Vector3d(-pos.getX(), pos.getY(), 0.f));
			for (uint i = 0; i < g_engine->_room->_walkboxes.size(); i++) {
				Walkbox &wb = g_engine->_room->_walkboxes[i];
				Color color = wb.isVisible() ? green : red;
				Common::Array<Vertex> vertices;
				for (uint j = 0; j < wb.getPoints().size(); j++) {
					Math::Vector2d p = wb.getPoints()[j];
					vertices.push_back(Vertex(p, color));

					Color vertexColor = wb.concave(j) ? white : yellow;
					Math::Matrix4 t(transf);
					p -= Math::Vector2d(2.f, 2.f);
					t.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
					g_engine->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), vertexColor, t);
				}
				g_engine->getGfx().drawLinesLoop(&vertices[0], vertices.size(), transf);
			}
		} break;
		case WalkboxMode::Merged: {
			Math::Matrix4 transf;
			Math::Vector2d pos = g_engine->getGfx().cameraPos();
			// cancel camera pos
			transf.translate(Math::Vector3d(-pos.getX(), pos.getY(), 0.f));
			for (uint i = 0; i < g_engine->_room->_mergedPolygon.size(); i++) {
				Walkbox &wb = g_engine->_room->_mergedPolygon[i];
				Color color = wb.isVisible() ? green : red;
				Common::Array<Vertex> vertices;
				for (uint j = 0; j < wb.getPoints().size(); j++) {
					Math::Vector2d p = wb.getPoints()[j];
					vertices.push_back(Vertex(p, color));

					Color vertexColor = wb.concave(j) ? white : yellow;
					Math::Matrix4 t(transf);
					p -= Math::Vector2d(2.f, 2.f);
					t.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
					g_engine->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), vertexColor, t);
				}
				g_engine->getGfx().drawLinesLoop(&vertices[0], vertices.size(), transf);
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

Math::Vector2d PathNode::fixPos(Math::Vector2d pos) {
	for (size_t i = 0; i < g_engine->_room->_mergedPolygon.size(); i++) {
		Walkbox &wb = g_engine->_room->_mergedPolygon[i];
		if (!wb.isVisible() && wb.contains(pos)) {
			return wb.getClosestPointOnEdge(pos);
		}
	}
	//   for wb in gEngine.room.mergedPolygon:
	//     if wb.visible and not wb.contains(pos):
	//       return wb.getClosestPointOnEdge(pos)
	return pos;
}

void PathNode::drawCore(Math::Matrix4 trsf) {
	Color red(1.f, 0.f, 0.f);
	Color yellow(1.f, 1.f, 0.f);
	Color blue(0.f, 0.f, 1.f);
	Object *actor = g_engine->_actor;
	// draw actor path
	if (((_mode == PathMode::GraphMode) || (_mode == PathMode::All)) && actor && actor->getWalkTo()) {
		WalkTo *walkTo = (WalkTo *)actor->getWalkTo();
		const Common::Array<Math::Vector2d> &path = walkTo->getPath();
		if (path.size() > 0) {
			Common::Array<Vertex> vertices;
			vertices.push_back(Vertex(g_engine->roomToScreen(actor->_node->getPos()), yellow));
			for (uint i = 0; i < path.size(); i++) {
				Math::Vector2d p = g_engine->roomToScreen(path[i]);
				vertices.push_back(Vertex(p, yellow));

				Math::Matrix4 t;
				p -= Math::Vector2d(2.f, 2.f);
				t.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
				g_engine->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), yellow, t);
			}
			g_engine->getGfx().drawLines(&vertices[0], vertices.size());
		}
	}

	// draw graph nodes
	const Twp::Graph *graph = g_engine->_room->_pathFinder.getGraph();
	if (((_mode == PathMode::GraphMode) || (_mode == PathMode::All)) && graph) {
		for (uint i = 0; i < graph->_concaveVertices.size(); i++) {
			Math::Vector2d v = graph->_concaveVertices[i];
			Math::Matrix4 t;
			Math::Vector2d p = g_engine->roomToScreen(v) - Math::Vector2d(2.f, 2.f);
			t.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
			g_engine->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), yellow);
		}

		if (_mode == PathMode::All) {
			for (uint i = 0; i < graph->_edges.size(); i++) {
				const Common::Array<GraphEdge> &edges = graph->_edges[i];
				for (uint j = 0; j < edges.size(); j++) {
					const GraphEdge &edge = edges[j];
					Math::Vector2d p1 = g_engine->roomToScreen(graph->_nodes[edge.start]);
					Math::Vector2d p2 = g_engine->roomToScreen(graph->_nodes[edge.to]);
					Vertex vertices[] = {Vertex(p1), Vertex(p2)};
					g_engine->getGfx().drawLines(&vertices[0], 2);
				}
			}
		}
	}

	// draw path from actor to mouse position
	if (((_mode == PathMode::GraphMode) || (_mode == PathMode::All)) && actor) {
		Math::Vector2d pos = g_engine->roomToScreen(actor->_node->getPos()) - Math::Vector2d(2.f, 2.f);
		Math::Matrix4 t;
		t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
		g_engine->getGfx().drawQuad(Math::Vector2d(4.f, 4.f), yellow, t);

		Math::Vector2d scrPos = g_engine->winToScreen(g_engine->_cursor.pos);
		Math::Vector2d roomPos = g_engine->screenToRoom(scrPos);
		Math::Vector2d p = fixPos(roomPos);
		t = Math::Matrix4();
		pos = g_engine->roomToScreen(p) - Math::Vector2d(4.f, 4.f);
		t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
		g_engine->getGfx().drawQuad(Math::Vector2d(8.f, 8.f), yellow, t);

		Object* obj = g_engine->objAt(roomPos);
		if(obj) {
			t = Math::Matrix4();
			pos = g_engine->roomToScreen(obj->getUsePos()) - Math::Vector2d(4.f, 4.f);
			t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
			g_engine->getGfx().drawQuad(Math::Vector2d(8.f, 8.f), red, t);
		}

		Common::Array<Math::Vector2d> path = g_engine->_room->calculatePath(fixPos(actor->_node->getPos()), p);
		Common::Array<Vertex> vertices;
		for (uint i = 0; i < path.size(); i++) {
			vertices.push_back(Vertex(g_engine->roomToScreen(path[i]), yellow));
		}
		if (vertices.size() > 0) {
			g_engine->getGfx().drawLines(&vertices[0], vertices.size());
		}
	}
}

} // namespace Twp
