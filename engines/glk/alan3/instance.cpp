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

#include "glk/alan3/instance.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/attribute.h"
#include "glk/alan3/current.h"
#include "glk/alan3/container.h"
#include "glk/alan3/debug.h"
#include "glk/alan3/checkentry.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/options.h"
#include "glk/alan3/output.h"
#include "glk/alan3/class.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/actor.h"
#include "glk/alan3/literal.h"
#include "glk/alan3/dictionary.h"
#include "glk/alan3/location.h"
#include "glk/alan3/compatibility.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */

InstanceEntry *instances;   /* Instance table pointer */

AdminEntry *admin;      /* Administrative data about instances */
AttributeEntry *attributes; /* Dynamic attribute values */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void AdminEntry::synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(location);

	Aword attr = 0;
	s.syncAsUint32LE(attr);

	s.syncAsUint32LE(alreadyDescribed);
	s.syncAsUint32LE(visitsCount);
	s.syncAsUint32LE(script);
	s.syncAsUint32LE(step);
	s.syncAsUint32LE(waitCount);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Instance query methods */

/*======================================================================*/
bool isA(int instance, int ancestor) {
	int parent;

	if (isLiteral(instance))
		parent = literals[instance - header->instanceMax]._class;
	else
		parent = instances[instance].parent;
	while (parent != 0 && parent != ancestor)
		parent = classes[parent].parent;

	return (parent != 0);
}


bool isAObject(int instance) {
	return isA(instance, OBJECT);
}

bool isAContainer(int instance) {
	return instance != 0 && !isLiteral(instance) && instances[instance].container != 0;
}

bool isAActor(int instance) {
	return isA(instance, ACTOR);
}

bool isALocation(int instance) {
	return isA(instance, LOCATION);
}


bool isLiteral(int instance) {
	return instance > (int)header->instanceMax;
}

bool isANumeric(int instance) {
	return isLiteral(instance) && literals[literalFromInstance(instance)].type == NUMERIC_LITERAL;
}

bool isAString(int instance) {
	return isLiteral(instance) && literals[literalFromInstance(instance)].type == STRING_LITERAL;
}


/*======================================================================*/
bool isOpaque(int container) {
	return getInstanceAttribute(container, OPAQUEATTRIBUTE);
}


/*======================================================================*/
void setInstanceAttribute(int instance, int attribute, Aptr value) {
	char str[80];

	if (instance > 0 && instance <= (int)header->instanceMax) {
		setAttribute(admin[instance].attributes, attribute, value);
		if (isALocation(instance) && attribute != VISITSATTRIBUTE)
			/* If it wasn't the VISITSATTRIBUTE the location may have
			   changed so describe next time */
			admin[instance].visitsCount = 0;
	} else {
		sprintf(str, "Can't SET/MAKE instance (%d).", instance);
		syserr(str);
	}
}


/*======================================================================*/
void setInstanceStringAttribute(int instance, int attribute, char *string) {
	deallocate(fromAptr(getInstanceAttribute(instance, attribute)));
	setInstanceAttribute(instance, attribute, toAptr(string));
}


/*======================================================================*/
void setInstanceSetAttribute(int instance, int attribute, Aptr set) {
	freeSet((Set *)fromAptr(getInstanceAttribute(instance, attribute)));
	setInstanceAttribute(instance, attribute, set);
}


/*----------------------------------------------------------------------*/
static Aptr literalAttribute(int literal, int attribute) {
	if (isPreBeta3(header->version)) {
		if (attribute == 1)
			return literals[literalFromInstance(literal)].value;
		else
			return 0;
	} else {
		if (attribute == 0)
			return literals[literalFromInstance(literal)].value;
		else
			return getAttribute(admin[header->instanceMax].attributes, attribute);
	}
	return (EOD);
}


/*======================================================================*/
Aptr getInstanceAttribute(int instance, int attribute) {
	char str[80];

	if (isLiteral(instance))
		return literalAttribute(instance, attribute);
	else {
		if (instance > 0 && instance <= (int)header->instanceMax) {
			if (attribute == -1)
				return locationOf(instance);
			else
				return getAttribute(admin[instance].attributes, attribute);
		} else {
			sprintf(str, "Can't ATTRIBUTE item (%d).", instance);
			syserr(str);
		}
	}
	return (EOD);
}


