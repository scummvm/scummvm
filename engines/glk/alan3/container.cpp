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

#include "glk/alan3/container.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/current.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/output.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
ContainerEntry *containers;  /* Container table pointer */


/*----------------------------------------------------------------------*/
static int countInContainer(int containerIndex) { /* IN - the container to count in */
	int j = 0;

	for (uint instanceIndex = 1; instanceIndex <= header->instanceMax; instanceIndex++)
		if (isIn(instanceIndex, containerIndex, DIRECT))
			/* Then it's in this container also */
			j++;
	return (j);
}


/*----------------------------------------------------------------------*/
static int sumAttributeInContainer(
    Aint containerIndex,         /* IN - the container to sum */
    Aint attributeIndex          /* IN - the attribute to sum over */
) {
	uint instanceIndex;
	int sum = 0;

	for (instanceIndex = 1; instanceIndex <= header->instanceMax; instanceIndex++)
		if (isIn(instanceIndex, containerIndex, DIRECT)) {  /* Then it's directly in this cont */
			if (instances[instanceIndex].container != 0)    /* This is also a container! */
				sum = sum + sumAttributeInContainer(instanceIndex, attributeIndex);
			sum = sum + getInstanceAttribute(instanceIndex, attributeIndex);
		}
	return (sum);
}


/*----------------------------------------------------------------------*/
static bool containerIsEmpty(int container) {
	uint i;

	for (i = 1; i <= header->instanceMax; i++)
		if (isDescribable(i) && isIn(i, container, TRANSITIVE))
			return FALSE;
	return TRUE;
}


/*======================================================================*/
void describeContainer(CONTEXT, int container) {
	if (!containerIsEmpty(container) && !isOpaque(container))
		CALL1(list, container)
}


/*======================================================================*/
bool passesContainerLimits(CONTEXT, Aint theContainer, Aint theAddedInstance) {
	LimitEntry *limit;
	Aword props;

	if (!isAContainer(theContainer))
		syserr("Checking limits for a non-container.");

	/* Find the container properties */
	props = instances[theContainer].container;

	if (containers[props].limits != 0) { /* Any limits at all? */
		for (limit = (LimitEntry *) pointerTo(containers[props].limits); !isEndOfArray(limit); limit++)
			if ((int)limit->atr == 1 - I_COUNT) { /* TODO This is actually some encoding of the attribute number, right? */
				if (countInContainer(theContainer) >= (int)limit->val) {
					R0CALL1(interpret, limit->stms)
					return (FALSE);
				}
			} else {
				if (sumAttributeInContainer(theContainer, limit->atr) + getInstanceAttribute(theAddedInstance, limit->atr) > limit->val) {
					R0CALL1(interpret, limit->stms)
					return (FALSE);
				}
			}
	}
	return (TRUE);
}


/*======================================================================*/
int containerSize(int container, ATrans trans) {
	Aword i;
	Aint count = 0;

	for (i = 1; i <= header->instanceMax; i++) {
		if (isIn(i, container, trans))
			count++;
	}
	return (count);
}

/*======================================================================*/
void list(CONTEXT, int container) {
	uint i;
	Aword props;
	Aword foundInstance[2] = {0, 0};
	int found = 0;
	Aint previousThis = current.instance;

	current.instance = container;

	/* Find container table entry */
	props = instances[container].container;
	if (props == 0) syserr("Trying to list something not a container.");

	for (i = 1; i <= header->instanceMax; i++) {
		if (isDescribable(i)) {
			/* We can only see objects and actors directly in this container... */
			if (admin[i].location == container) { /* Yes, it's in this container */
				if (found == 0) {
					if (containers[props].header != 0) {
						CALL1(interpret, containers[props].header)
					} else {
						if (isAActor(containers[props].owner))
							printMessageWithInstanceParameter(M_CARRIES, containers[props].owner);
						else
							printMessageWithInstanceParameter(M_CONTAINS, containers[props].owner);
					}
					foundInstance[0] = i;
				} else if (found == 1)
					foundInstance[1] = i;
				else {
					printMessageWithInstanceParameter(M_CONTAINS_COMMA, i);
				}
				found++;
			}
		}
	}

	if (found > 0) {
		if (found > 1)
			printMessageWithInstanceParameter(M_CONTAINS_AND, foundInstance[1]);
		printMessageWithInstanceParameter(M_CONTAINS_END, foundInstance[0]);
	} else {
		if (containers[props].empty != 0) {
			CALL1(interpret, containers[props].empty)
		} else {
			if (isAActor(containers[props].owner))
				printMessageWithInstanceParameter(M_EMPTYHANDED, containers[props].owner);
			else
				printMessageWithInstanceParameter(M_EMPTY, containers[props].owner);
		}
	}
	needSpace = TRUE;
	current.instance = previousThis;
}

} // End of namespace Alan3
} // End of namespace Glk
