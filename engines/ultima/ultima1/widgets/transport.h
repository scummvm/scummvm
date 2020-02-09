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
	uint getTileNum() const override;
};

/**
 * Base class for the different types of transports
 */
class Transport : public OverworldWidget {
protected:
	uint _transportId;
public:
	/**
	 * Constructor
	 */
	Transport(Ultima1Game *game, Maps::MapBase *map, uint transportId);

	/**
	 * Returns true if the player can move onto a tile the widget occupies
	 */
	bool isBlocking() const override { return false; }

	/**
	 * Board a transport
	 */
	virtual void board();

	/**
	 * Disembarks from the transport
	 */
	virtual void disembark();

	/**
	 * Get the name of a transport's weapons
	 */
	virtual Common::String getWeaponsName() { return ""; }

	/**
	 * Get an Id for the transport type
	 */
	uint transportId() const { return _transportId; }
};

/**
 * Horse widget
 */
class Horse : public Transport {
	DECLARE_MESSAGE_MAP;
public:
	DECLARE_WIDGET(Horse);
	CLASSDEF;

	/**
	 * Constructor
	 */
	Horse(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Get the tile for the transport method
	 */
	uint getTileNum() const override { return 9; }
};

/**
 * Cart widget
 */
class Cart : public Transport {
	DECLARE_MESSAGE_MAP;
public:
	DECLARE_WIDGET(Cart);
	CLASSDEF;

	/**
	 * Constructor
	 */
	Cart(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Get the tile for the transport method
	 */
	uint getTileNum() const override { return 10; }
};

/**
 * Raft widget
 */
class Raft : public Transport {
	DECLARE_MESSAGE_MAP;
public:
	DECLARE_WIDGET(Raft);
	CLASSDEF;

	/**
	 * Constructor
	 */
	Raft(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Get the tile for the transport method
	 */
	uint getTileNum() const override { return 11; }
};

/**
 * Frigate widget
 */
class Frigate : public Transport {
	DECLARE_MESSAGE_MAP;
public:
	DECLARE_WIDGET(Frigate);
	CLASSDEF;

	/**
	 * Constructor
	 */
	Frigate(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Get the name of a transport's weapons
	 */
	Common::String getWeaponsName() override;

	/**
	 * Get the tile for the transport method
	 */
	uint getTileNum() const override { return 12; }
};

/**
 * Aircar widget
 */
class Aircar : public Transport {
	DECLARE_MESSAGE_MAP;
public:
	DECLARE_WIDGET(Aircar);
	CLASSDEF;

	/**
	 * Constructor
	 */
	Aircar(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Get the name of a transport's weapons
	 */
	Common::String getWeaponsName() override;

	/**
	 * Get the tile for the transport method
	 */
	uint getTileNum() const override { return 14; }
};

/**
 * Shuttle widget
 */
class Shuttle : public Transport {
	DECLARE_MESSAGE_MAP;
public:
	uint _space1, _space2;
public:
	DECLARE_WIDGET(Shuttle);
	CLASSDEF;

	/**
	 * Constructor
	 */
	Shuttle(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Handles loading and saving data
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Get the tile for the transport method
	 */
	uint getTileNum() const override { return 15; }
};

/**
 * Time machine widget
 */
class TimeMachine : public Transport {
	DECLARE_MESSAGE_MAP;
public:
	DECLARE_WIDGET(TimeMachine);
	CLASSDEF;

	/**
	 * Constructor
	 */
	TimeMachine(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Get the tile for the transport method
	 */
	uint getTileNum() const override { return 16; }

	/**
	 * Board a transport
	 */
	void board() override;
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