/*======================================================================*/
char *getInstanceStringAttribute(int instance, int attribute) {
	return scumm_strdup((char *)fromAptr(getInstanceAttribute(instance, attribute)));
}


/*======================================================================*/
Set *getInstanceSetAttribute(int instance, int attribute) {
	return copySet((Set *)fromAptr(getInstanceAttribute(instance, attribute)));
}


/*----------------------------------------------------------------------*/
static void verifyInstance(int instance, const char *action) {
	char message[200];

	if (instance == 0) {
		sprintf(message, "Can't %s instance (%d).", action, instance);
		syserr(message);
	} else if (instance > (int)header->instanceMax) {
		sprintf(message, "Can't %s instance (%d > instanceMax).", action, instance);
		syserr(message);
	}
}


/*======================================================================*/
bool isHere(int id, ATrans transitivity) {
	verifyInstance(id, "HERE");

	return isAt(id, current.location, transitivity);
}


/*======================================================================*/
bool isNearby(int instance, ATrans transitivity) {
	verifyInstance(instance, "NEARBY");

	if (isALocation(instance))
		return exitto(current.location, instance);
	else
		return exitto(current.location, where(instance, transitivity));
}


/*======================================================================*/
bool isNear(int instance, int other, ATrans trans) {
	Aint l1, l2;

	verifyInstance(instance, "NEAR");

	if (isALocation(instance))
		l1 = instance;
	else
		l1 = where(instance, trans);
	if (isALocation(other))
		l2 = other;
	else
		l2 = where(other, trans);
	return exitto(l2, l1);
}


/*======================================================================*/
/* Look in a container to see if the instance is in it. */
bool isIn(int instance, int container, ATrans trans) {
	int loc;

	if (!isAContainer(container))
		syserr("IN in a non-container.");

	if (trans == DIRECT)
		return admin[instance].location == container;
	else {
		loc = admin[instance].location;
		if (trans == INDIRECT && loc != 0 && !isA(loc, LOCATION))
			loc = admin[loc].location;
		while (loc != 0 && !isA(loc, LOCATION))
			if (loc == container)
				return TRUE;
			else
				loc = admin[loc].location;
		return FALSE;
	}
}



/*======================================================================*/
/* Look see if an instance is AT another. */
bool isAt(int instance, int other, ATrans trans) {
	if (instance == 0 || other == 0) return FALSE;

	if (isALocation(instance)) {
		/* Nested locations */
		/* TODO - What if the other is not a location? */
		int curr = admin[instance].location;
		switch (trans) {
		case DIRECT:
			return admin[instance].location == other;

		case INDIRECT:
			if (curr == other)
				return FALSE;
			curr = admin[curr].location;
			// fall through

		case TRANSITIVE:
			while (curr != 0) {
				if (curr == other)
					return TRUE;
				else
					curr = admin[curr].location;
			}
			return FALSE;

		default:
			break;
		}

		syserr("Unexpected value in switch in isAt() for location");
		return FALSE;
	} else if (isALocation(other)) {
		/* Instance is not a location but other is */
		switch (trans) {
		case DIRECT:
			return admin[instance].location == other;
		case INDIRECT:
			if (admin[instance].location == other)
				return FALSE;   /* Directly, so not Indirectly */
			// fall through
		case TRANSITIVE: {
			int location = locationOf(instance);
			int curr = other;
			while (curr != 0) {
				if (curr == location)
					return TRUE;
				else
					curr = admin[curr].location;
			}
			return FALSE;
		}
		default:
			break;
		}
		syserr("Unexpected value in switch in isAt() for non-location");
		return FALSE;
	} else {
		/* Other is also not a location */
		switch (trans) {
		case DIRECT:
			return positionOf(instance) == admin[other].location;
		case INDIRECT: {
			int location = locationOf(instance);
			int curr = other;
			if (location == curr)
				return FALSE;
			else
				curr = admin[curr].location;
			while (curr != 0) {
				if (curr == location)
					return TRUE;
				else
					curr = admin[curr].location;
			}
			return FALSE;
		}
		case TRANSITIVE: {
			int location = locationOf(other);
			int curr = locationOf(instance);
			bool ok = FALSE;
			while (curr != 0 && !ok) {
				if (curr == location)
					ok = TRUE;
				else
					curr = admin[curr].location;
			}
			return ok;
		}
		default:
			break;
		}
		syserr("Unexpected value in switch in isAt() for non-location");
		return FALSE;
	}
}


