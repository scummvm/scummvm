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

#ifndef ARCHETYPE_SEMANTIC
#define ARCHETYPE_SEMANTIC

/* Used by the SYNTAX unit, it provides the high-level semantic checking
 * as well as .ACX file output.
 */

#include "glk/archetype/array.h"
#include "glk/archetype/expression.h"
#include "glk/archetype/linked_list.h"

namespace Glk {
namespace Archetype {

/**
 * Works closely with the ID_Table to create and verify the various semantic
 * interpretations of identifiers, which are classified as either:
 *   TYPE_ID:      names a type definition template in the type list.
 *   OBJECT_ID:    names an object instantiation in the object list.
 *   ATTRIBUTE_ID: an attribute identifier.
 *   ENUMERATE_ID: an identifier like "open" or "closed" which is simply
 *                   assigned so that it can be tested.
 *   UNDEFINED_ID: Not defined anywhere.  If /K is asserted for CREATE, then
 *                 this is the value returned by default; otherwise,
 *                 ENUMERATE_ID is.
 *
 * @param f					the progfile that is being read.  Since this function is part
 *							of the first "pass", it needs access to the file being read.
 * @param id_number			the index in the ID table
 * @param interpretation	one of the constants above
 * @param ptr_to_data (IN)  if not nil, points to the data that the identifier represents
 *							(when first encountered)
 * @returns					depends on interpretation:
 *           TYPE_ID:      the index in Type_List
 *           OBJECT_ID:    the index in Object_List
 *           ATTRIBUTE_ID: the order the identifier was declared in, i.e.
 *                           for the first attribute encountered, 1, for the
 *                           second, 2, etc.
 *           ENUMERATE_ID: the unchanged id_number, for a simple unique number.
 *           UNDEFINED_ID: same as ENUMERATE_ID
 *
 *         In any case, classify_as returns 0 if there was an error.
 *         Such an error will have been printed by this routine, so there
 *         will be no need for the caller to print out its own.
 */
extern int classify_as(progfile &f, int id_number, ClassifyType interpretation, void *ptr_to_data);

/**
 * Given an ID_Table index, finds what it represents and returns an appropriate enumerated
 * type and index.
 * 
 * If /K is asserted, default return is UNDEFINED_ID; else it is ENUMERATE_ID.
 * @param id_number		integer index to ID_Table
 * @param meaning		Output classification of ID
 * @param number		Output integer appropriate to classification
 */
extern void get_meaning(int id_number, ClassifyType &meaning, int &number);

/**
 * Used for adding the number of an undefined identifier to a list to be produced
 * at the end of translation.
 */
extern void add_undefined(int the_ID);

/**
 * Displays the list of undefined identifiers collected with add_undefined
 */
extern bool display_undefined();

/**
 * Assumes that expression tree contains no OP_LPAREN nodes.
 * Ensures the following:
 * 1.    All OP_DOT operators have identifiers as their right-hand
 *       arguments, which are classified as ATTRIBUTE_ID's.
 * 2.    All assignment operators have OP_DOT operators or identifiers
 *       as their left-hand arguments, and any such identifiers are
 *       classified as ATTRIBUTE_ID's.
 * This is necessary because the only way to use the OP_DOT operator is
 * to discover the value of some attribute, and attributes are the only
 * things which may be assigned to.
 *
 * @param f				Program file (for logging errors)
 * @param the_expr		Expression to be verified
 */
extern bool verify_expr(progfile &f, ExprTree the_expr);

} // End of namespace Archetype
} // End of namespace Glk

#endif
