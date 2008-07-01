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
 * $URL$
 * $Id$
 *
 */

#ifndef THEME_PARSER_H
#define THEME_PARSER_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "common/system.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"
#include "common/xmlparser.h"

#include "graphics/VectorRenderer.h"
#include "gui/InterfaceManager.h"

/**
 *********************************************
 ** Theme Description File format overview. **
 *********************************************
	This document is a work in progress.
	A more complete version will be posted on the wiki soon.

In the new version of the Graphical User Interface for ScummVM, almost
all properties regarding looks, design and positioning of the UI
elements are defined in a set of external files.

The chosen syntax for theme description is a basic subset of XML.
The process of theme description is divided in two main parts: Drawing
specifications for the vector renderer and theme design/layout
information for the actual theme engine.

These two core sections of a theme's description may be placed in a
single file or split for convenience across several files.

_DRAWING SPECIFICATIONS_

The process of rendering a widget on the screen is discretized into
several phases called "drawing steps". A set of such steps, which
generate a basic widget shape on screen is called a Draw Data set. The
GUI Engine loads all the different data sets for a given
widget and takes care of rendering it into the screen based on its
current state.

For example, the basic Button widget may be composed of several sets
of data: Drawing data for the button's idle state, drawing data for
when the button is hovered and drawing data for when the button is
pressed.

The functionality of each set of Drawing Data is hard-coded into the
Graphical User Interface; the most up to date version of all the
drawing sets may be found extensively commented in the 
"gui/InterfaceManager.h" file, in the DrawData enumeration inside the
InterfaceManager class.

In order to successfully parse and load a custom theme definition, the
whole list of Draw Data sets must be specified.

_THEME LAYOUT SPECIFICATIONS_

#######

_SYNTAX OVERVIEW AND PARAMETERS_

As stated before, all the theme description is done through a XML-like
syntax. The files are parsed left-to-right, ignoring extra whitespaces
and newlines. Parser data is interpreted during the parsing. As a
general guideline, theme files are composed of keys which may or not
contain specific values for the key and which may parent several
subkeys; independently of this, all keys must be properly closed with
the '/' operator.

		<parent_key value1 = "sample">

			<child_key1>
				<self_closed_child value2 = 124 />

				<external_closed_child value3 = 245>

				</external_closed_child>
			</child_key1>

			<child_key2 with_value = "sample">
				<subchild/>
			</child_key2>

		</parent_key>
				
Note how keys which contain no children may be closed by themselves
or with an external closure.

- Comments
The parser supports the same comment syntax as the C++ programming
language. Comment blocks may be specified by surrounding them with the
'/ *' and '* /' operators, while whole lines may be commented out by
preceding them with the // operator.

Block comments are parsed in a non-hungry manner, i.e. the first
comment closure is understood to close the whole commenting block, so
syntax like

	   / * hey look this comment finishes here * / or maybe here?? * /

is invalid.

- Section keys.
The section key is the root level of a theme description file. Each
file may contain one or more of these keys, which specifies the nature
of all their children, namely if the children keys specify drawing or
layout information. Its syntax is as follows:

   	   <layout_info platform = "NDS" resolution = "320x240">
	   				// ...
	   </layout_info>
	   
	   <render_info platform = "NDS" resolution = "320x240">
	   				// ...
	   </render_info>

The "layout_info" key specifies that all children keys contain
information regarding the layout of the theme, while the "render_info"
key specifies that all children keys contain information regarding the
looks of the theme.

Both keys support the two optional parameters "platform" and
"resolution", in order to make a certain layout apply to a single
resolution or to a single platform. To make a key apply for more than
one specific platform or resolution at the same time, you may separate
their names with commas.

	  <render_info platform = "nds, palmos, pocketpc">


- Render Info keys:
The children of a "render_info" key are expected to be one of these
kind:

	-- DrawData key:
	DrawData keys are the core of the rendering engine. They specifiy
	via their own children the looks of all the UI elements. Here's
	their syntax:

		  <drawdata id = "button_idle" cache = true platform = "NDS"
		  resolution = "320x240">

		  </drawdata>

	All drawdata keys must contain an "id" value, specifying which set
	of drawing data they implement. Here's a list of all possible ids.

	#########

	Remember that all these ids must me implemented in order for the
	parsing to be considered succesful.

	DrawData keys may also contain an optional boolean value "cache",
	which states if the set of DrawingSteps may be cached into the
	memory so it can be blit into the Overlay each frame or if the set
	of Drawing Steps should be performed individually each frame. If
	omitted, the "cache" value defaults to false.

	Also, just like the <render_info> key, DrawData keys may also
	contain optional "platform" and "resolution" values, making such
	draw steps specific for a single or several platforms or
	resolutions. In order to specify several platforms or resolutions,
	they must be separated by commas inside the key's value.

		 <drawdata id = "background_default" cache = true platform = "nds, palmos">
		 </drawdata>

	When making a set of Drawing Data for a widget specific to a
	single platform or resolution, remember that the set must be also
	implemented later generically for other platforms, or the
	rendering of the theme will fail in such platforms.
 
	Lastly, each DrawData key must contain at least a children
	"drawstep" subkey, with the necessary info for the
	VectorRenderer.

	- The DrawStep key
	The DrawStep key is probably the most complex definition of
	a ThemeDescription file. It contains enough information to
	allow the Vector Renderer to draw a basic or complex shape
	into the screen.

	DrawStep keys are supposed to have no children, so they must
	be either self-closed or closed externally.

	Their basic syntax is as follows:

	<drawstep func = "roundedsq">
	</drawstep>

	The only required value is the function "func" which states
	the drawing function that will be used, and it must be
	accompanied by a set of parameters specific to each drawing
	step. Here's a list of such parameters:

Common parameters for all functions:

fill = "none|foreground|background|gradient"
	 Specifies the fill mode for the drawn shape.
	 Possible values:

	 "none": Disables filling so only the stroke is shown.
	 "foreground" (default): Fills the whole shape with the active foreground
	 color.
	 "background": Fills the whole shape with the active background
	 color. 
	 "gradient": Fills the whole shape with the active gradient.

gradient_start = "R, G, B" | "color name"
gradient_end = "R, G, B" | "color name"
fg_color = "R, G, B" | "color name"
bg_color = "R, G, B" | "color name"
	Sets the active gradient, foreground or backgroud colors. Colors
	may be specified with their three components (red, green, blue)
	ranging each from 0 to 255, or via a color name which has
	previously been defined in the palette section.

	These colours have no default values. 

stroke = width (integer)
	Sets the active stroke width; strokes may be disabled by setting
	this value to 0. All shapes are automatically stroked with the
	given width and the active foreground color. Defaults to 1.

shadow = offset (integer)
	Sets the shadow offset. In the rendering engines that support it,
	drawn shapes will have a soft shadow offseted the given amount on
	their bottom-right corner. Defaults to 0 (disabled).

factor = amount (integer)
	The factor value specifies the displacement of the active
	gradient, i.e. its zoom level. It is only taken into account if
	the active fill mode is set to gradient. Defaults to 1.

Standard primitive drawing functions:

func = "circle"
	 Draws a primitive circle. Requires the additional parameter
	 "radius", with an integer defining the radius of the circle or
	 the "auto" value.

func = "square"
	 Draws a primitive square/rectangle. Requires no additional parameters.

func = "roundedsq"
	 Draws a square/rectangle with rounded corners. Requires the
	 additional parameter "radius" defining the radius of the rounded
	 corners.

func = "bevelsq"
	 Draws a square/rectangle with beveled borders. This square
	 ignores the active fill mode, as it is never filled. Requires the
	 additional parameter "bevel" with the amount of bevel.

func = "line"
	 Draws a line. If the "size" parameter is specified, the line will
	 be drawn ranging from the bottom-left corner to the top-right
	 corner of the defined box. Optionally, you may define the ending
	 point of the line with the "end" parameter.

func = "triangle"
	 Draws a triangle. Triangles are always isosceles, meaning they
	 are drawn inside the square defined by the position and size
	 values, with the given width as the base of the triangle and the
	 given height as the height of the triangle.

	 The optional parameter

	 orientation = "top|left|right|bottom"

	 may be specified to define the way in which the triangle is
	 pointing. Defaults to top.

func = "fill"
	 This call ignores position and size parameters, as it completely
	 fills the active drawing surface taken into account the active
	 fill mode and colors.
*/