/*======================================================================*/
/* Return the *location* of an instance, transitively, i.e. the first
   location instance found when traversing the containment/position
   links. If that didn't turn up a location see if it was in a
   container that is somewhere, or a THING that is nowhere. It might
   also be an ENTITY which is always everywhere so take that to mean
   where the hero is. */
int locationOf(int instance) {
	int position;
	int container = 0;

	verifyInstance(instance, "get LOCATION of");

	position = admin[instance].location;
	while (position != 0 && !isALocation(position)) {
		container = position;   /* Remember innermost container */
		position = admin[position].location;
	}
	if (position > NOWHERE) /* It was a location so return that */
		return position;
	else {
		/* If we did not find a location then it might be in a container */
		if (container != 0)
			instance = container;
		/* If the instance or the container it was in is a THING then its nowhere. */
		if (isA(instance, THING))
			return NOWHERE;     /* #nowhere */
		else if (isALocation(instance))
			return NO_LOCATION; /* No location */
		else
			return locationOf(HERO);
	}
}


/*======================================================================*/
/* Return the current position of an instance, directly or not */
/* TODO: this will be a possible duplicate of where() */
int positionOf(int instance) {
	return admin[instance].location;
}


/*======================================================================*/
/* Return the current position of an instance, directly or not */
int where(int instance, ATrans trans) {
	verifyInstance(instance, "WHERE");

	if (isALocation(instance))
		return 0;
	else if (trans == DIRECT)
		return admin[instance].location;
	else
		return locationOf(instance);
}


/*----------------------------------------------------------------------*/
static bool executeInheritedMentioned(CONTEXT, int cls) {
	if (cls == 0) return FALSE;

	if (classes[cls].mentioned) {
		R0CALL1(interpret, classes[cls].mentioned)
		return TRUE;
	} else {
		bool flag;
		R0FUNC1(executeInheritedMentioned, flag, classes[cls].parent)
		return flag;
	}
}


/*----------------------------------------------------------------------*/
static bool mention(CONTEXT, int instance) {
	if (instances[instance].mentioned) {
		R0CALL1(interpret, instances[instance].mentioned)
		return TRUE;
	} else {
		bool flag;
		R0FUNC1(executeInheritedMentioned, flag, instances[instance].parent)
		return flag;
	}
}


/*======================================================================*/
void sayInstance(CONTEXT, int instance) {
#ifdef SAY_INSTANCE_WITH_PLAYER_WORDS_IF_PARAMETER
	int p, i;

	/* Find the id in the parameters... */
	if (params != NULL)
		for (p = 0; params[p].code != EOD; p++)
			if (params[p].code == instance) {
				/* Found it so.. */
				if (params[p].firstWord == EOD) /* Any words he used? */
					break;      /* No... */
				else {              /* Yes, so use them... */
					char *capitalized;
					/* Assuming the noun is the last word we can simply output the adjectives... */
					for (i = params[p].firstWord; i <= params[p].lastWord - 1; i++)
						output((char *)pointerTo(dict[wrds[i]].wrd));
					/* ... and then the noun, capitalized if necessary */
					if (header->capitalizeNouns) {
						capitalized = scumm_strdup((char *)pointerTo(dict[wrds[params[p].lastWord]].wrd));
						capitalized[0] = IsoToUpperCase(capitalized[0]);
						output(capitalized);
						deallocate(capitalized);
					} else
						output((char *)pointerTo(dict[wrds[params[p].lastWord]].wrd));
				}
				return;
			}
#endif

	bool flag;
	FUNC1(mention, flag, instance)
	if (!flag)
		CALL1(interpret, instances[instance].name)
}


