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

#ifndef ULTIMA_ULTIMA1_WIDGETS_TRANSPORT_H
#define ULTIMA_ULTIMA1_WIDGETS_TRANSPORT_H

#include "ultima/ultima1/widgets/overworld_widget.h"
#include "ultima/shared/maps/map_base.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

/**
 * Widget for the player moving within the overworld on foot
 */
class TransportOnFoot : public OverworldWidget {
public:
	DECLARE_WIDGET(TransportOnFoot)

	/**
	 * Constructor
	 */
	TransportOnFoot(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Get the tile for the transport method
	 */
	virtual uint getTileNum() const override;
};

/**
 * Base class for the different types of transports
 */
class Transport : public OverworldWidget {
public:
	/**
	 * Constructor
	 */
	Transport(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Disembarks from the transport
	 */
	virtual void disembark();

	/**
	 * Fire a transport's weapons
	 */
	virtual bool fire() { return false; }
};

/**
 * Horse widget
 */
class Horse : public Transport {
public:
	/**
	 * Constructor
	 */
	Horse(Ultima1Game *game, Maps::MapBase *map);
};

/**
 * Cart widget
 */
class Cart : public Transport {
public:
	/**
	 * Constructor
	 */
	Cart(Ultima1Game *game, Maps::MapBase *map);
};

/**
 * Raft widget
 */
class Raft : public Transport {
public:
	/**
	 * Constructor
	 */
	Raft(Ultima1Game *game, Maps::MapBase *map);
};

/**
 * Frigate widget
 */
class Frigate : public Transport {
public:
	/**
	 * Constructor
	 */
	Frigate(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Fire a transport's weapons
	 */
	virtual bool fire();
};

/**
 * Aircar widget
 */
class Aircar : public Transport {
public:
	/**
	 * Constructor
	 */
	Aircar(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Fire a transport's weapons
	 */
	virtual bool fire();
};

/**
 * Shuttle widget
 */
class Shuttle : public Transport {
public:
	/**
	 * Constructor
	 */
	Shuttle(Ultima1Game *game, Maps::MapBase *map);
};

/**
 * Time machine widget
 */
class TimeMachine : public Transport {
public:
	/**
	 * Constructor
	 */
	TimeMachine(Ultima1Game *game, Maps::MapBase *map);
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
