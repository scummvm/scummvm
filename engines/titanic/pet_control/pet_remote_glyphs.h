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

#ifndef TITANIC_PET_REMOTE_GLYPHS_H
#define TITANIC_PET_REMOTE_GLYPHS_H

#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/pet_control/pet_gfx_element.h"

namespace Titanic {

enum RemoteGlyph {
	GLYPH_SUMMON_ELEVATOR = 0, GLYPH_SUMMON_PELLERATOR = 1,
	GLYPH_TELEVISION_CONTROL = 2, GLYPH_ENTERTAINMENT_DEVICE = 3,
	GLYPH_OPERATE_LIGHTS = 4, GLYPH_DEPLOY_FLORAL = 5,
	GLYPH_DEPLOY_FULLY_RELAXATION = 6, GLYPH_DEPLOY_COMFORT = 7,
	GLYPH_DEPLOY_MINOR_STORAGE = 8, GLYPH_DEPLOY_MAJOR_RELAXATION = 9,
	GLYPH_INFLATE_RELAXATION = 10, GLYPH_DEPLOY_MAINTENANCE = 11,
	GLYPH_DEPLOY_WORK_SURFACE = 12, GLYPH_DEPLOY_MINOR_RELAXATION = 13,
	GLYPH_DEPLOY_SINK = 14, GLYPH_DEPLOY_MAJOR_STORAGE = 15,
	GLYPH_SUCCUBUS_DELIVERY = 16, GLYPH_NAVIGATION_CONTROLLER = 17,
	GLYPH_SKIP_NAVIGATION = 18, GLYPH_GOTO_BOTTOM_OF_WELL = 19,
	GLYPH_GOTO_TOP_OF_WELL = 20, GLYPH_GOTO_STATEROOM = 21,
	GLYPH_GOTO_BAR = 22, GLYPH_GOTO_PROMENADE = 23, GLYPH_GOTO_ARBORETUM = 24,
	GLYPH_GOTO_MUSIC_ROOM = 25, GLYPH_GOTO_RESTAURANT = 26
};

enum RemoteMessage {
	RMSG_LEFT = 0, RMSG_RIGHT = 1, RMSG_UP = 2, RMSG_DOWN = 3, RMSG_ACTIVATE = 4
};

class CPetRemote;

class CPetRemoteGlyphs : public CPetGlyphs {
public:
	/**
	 * Returns the owning CPetRemote
	 */
	CPetRemote *getOwner() const;

	/**
	 * Generates a PET message
	 */
	void generateMessage(RemoteMessage msgNum, const CString &name, int num = -1);
};

class CPetRemoteGlyph : public CPetGlyph {
protected:
	CPetGfxElement *_callButton;
protected:
	CPetRemoteGlyph() : CPetGlyph(), _callButton(nullptr) {}

	/**
	 * Set defaults for the glyph
	 */
	void setDefaults(const CString &name, CPetControl *petControl);

	/**
	 * Get the owner
	 */
	CPetRemoteGlyphs *getOwner() const;

	/**
	 * Get an element by id from the parent Remote section
	 */
	CPetGfxElement *getElement(uint id) const;
};

class CBasicRemoteGlyph : public CPetRemoteGlyph {
private:
	CString _gfxName, _tooltip, _msgString;
public:
	CBasicRemoteGlyph(const CString &gfxName, const CString &tooltip,
		const CString &msgString) : CPetRemoteGlyph(),
		_gfxName(gfxName), _tooltip(tooltip), _msgString(msgString) {}

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CToggleRemoteGlyph : public CPetRemoteGlyph {
protected:
	CPetGfxElement *_toggle;
	bool _toggleFlag;
public:
	CToggleRemoteGlyph() : CPetRemoteGlyph(), _toggle(nullptr), _toggleFlag(false) {}

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages to the default element
	 */
	bool elementMouseButtonDownMsg(const Point &pt, int petNum);

	/**
	 * Called for mouse button up messages to the default element
	 */
	bool elementMouseButtonUpMsg(const Point &pt, int petNum);
};

class CRemoteGotoGlyph : public CPetRemoteGlyph {
protected:
	int _roomIndex;
	CPetGfxElement *_goButton;
	CString _gfxName, _tooltip;
public:
	CRemoteGotoGlyph() : CPetRemoteGlyph(), _goButton(nullptr), _roomIndex(21) {}
	CRemoteGotoGlyph(const CString &gfxName, const CString &tooltip, int roomIndex) :
		CPetRemoteGlyph(), _gfxName(gfxName), _tooltip(tooltip), _roomIndex(roomIndex),
		_goButton(nullptr) {}

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CSummonElevatorGlyph : public CBasicRemoteGlyph {
public:
	CSummonElevatorGlyph();
};

class CSummonPelleratorGlyph : public CBasicRemoteGlyph {
public:
	CSummonPelleratorGlyph();
};

class CTelevisionControlGlyph : public CPetRemoteGlyph {
private:
	bool _flag;
	CPetGfxElement *_up, *_down, *_onOff;
public:
	CTelevisionControlGlyph() : CPetRemoteGlyph(), _flag(false),
		_up(nullptr), _down(nullptr), _onOff(nullptr) {}

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CEntertainmentDeviceGlyph : public CToggleRemoteGlyph {
public:
	bool _flag2;
	CPetGfxElement *_up, *_down;
public:
	CEntertainmentDeviceGlyph() : CToggleRemoteGlyph(),
		_flag2(false), _up(nullptr), _down(nullptr) {}

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};


class COperateLightsGlyph : public CPetRemoteGlyph {
public:
	CPetGfxElement *_left, *_right, *_up, *_down, *_activate;
public:
	COperateLightsGlyph() : CPetRemoteGlyph(), _left(nullptr), _right(nullptr),
		_up(nullptr), _down(nullptr), _activate(nullptr) {}