/*======================================================================*/
void sayInteger(int value) {
	char buf[25];

	if (isHere(HERO, /*FALSE*/ TRANSITIVE)) {
		sprintf(buf, "%d", value);
		output(buf);
	}
}


/*======================================================================*/
void sayString(char *string) {
	if (isHere(HERO, /*FALSE*/ TRANSITIVE))
		output(string);
	deallocate(string);
}


/*----------------------------------------------------------------------*/
static void sayLiteral(int literal) {
	char *str;

	if (isANumeric(literal))
		sayInteger(literals[literal - header->instanceMax].value);
	else {
		str = (char *)scumm_strdup((char *)fromAptr(literals[literal - header->instanceMax].value));
		sayString(str);
	}
}


/*----------------------------------------------------------------------*/
static char *wordWithCode(int classBit, int code) {
	int w;
	char str[50];

	for (w = 0; w < dictionarySize; w++)
		if (dictionary[w].code == (Aword)code && ((classBit & dictionary[w].classBits) != 0))
			return (char *)pointerTo(dictionary[w].string);
	sprintf(str, "Could not find word of class %d with code %d.", classBit, code);
	syserr(str);
	return NULL;
}


/*----------------------------------------------------------------------*/
static bool sayInheritedDefiniteForm(CONTEXT, int cls) {
	if (cls == 0) {
		syserr("No default definite article");
		return FALSE;
	} else {
		if (classes[cls].definite.address) {
			R0CALL1(interpret, classes[cls].definite.address)
			return classes[cls].definite.isForm;
		} else {
			bool flag;
			R0FUNC1(sayInheritedDefiniteForm, flag, classes[cls].parent)
			return flag;
		}
	}
}


/*----------------------------------------------------------------------*/
static void sayDefinite(CONTEXT, int instance) {
	if (instances[instance].definite.address) {
		CALL1(interpret, instances[instance].definite.address)

		if (!instances[instance].definite.isForm)
			CALL1(sayInstance, instance)
	} else {
		bool flag;
		FUNC1(sayInheritedDefiniteForm, flag, instances[instance].parent)
		if (!flag)
			CALL1(sayInstance, instance)
	}
}


/*----------------------------------------------------------------------*/
static bool sayInheritedIndefiniteForm(CONTEXT, int cls) {
	if (cls == 0) {
		syserr("No default indefinite article");
		return FALSE;
	} else {
		if (classes[cls].indefinite.address) {
			R0CALL1(interpret, classes[cls].indefinite.address)
			return classes[cls].indefinite.isForm;
		} else {
			bool flag;
			R0FUNC1(sayInheritedIndefiniteForm, flag, classes[cls].parent)
			return flag;
		}
	}
}


/*----------------------------------------------------------------------*/
static void sayIndefinite(CONTEXT, int instance) {
	if (instances[instance].indefinite.address) {
		CALL1(interpret, instances[instance].indefinite.address)

		if (!instances[instance].indefinite.isForm)
			CALL1(sayInstance, instance)
	} else {
		bool flag;
		FUNC1(sayInheritedIndefiniteForm, flag, instances[instance].parent)
		if (!flag)
			CALL1(sayInstance, instance)
	}
}


/*----------------------------------------------------------------------*/
static bool sayInheritedNegativeForm(CONTEXT, int cls) {
	if (cls == 0) {
		syserr("No default negative form");
		return FALSE;
	} else {
		if (classes[cls].negative.address) {
			R0CALL1(interpret, classes[cls].negative.address)
			return classes[cls].negative.isForm;
		} else {
			bool flag;
			R0FUNC1(sayInheritedNegativeForm, flag, classes[cls].parent)
			return flag;
		}
	}
}


