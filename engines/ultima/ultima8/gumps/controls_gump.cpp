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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/controls_gump.h"

#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/gumps/bind_gump.h"
#include "ultima/ultima8/gumps/paged_gump.h"
#include "ultima/ultima8/kernel/hid_manager.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

static const int font = 9;

class ControlEntryGump : public Gump {
public:
	ENABLE_RUNTIME_CLASSTYPE()
	ControlEntryGump(int x_, int y_, int width, const char *binding, const char *name);
	virtual ~ControlEntryGump(void);
	virtual void InitGump(Gump *newparent, bool take_focus = true) override;
	virtual void ChildNotify(Gump *child, uint32 message) override;
	void init();
protected:
	Pentagram::istring bindingName;
	Std::string displayedName;
	Gump *button;
};

DEFINE_RUNTIME_CLASSTYPE_CODE(ControlEntryGump, Gump)

ControlEntryGump::ControlEntryGump(int x_, int y_, int width, const char *binding, const char *name)
	: Gump(x_, y_, width, 5), bindingName(binding), displayedName(name) {
}

ControlEntryGump::~ControlEntryGump() {
}

void ControlEntryGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	init();
}

void ControlEntryGump::init() {
	// close all children so we can simply use this method to re init
	Std::list<Gump *>::iterator it;
	for (it = children.begin(); it != children.end(); ++it) {
		Gump *g = *it;
		if (! g->IsClosing())
			g->Close();
	}

	Std::vector<const char *> controls;

	Pentagram::Rect rect;
	button = new ButtonWidget(0, 0, displayedName, true, font, 0x80D000D0);
	button->InitGump(this);
	button->GetDims(rect);

	dims.h = rect.h;

	//HIDManager * hidmanager = HIDManager::get_instance();
	//hidmanager->getBindings(bindingName, controls);
	/*  int x = 120;
	    Std::vector<const char *>::iterator i;
	    for (i = controls.begin(); i != controls.end(); ++i)
	    {
	        Gump * widget;
	        widget = new TextWidget(x, 0, *i, true, font);
	        widget->InitGump(this, false);
	        widget->GetDims(rect);
	        x += rect.w + 5;
	    }
	*/
}

void ControlEntryGump::ChildNotify(Gump *child, uint32 message) {
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK) {
		if (cid == button->getObjId()) {
			ModalGump *gump = new BindGump(&bindingName, parent);
			gump->InitGump(0);
			gump->setRelativePosition(CENTER);
		}
	} else if (message == BindGump::UPDATE) {
		parent->ChildNotify(child, message);
	}
}

DEFINE_RUNTIME_CLASSTYPE_CODE(ControlsGump, Gump)

ControlsGump::ControlsGump(): Gump(0, 0, 5, 5) {
}

ControlsGump::~ControlsGump() {
}

void ControlsGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	dims.w = 220;
	dims.h = 120;

	Gump *widget = new TextWidget(0, 0, "Controls", true, font);
	widget->InitGump(this, false);
	widget = new TextWidget(120, 0, "Keys", true, font);
	widget->InitGump(this, false);
}

void ControlsGump::addEntry(const char *binding, const char *name, int &x_, int &y_) {
	Pentagram::Rect rect;
	Gump *widget = new ControlEntryGump(x_, y_, dims.w - x_, binding, name);
	widget->InitGump(this);
	widget->GetDims(rect);
	y_ += rect.h;
}

void ControlsGump::ChildNotify(Gump *child, uint32 message) {
	if (message == BindGump::UPDATE) {
		Std::list<Gump *>::iterator it;
		for (it = children.begin(); it != children.end(); ++it) {
			ControlEntryGump *g =  p_dynamic_cast<ControlEntryGump *>(*it);
			if (g)
				g->init();
		}
	}
}

void ControlsGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

bool ControlsGump::OnKeyDown(int key, int mod) {
	return false;
}

//static
void ControlsGump::showEngineMenu(PagedGump *pagedgump) {
	ControlsGump *gump = new ControlsGump();
	gump->InitGump(pagedgump, false);
	int x = 4;
	int y = 12;
	gump->addEntry("quickSave", "Quick Save", x, y);
	gump->addEntry("quickLoad", "Quick Load", x, y);
	gump->addEntry("avatarInStasis", "Stasis Mode", x, y);
	gump->addEntry("engineStats", "Engine Stats", x, y);
	gump->addEntry("paintEditorItems", "View Editor Items", x, y);
	gump->addEntry("itemLocator", "Locate Item", x, y);
	gump->addEntry("toggleFrameByFrame", "Single Frame Mode", x, y);
	gump->addEntry("advanceFrameByFrame", "Next Frame", x, y);
	gump->addEntry("toggleConsole", "Console", x, y);

	pagedgump->addPage(gump);
}

// static
void ControlsGump::showU8Menu(PagedGump *pagedgump) {
	ControlsGump *gump = new ControlsGump();
	gump->InitGump(pagedgump, false);
	int x = 4;
	int y = 12;
	gump->addEntry("toggleCombat", "Combat Mode", x, y);
	gump->addEntry("openInventory", "Open Inventory", x, y);
	gump->addEntry("openBackpack", "Open Backpack", x, y);
	gump->addEntry("recall", "Recall", x, y);
	gump->addEntry("runFirstEgg", "First Scene", x, y);
	gump->addEntry("runExecutionEgg", "Execution Scene", x, y);
	gump->addEntry("u8ShapeViewer", "Shape Viewer", x, y);
	gump->addEntry("showMenu", "Menu", x, y);
	gump->addEntry("quit", "Quit", x, y);

	pagedgump->addPage(gump);
}

bool ControlsGump::loadData(IDataSource *ids) {
	return true;
}

void ControlsGump::saveData(ODataSource *ods) {
}

} // End of namespace Ultima8
} // End of namespace Ultima