	/**
	* Setup the glyph
	*/
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	* Handles any secondary drawing of the glyph
	*/
	void draw2(CScreenManager *screenManager) override;

	/**
	* Called for mouse button down messages
	*/
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	* Handles mouse button up messages
	*/
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	* Returns the tooltip text for when the glyph is selected
	*/
	void getTooltip(CTextControl *text) override;
};

class CDeployFloralGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 0);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 0);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CDeployFullyRelaxationGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 1);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 1);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CDeployComfortGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 2);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 2);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CDeployMinorStorageGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 3);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 3);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CDeployMajorRelaxationGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 5);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 5);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CInflateRelaxationGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 6);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 6);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CDeployMaintenanceGlyph : public CToggleRemoteGlyph {
public:
	/**
	* Setup the glyph
	*/
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	* Called for mouse button down messages
	*/
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 7);
	}

	/**
	* Handles mouse button up messages
	*/
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 7);
	}

	/**
	* Returns the tooltip text for when the glyph is selected
	*/
	void getTooltip(CTextControl *text) override;
};

class CDeployWorkSurfaceGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 8);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 8);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CDeployMinorRelaxationGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 9);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 9);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CDeploySinkGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 10);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 10);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CDeployMajorStorageGlyph : public CToggleRemoteGlyph {
public:
	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override {
		return elementMouseButtonDownMsg(pt, 11);
	}

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override {
		return elementMouseButtonUpMsg(pt, 11);
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CSuccubusDeliveryGlyph : public CPetRemoteGlyph {
private:
	CPetGfxElement *_send, *_receive;
public:
	CSuccubusDeliveryGlyph() : CPetRemoteGlyph(),
		_send(nullptr), _receive(nullptr) {}

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CNavigationControllerGlyph : public CPetRemoteGlyph {
private:
	bool _flag;
	CPetGfxElement *_gfxElement;
public:
	CNavigationControllerGlyph() : CPetRemoteGlyph(),
		_flag(false), _gfxElement(nullptr) {}

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

class CGotoBottomOfWellGlyph : public CRemoteGotoGlyph {
public:
	CGotoBottomOfWellGlyph();
};

class CGotoTopOfWellGlyph : public CRemoteGotoGlyph {
public:
	CGotoTopOfWellGlyph();
};

class CGotoStateroomGlyph : public CRemoteGotoGlyph {
public:
	CGotoStateroomGlyph();
};

class CGotoBarGlyph : public CRemoteGotoGlyph {
public:
	CGotoBarGlyph();
};

class CGotoPromenadeDeckGlyph : public CRemoteGotoGlyph {
public:
	CGotoPromenadeDeckGlyph();
};

class CGotoArboretumGlyph : public CRemoteGotoGlyph {
public:
	CGotoArboretumGlyph();
};

class CGotoMusicRoomGlyph : public CRemoteGotoGlyph {
public:
	CGotoMusicRoomGlyph();
};

class CGotoRestaurantGlyph : public CRemoteGotoGlyph {
public:
	CGotoRestaurantGlyph();
};

class CSkipNavigationGlyph : public CPetRemoteGlyph {
protected:
	CPetGfxElement *_button;
public:
	CSkipNavigationGlyph() : CPetRemoteGlyph(), _button(nullptr) {}

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;
};

} // End of namespace Titanic

#endif /* TITANIC_PET_REMOTE_GLYPHS_H */