/*----------------------------------------------------------------------*/
static void sayNegative(CONTEXT, int instance) {
	if (instances[instance].negative.address) {
		CALL1(interpret, instances[instance].negative.address)
		if (!instances[instance].negative.isForm)
			CALL1(sayInstance, instance)
	} else {
		bool flag;
		FUNC1(sayInheritedNegativeForm, flag, instances[instance].parent)
		if (!flag)
			CALL1(sayInstance, instance)
	}
}


/*----------------------------------------------------------------------*/
static void sayInheritedPronoun(CONTEXT, int instance) {
	if (instance == 0)
		syserr("No default pronoun");
	else {
		if (classes[instance].pronoun != 0)
			output(wordWithCode(PRONOUN_BIT, classes[instance].pronoun));
		else
			CALL1(sayInheritedPronoun, classes[instance].parent)
	}
}


/*----------------------------------------------------------------------*/
static void sayPronoun(CONTEXT, int instance) {
	if (instances[instance].pronoun != 0)
		output(wordWithCode(PRONOUN_BIT, instances[instance].pronoun));
	else
		CALL1(sayInheritedPronoun, instances[instance].parent)
}


/*----------------------------------------------------------------------*/
static void sayArticleOrForm(CONTEXT, int id, SayForm form) {
	if (!isLiteral(id)) {
		switch (form) {
		case SAY_DEFINITE:
			CALL1(sayDefinite, id)
			break;
		case SAY_INDEFINITE:
			CALL1(sayIndefinite, id)
			break;
		case SAY_NEGATIVE:
			CALL1(sayNegative, id)
			break;
		case SAY_PRONOUN:
			CALL1(sayPronoun, id)
			break;
		case SAY_SIMPLE:
			CALL1(say, id)
			break;
		default:
			syserr("Unexpected form in 'sayArticleOrForm()'");
		}
	} else {
		CALL1(say, id)
	}
}


/*======================================================================*/
void say(CONTEXT, int instance) {
	Aword previousInstance = current.instance;
	current.instance = instance;

	if (isHere(HERO, /*FALSE*/ TRANSITIVE)) {
		if (isLiteral(instance))
			sayLiteral(instance);
		else {
			verifyInstance(instance, "SAY");
			sayInstance(context, instance);
		}
	}
	current.instance = previousInstance;
}


/*======================================================================*/
void sayForm(CONTEXT, int instance, SayForm form) {
	Aword previousInstance = current.instance;
	current.instance = instance;

	sayArticleOrForm(context, instance, form);

	current.instance = previousInstance;
}


/*======================================================================*/
bool isDescribable(int instance) {
	return isAObject(instance) || isAActor(instance);
}


/*----------------------------------------------------------------------*/
static bool inheritsDescriptionFrom(int cls) {
	if (classes[cls].description != 0)
		return TRUE;
	else if (classes[cls].parent != 0)
		return inheritsDescriptionFrom(classes[cls].parent);
	else
		return FALSE;
}


/*======================================================================*/
bool hasDescription(int instance) {
	if (instances[instance].description != 0)
		return TRUE;
	else if (instances[instance].parent != 0)
		return inheritsDescriptionFrom(instances[instance].parent);
	else
		return FALSE;
}


/*----------------------------------------------------------------------*/
static void describeClass(CONTEXT, int instance) {
	if (classes[instance].description != 0) {
		/* This class has a description, run it */
		CALL1(interpret, classes[instance].description)
	} else {
		/* Search up the inheritance tree, if any, to find a description */
		if (classes[instance].parent != 0)
			CALL1(describeClass, classes[instance].parent)
	}
}


/*======================================================================*/
void describeAnything(CONTEXT, int instance) {
	if (instances[instance].description != 0) {
		/* This instance has its own description, run it */
		CALL1(interpret, instances[instance].description)
	} else {
		/* Search up the inheritance tree to find a description */
		if (instances[instance].parent != 0)
			CALL1(describeClass, instances[instance].parent)
	}
	admin[instance].alreadyDescribed = TRUE;
}


/*----------------------------------------------------------------------*/
static void describeObject(CONTEXT, int object) {
	if (hasDescription(object)) {
		CALL1(describeAnything, object)
	} else {
		printMessageWithInstanceParameter(M_SEE_START, object);
		printMessage(M_SEE_END);
		if (instances[object].container != 0)
			CALL1(describeContainer, object)
	}
	admin[object].alreadyDescribed = TRUE;
}