namespace GUI {

using namespace Graphics;
using namespace Common;
class InterfaceManager;

class ThemeParser : public XMLParser {
	typedef void (VectorRenderer::*DrawingFunctionCallback)(const Common::Rect &, const DrawStep &);
	typedef bool (ThemeParser::*ParserCallback)();
	typedef GUI::InterfaceManager InterfaceManager;

public:
	ThemeParser(InterfaceManager *parent);

protected:
	InterfaceManager *_GUI;
	bool keyCallback(Common::String keyName);

	bool parserCallback_DRAWSTEP();
	bool parserCallback_DRAWDATA();
	bool parserCallback_palette();
	bool parserCallback_color();
	bool parserCallback_renderInfo();
	bool parserCallback_layoutInfo();
	bool parserCallback_defaultSet();

	Graphics::DrawStep *newDrawStep();
	Graphics::DrawStep *defaultDrawStep();
	bool parseDrawStep(ParserNode *stepNode, Graphics::DrawStep *drawstep, bool functionSpecific);

	Graphics::DrawStep *_defaultStepGlobal;
	Graphics::DrawStep *_defaultStepLocal;

	Common::HashMap<Common::String, DrawingFunctionCallback, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _drawFunctions;
	Common::HashMap<Common::String, ParserCallback, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _callbacks;

	struct PaletteColor {
		uint8 r, g, b;
	};

	Common::HashMap<Common::String, PaletteColor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _palette;
};

}

#endif
