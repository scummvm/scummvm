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

#ifndef TSAGE_RINGWORLD2_AIRDUCT_H
#define TSAGE_RINGWORLD2_AIRDUCT_H

#include "tsage/tsage.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class Scene1200 : public SceneExt {
	enum CrawlDirection { CRAWL_EAST = 1, CRAWL_WEST = 2, CRAWL_SOUTH = 3, CRAWL_NORTH = 4 };

	class LaserPanel: public ModalWindow {
	public:
		class Jumper : public SceneActorExt {
		public:
			void init(int state);
			bool startAction(CursorType action, Event &event) override;
		};

		Jumper _jumper1;
		Jumper _jumper2;
		Jumper _jumper3;

		LaserPanel();

		void postInit(SceneObjectList *OwnerList = NULL) override;
		void remove() override;
	};

public:
	NamedHotspot _item1;
	SceneActor _actor1;
	LaserPanel _laserPanel;
	MazeUI _mazeUI;
	SequenceManager _sequenceManager;

	int _nextCrawlDirection;
	int _field414;
	int _field416;
	int _field418;
	int _field41A;
	bool _fixupMaze;

	Scene1200();
	void synchronize(Serializer &s) override;

	void startCrawling(CrawlDirection dir);

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
	void saveCharacter(int characterIndex) override;
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