/*----------------------------------------------------------------------*/
static bool inheritedDescriptionCheck(CONTEXT, int cls) {
	if (cls == 0) return TRUE;

	bool flag;
	R0FUNC1(inheritedDescriptionCheck, flag, classes[cls].parent)
	if (!flag) return FALSE;

	if (classes[cls].descriptionChecks == 0) return TRUE;
	R0FUNC2(checksFailed, flag, classes[cls].descriptionChecks, TRUE)
	return !flag;
}

/*----------------------------------------------------------------------*/
static bool descriptionCheck(CONTEXT, int instance) {
	int previousInstance = current.instance;
	bool r;

	current.instance = instance;
	if (inheritedDescriptionCheck(context, instances[instance].parent)) {
		if (instances[instance].checks == 0)
			r = TRUE;
		else
			r = !checksFailed(context, instances[instance].checks, TRUE);
	}
	else
		r = FALSE;
	current.instance = previousInstance;
	return r;
}


/*======================================================================*/
void describeInstances(CONTEXT) {
	uint i;
	int lastInstanceFound = 0;
	int found = 0;

	/* First describe every object here with its own description */
	for (i = 1; i <= header->instanceMax; i++)
		if (admin[i].location == current.location && isAObject(i) &&
		        !admin[i].alreadyDescribed && hasDescription(i))
			CALL1(describe, i)

	/* Then list all things without a description */
	for (i = 1; i <= header->instanceMax; i++)
		if (admin[i].location == current.location
		        && !admin[i].alreadyDescribed
		        && isAObject(i)
		        && descriptionCheck(context, i)) {
			if (found == 0)
				printMessageWithInstanceParameter(M_SEE_START, i);
			else if (found > 1)
				printMessageWithInstanceParameter(M_SEE_COMMA, lastInstanceFound);
			admin[i].alreadyDescribed = TRUE;

			// TODO : isOpaque()
			if (instances[i].container && containerSize(i, DIRECT) > 0 && !getInstanceAttribute(i, OPAQUEATTRIBUTE)) {
				if (found > 0)
					printMessageWithInstanceParameter(M_SEE_AND, i);
				printMessage(M_SEE_END);
				CALL1(describeContainer, i)
				found = 0;
				continue;       /* Actually start another list. */
			}
			found++;
			lastInstanceFound = i;
		}

	if (found > 0) {
		if (found > 1) {
			printMessageWithInstanceParameter(M_SEE_AND, lastInstanceFound);
		}
		printMessage(M_SEE_END);
	}

	/* Finally all actors with a separate description */
	for (i = 1; i <= header->instanceMax; i++)
		if (admin[i].location == current.location && i != HERO && isAActor(i)
		        && !admin[i].alreadyDescribed)
			CALL1(describe, i)

	/* Clear the describe flag for all instances */
	for (i = 1; i <= header->instanceMax; i++)
		admin[i].alreadyDescribed = FALSE;
}


/*======================================================================*/
bool describe(CONTEXT, int instance) {
	bool descriptionOk;
	int previousInstance = current.instance;

	current.instance = instance;
	verifyInstance(instance, "DESCRIBE");

	if (descriptionCheck(context, instance)) {
		descriptionOk = TRUE;
		if (isAObject(instance)) {
			describeObject(context, instance);
		} else if (isAActor(instance)) {
			describeActor(context, instance);
		} else
			describeAnything(context, instance);
	} else
		descriptionOk = FALSE;

	current.instance = previousInstance;
	return descriptionOk;
}


/*----------------------------------------------------------------------*/
static void locateIntoContainer(CONTEXT, Aword theInstance, Aword theContainer) {
	if (!isA(theInstance, containers[instances[theContainer].container]._class))
		printMessageUsing2InstanceParameters(M_CANNOTCONTAIN, theContainer, theInstance);
	else if (passesContainerLimits(context, theContainer, theInstance))
		admin[theInstance].location = theContainer;
	else
		abortPlayerCommand(context);
}


