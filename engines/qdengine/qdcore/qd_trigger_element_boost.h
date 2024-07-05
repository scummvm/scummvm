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

#ifndef QDENGINE_QDCORE_QD_TRIGGER_ELEMENT_BOOST_H
#define QDENGINE_QDCORE_QD_TRIGGER_ELEMENT_BOOST_H

#ifdef _QUEST_EDITOR
#include <boost/type_traits.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <additional/itr_traits_spec.hpp>


BROKEN_COMPILER_ITER_TRAITS_PTR_SPECIALIZATION(qdTriggerLink)
BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(qdTriggerLink)
BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(qdTriggerElement)
BROKEN_COMPILER_ITER_TRAITS_PTR_SPECIALIZATION(qdTriggerElement)
BROKEN_COMPILER_PTR_ITER_TRAITS_SPECIALIZATION(qdTriggerElement)

BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(qdTriggerElementPtr)
BROKEN_COMPILER_ITER_TRAITS_PTR_SPECIALIZATION(qdTriggerElementPtr)
BROKEN_COMPILER_PTR_ITER_TRAITS_SPECIALIZATION(qdTriggerElementPtr)

BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(qdTriggerElementConstPtr)
BROKEN_COMPILER_ITER_TRAITS_PTR_SPECIALIZATION(qdTriggerElementConstPtr)
BROKEN_COMPILER_PTR_ITER_TRAITS_SPECIALIZATION(qdTriggerElementConstPtr)
#endif//_QUEST_EDITOR

#endif // QDENGINE_QDCORE_QD_TRIGGER_ELEMENT_BOOST_H