/*----------------------------------------------------------------------*/
static void locateLocation(Aword loc, Aword whr) {
	Aint l = whr;

	/* Ensure this does not create a recursive location chain */
	while (l != 0) {
		if (admin[l].location == (int)loc)
			apperr("Locating a location that would create a recursive loop of locations containing each other.");
		else
			l = admin[l].location;
	}
	admin[loc].location = whr;
}


/*----------------------------------------------------------------------*/
static void locateObject(CONTEXT, Aword obj, Aword whr) {
	if (isAContainer(whr)) { /* Into a container */
		locateIntoContainer(context, obj, whr);
	} else {
		admin[obj].location = whr;
		/* Make sure the location is described since it's changed */
		admin[whr].visitsCount = 0;
	}
}


/*----------------------------------------------------------------------*/
static void traceEnteredClass(Aint theClass, bool empty) {
	printf("\n<ENTERED in class ");
	printf("%s", idOfClass(theClass));
	printf("[%d]%s>\n", theClass, empty ? " is empty" : ":");
}


/*----------------------------------------------------------------------*/
static void traceEnteredInstance(CONTEXT, Aint instance, bool empty) {
	printf("\n<ENTERED in instance ");
	traceSay(context, instance);
	printf("[%d]%s>\n", instance, empty ? " is empty" : "");
}


/*----------------------------------------------------------------------*/
static void executeInheritedEntered(CONTEXT, Aint theClass) {
	if (theClass == 0) return;
	CALL1(executeInheritedEntered, classes[theClass].parent)

	if (traceSectionOption)
		traceEnteredClass(theClass, classes[theClass].entered == 0);
	if (classes[theClass].entered) {
		CALL1(interpret, classes[theClass].entered)
	}
}


/*----------------------------------------------------------------------*/
static void executeEntered(CONTEXT, Aint instance) {
	int currentInstance = current.instance;
	current.instance = instance;
	if (admin[instance].location != 0)
		CALL1(executeEntered, admin[instance].location)
	CALL1(executeInheritedEntered, instances[instance].parent)

	if (traceSectionOption)
		CALL2(traceEnteredInstance, instance, instances[instance].entered == 0)
	if (instances[instance].entered != 0) {
		CALL1(interpret, instances[instance].entered)
	}
	current.instance = currentInstance;
}


/*----------------------------------------------------------------------*/
static int getVisits(int location) {
	return getInstanceAttribute(location, VISITSATTRIBUTE);
}


/*----------------------------------------------------------------------*/
static void incrementVisits(int location) {
	setInstanceAttribute(location, VISITSATTRIBUTE, getVisits(location) + 1);
	if (admin[location].location != 0)
		/* Nested location, so increment that too */
		incrementVisits(admin[location].location);
}


/*----------------------------------------------------------------------*/
static void revisited(CONTEXT) {
	if (anyOutput)
		para();
	CALL1(say, where(HERO, DIRECT))
	printMessage(M_AGAIN);
	newline();
	CALL0(describeInstances)
}


/*----------------------------------------------------------------------*/
static bool shouldBeDescribed(void) {
	if (!isPreBeta5(header->version))
		return getVisits(admin[HERO].location) % (current.visits + 1) == 0
		       || admin[admin[HERO].location].visitsCount == 0;
	else
		return admin[admin[HERO].location].visitsCount % (current.visits + 1) == 0;
}


/*----------------------------------------------------------------------*/
static void locateActor(CONTEXT, Aint movingActor, Aint whr) {
	Aint previousCurrentLocation = current.location;
	Aint previousActorLocation = admin[movingActor].location;
	Aint previousActor = current.actor;
	Aint previousInstance = current.instance;

	/* Before leaving, remember that we visited the location */
	if (!isPreBeta5(header->version))
		if (movingActor == (int)HERO)
			incrementVisits(where(HERO, DIRECT));

	/* TODO Actors locating into containers is dubious, anyway as it
	   is now it allows the hero to be located into a container. And what
	   happens with current location if so... */
	if (isAContainer(whr))
		CALL2(locateIntoContainer, movingActor, whr)
	else {
		current.location = whr;
		admin[movingActor].location = whr;
	}

	/* Now we have moved, so show what is needed... */
	current.instance = current.location;

	/* Execute possible entered */
	current.actor = movingActor;
	if (previousActorLocation != current.location) {
		CALL1(executeEntered, current.location)
	}
	current.instance = previousInstance;
	current.actor = previousActor;

	if (movingActor == (int)HERO) {
		if (shouldBeDescribed()) {
			CALL0(look)
		} else {
			CALL0(revisited)
		}
		admin[where(HERO, DIRECT)].visitsCount++;
	} else
		/* Ensure that the location will be described to the hero next time */
		admin[whr].visitsCount = 0;

	if (current.actor != movingActor)
		current.location = previousCurrentLocation;

	current.instance = previousInstance;
}


/*----------------------------------------------------------------------*/
static void traceExtract(CONTEXT, int instance, int containerId, const char *what) {
	if (traceSectionOption) {
		printf("\n<EXTRACT from ");
		traceSay(context, instance);
		printf("[%d, container %d], %s:>\n", instance, containerId, what);
	}
}


/*----------------------------------------------------------------------*/
static void containmentLoopError(CONTEXT, int instance, int whr) {
	ParameterArray parameters = newParameterArray();
	if (isPreBeta4(header->version))
		output("That would be to put something inside itself.");
	else if (whr == instance) {
		addParameterForInstance(parameters, instance);
		printMessageWithParameters(M_CONTAINMENT_LOOP, parameters);
	} else {
		addParameterForInstance(parameters, instance);
		addParameterForInstance(parameters, whr);
		printMessageWithParameters(M_CONTAINMENT_LOOP2, parameters);
	}
	free(parameters);
	CALL1(error, NO_MSG)
}


/*----------------------------------------------------------------------*/
static void runExtractStatements(CONTEXT, int instance, int containerId) {
	ContainerEntry *theContainer = &containers[containerId];

	if (theContainer->extractStatements != 0) {
		CALL3(traceExtract, instance, containerId, "Executing")
		CALL1(interpret, theContainer->extractStatements)
	}
}


/*----------------------------------------------------------------------*/
static bool runExtractChecks(CONTEXT, int instance, int containerId) {
	ContainerEntry *theContainer = &containers[containerId];

	if (theContainer->extractChecks != 0) {
		R0CALL3(traceExtract, instance, containerId, "Checking")
		if (checksFailed(context, theContainer->extractChecks, EXECUTE_CHECK_BODY_ON_FAIL)) {
			fail = TRUE;
			return FALSE;       /* Failed! */
		}
	}
	return TRUE;                /* Passed! */
}


/*======================================================================*/
void locate(CONTEXT, int instance, int whr) {
	int containerId;
	int previousInstance = current.instance;

	verifyInstance(instance, "LOCATE");
	verifyInstance(whr, "LOCATE AT");

	/* Will this create a containment loop? */
	if (whr == instance || (isAContainer(instance) && isIn(whr, instance, TRANSITIVE)))
		CALL2(containmentLoopError, instance, whr)

	/* First check if the instance is in a container, if so run extract checks */
	if (isAContainer(admin[instance].location)) {    /* In something? */
		int loc = admin[instance].location;

		/* Run all nested extraction checks */
		while (isAContainer(loc)) {
			current.instance = loc;
			containerId = instances[loc].container;

			if (!runExtractChecks(context, instance, containerId)) {
				current.instance = previousInstance;
				return;
			}
			runExtractStatements(context, instance, containerId);
			loc = admin[loc].location;
		}
		current.instance = previousInstance;
	}

	if (isAActor(instance)) {
		CALL2(locateActor, instance, whr)
	} else if (isALocation(instance)) {
		locateLocation(instance, whr);
	} else {
		CALL2(locateObject, instance, whr)
	}
	gameStateChanged = TRUE;
}

} // End of namespace Alan3
} // End of namespace Glk
