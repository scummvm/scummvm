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

#include "common/array.h"
#include "glk/alan3/parse.h"
#include "glk/alan3/act.h"
#include "glk/alan3/alt_info.h"
#include "glk/alan3/class.h"
#include "glk/alan3/compatibility.h"
#include "glk/alan3/current.h"
#include "glk/alan3/dictionary.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/inter.h"
#include "glk/jumps.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/literal.h"
#include "glk/alan3/location.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/options.h"
#include "glk/alan3/output.h"
#include "glk/alan3/parameter_position.h"
#include "glk/alan3/syntax.h"
#include "glk/alan3/scan.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/utils.h"
#include "glk/alan3/word.h"

namespace Glk {
namespace Alan3 {

/* PRIVATE TYPES */
/* To remember parameter/pronoun relations */
struct Pronoun {
	int pronoun;
	int instance;
};

/*----------------------------------------------------------------------*/
static void clearPronounList(Pronoun list[]) {
	implementationOfSetEndOfArray((Aword *)list);
}


typedef Aint *(*ReferencesFinder)(int wordIndex);
typedef void (*ParameterParser)(CONTEXT, Parameter parameters[]);



/* PRIVATE DATA */
static Pronoun *pronouns = NULL;


/* Syntax Parameters */
static Parameter *previousMultipleParameters; /* Previous multiple list */


/* For parameters that are literals we need to trick message handling to
 * output the word and create a string literal instance if anyone wants to
 * refer to an attribute of it (literals inherit from entity so application
 * can have added an attribute) */

/*----------------------------------------------------------------------*/
static void addParameterForWord(Parameter *parameters, int wordIndex) {
	Parameter *parameter = findEndOfParameterArray(parameters);

	createStringLiteral((char *)pointerTo(dictionary[playerWords[wordIndex].code].string));
	parameter->instance = instanceFromLiteral(litCount); /* A faked literal */
	parameter->useWords = TRUE;
	parameter->firstWord = parameter->lastWord = wordIndex;
	setEndOfArray(parameter + 1);
}


/*----------------------------------------------------------------------*/
static Pronoun *allocatePronounArray(Pronoun *currentList) {
	if (currentList == NULL)
		currentList = (Pronoun *)allocate(sizeof(Pronoun) * (MAXPARAMS + 1));
	clearPronounList(currentList);
	return currentList;
}


/*----------------------------------------------------------------------*/
static bool endOfWords(int wordIndex) {
	return isEndOfArray(&playerWords[wordIndex]);
}


/*----------------------------------------------------------------------*/
static void handleDirectionalCommand(CONTEXT) {
	currentWordIndex++;
	if (!endOfWords(currentWordIndex) && !isConjunctionWord(currentWordIndex)) {
		CALL1(error, M_WHAT)
	} else {
		CALL2(go, current.location, dictionary[playerWords[currentWordIndex - 1].code].code)
	}

	if (!endOfWords(currentWordIndex))
		currentWordIndex++;
}


/*----------------------------------------------------------------------*/
static void errorWhichOne(CONTEXT, Parameter alternative[]) {
	int p; /* Index into the list of alternatives */
	ParameterArray parameters = newParameterArray();

	parameters[0] = alternative[0];
	setEndOfArray(&parameters[1]);
	printMessageWithParameters(M_WHICH_ONE_START, parameters);
	for (p = 1; !isEndOfArray(&alternative[p + 1]); p++) {
		clearParameterArray(parameters);
		addParameterToParameterArray(parameters, &alternative[p]);
		printMessageWithParameters(M_WHICH_ONE_COMMA, parameters);
	}
	clearParameterArray(parameters);
	addParameterToParameterArray(parameters, &alternative[p]);
	printMessageWithParameters(M_WHICH_ONE_OR, parameters);
	freeParameterArray(parameters);
	CALL0(abortPlayerCommand) /* Return with empty error message */
}

/*----------------------------------------------------------------------*/
static void errorWhichPronoun(CONTEXT, int pronounWordIndex, Parameter alternatives[]) {
	int p; /* Index into the list of alternatives */
	Parameter *messageParameters = newParameterArray();

	addParameterForWord(messageParameters, pronounWordIndex);
	printMessageWithParameters(M_WHICH_PRONOUN_START, messageParameters);

	clearParameterArray(messageParameters);
	addParameterToParameterArray(messageParameters, &alternatives[0]);

	printMessageWithParameters(M_WHICH_PRONOUN_FIRST, messageParameters);

	for (p = 1; !isEndOfArray(&alternatives[p + 1]); p++) {
		clearParameterArray(messageParameters);
		addParameterToParameterArray(messageParameters, &alternatives[p]);
		printMessageWithParameters(M_WHICH_ONE_COMMA, messageParameters);
	}
	clearParameterArray(messageParameters);
	addParameterToParameterArray(messageParameters, &alternatives[p]);
	printMessageWithParameters(M_WHICH_ONE_OR, messageParameters);
	freeParameterArray(messageParameters);
	CALL0(abortPlayerCommand)
}

/*----------------------------------------------------------------------*/
static void errorWhat(CONTEXT, int playerWordIndex) {
	Parameter *messageParameters = newParameterArray();

	addParameterForWord(messageParameters, playerWordIndex);
	printMessageWithParameters(M_WHAT_WORD, messageParameters);
	freeParameterArray(messageParameters);
	CALL0(abortPlayerCommand)
}

/*----------------------------------------------------------------------*/
static void errorAfterExcept(CONTEXT, int butWordIndex) {
	Parameter *messageParameters = newParameterArray();
	addParameterForWord(messageParameters, butWordIndex);
	printMessageWithParameters(M_AFTER_BUT, messageParameters);
	freeParameterArray(messageParameters);
	CALL0(abortPlayerCommand)
}

/*----------------------------------------------------------------------*/
static int fakePlayerWordForAll() {
	/* Look through the dictionary and find any ALL_WORD, then add a
	   player word so that it can be used in the message */
	int p, d;

	for (p = 0; !isEndOfArray(&playerWords[p]); p++)
		;
	setEndOfArray(&playerWords[p + 1]); /* Make room for one more word */
	for (d = 0; d < dictionarySize; d++)
		if (isAll(d)) {
			playerWords[p].code = d;
			return p;
		}
	syserr("No ALLWORD found");
	return 0;
}

/*----------------------------------------------------------------------*/
static void errorButAfterAll(CONTEXT, int butWordIndex) {
	Parameter *messageParameters = newParameterArray();
	addParameterForWord(messageParameters, butWordIndex);
	addParameterForWord(messageParameters, fakePlayerWordForAll());
	printMessageWithParameters(M_BUT_ALL, messageParameters);
	freeParameterArray(messageParameters);
	CALL0(abortPlayerCommand)
}

/*----------------------------------------------------------------------*/
static Aint findInstanceForNoun(int wordIndex) {
	DictionaryEntry *d = &dictionary[wordIndex];
	if (d->nounRefs == 0 || d->nounRefs == EOD)
		syserr("No references for noun");
	return *(Aint *) pointerTo(d->nounRefs);
}

/*----------------------------------------------------------------------*/
static void errorNoSuch(CONTEXT, Parameter parameter) {

	/* If there was no instance, assume the last word used is the noun,
	 * then find any instance with the noun he used */
	if (parameter.instance == (Aid) - 1)
		parameter.instance = 0;
	if (parameter.instance == 0)
		parameter.instance = findInstanceForNoun(playerWords[parameter.lastWord].code);
	parameter.useWords = TRUE; /* Indicate to use words and not names */

	clearParameterArray(globalParameters);
	addParameterToParameterArray(globalParameters, &parameter);
	CALL1(error, M_NO_SUCH)
}

/*----------------------------------------------------------------------*/
static void buildAllHere(CONTEXT, Parameter list[]) {
	uint instance;
	bool found = FALSE;
	int word = list[0].firstWord;

	for (instance = 1; instance <= header->instanceMax; instance++)
		if (isHere(instance, /*FALSE*/ TRANSITIVE)) {
			Parameter *parameter = newParameter(instance);
			addParameterToParameterArray(list, parameter);
			deallocate(parameter);
			found = TRUE;
		}
	if (!found)
		errorWhat(context, word);
}


/*----------------------------------------------------------------------*/
static bool endOfPronouns(int pronounIndex) {
	return isEndOfArray(&pronouns[pronounIndex]);
}


/*----------------------------------------------------------------------*/
static int getPronounInstances(int word, Parameter instanceParameters[]) {
	/* Find the instance that the pronoun word could refer to, return 0
	   if none or multiple */
	int p;
	int instanceCount = 0;

	clearParameterArray(instanceParameters);
	for (p = 0; !endOfPronouns(p); p++)
		if (pronouns[p].instance != 0 && dictionary[word].code == (Aword)pronouns[p].pronoun) {
			instanceParameters[instanceCount].instance = pronouns[p].instance;
			instanceParameters[instanceCount].useWords = FALSE; /* Can't use words since they are gone, pronouns
                                                                   refer to parameters in previous command */
			setEndOfArray(&instanceParameters[++instanceCount]);
		}
	return instanceCount;
}

/*----------------------------------------------------------------------*/
static bool inOpaqueContainer(int originalInstance) {
	int instance = admin[originalInstance].location;

	while (isAContainer(instance)) {
		// TODO : isOpaque()
		if (getInstanceAttribute(instance, OPAQUEATTRIBUTE))
			return TRUE;
		instance = admin[instance].location;
	}
	return FALSE;
}

/*----------------------------------------------------------------------*/
static bool reachable(int instance) {
	if (isA(instance, THING) || isA(instance, LOCATION))
		return isHere(instance, TRANSITIVE) && !inOpaqueContainer(instance);
	else
		return TRUE;
}

/*----------------------------------------------------------------------*/
static Aint *nounReferencesForWord(int wordIndex) {
	return (Aint *) pointerTo(dictionary[playerWords[wordIndex].code].nounRefs);
}


/*----------------------------------------------------------------------*/
static Aint *adjectiveReferencesForWord(int wordIndex) {
	return (Aint *) pointerTo(dictionary[playerWords[wordIndex].code].adjectiveRefs);
}


/*----------------------------------------------------------------------*/
static void parseLiteral(Parameter parameters[]) {
	parameters[0].firstWord = parameters[0].lastWord = currentWordIndex++;
	parameters[0].instance = 0;
	parameters[0].isLiteral = TRUE;
	setEndOfArray(&parameters[1]);
}


/*----------------------------------------------------------------------*/
static void parsePronoun(Parameter parameters[]) {
	parameters[0].firstWord = parameters[0].lastWord = currentWordIndex++;
	parameters[0].instance = 0;
	parameters[0].isPronoun = TRUE;
	setEndOfArray(&parameters[1]);
}


/*----------------------------------------------------------------------*/
static bool anotherAdjective(int wordIndex) {
	return !endOfWords(wordIndex) && isAdjectiveWord(wordIndex);
}


/*----------------------------------------------------------------------*/
static bool lastPossibleNoun(int wordIndex) {
	return isNounWord(wordIndex) && (endOfWords(wordIndex + 1) || !isNounWord(wordIndex + 1));
}


/*----------------------------------------------------------------------*/
static void updateWithReferences(Parameter result[], int wordIndex, Aint * (*referenceFinder)(int wordIndex)) {
	static Parameter *references = NULL; /* Instances referenced by a word */
	references = ensureParameterArrayAllocated(references);

	copyReferencesToParameterArray(referenceFinder(wordIndex), references);
	if (lengthOfParameterArray(result) == 0)
		copyParameterArray(result, references);
	else
		intersectParameterArrays(result, references);
}


/*----------------------------------------------------------------------*/
static void filterOutNonReachable(Parameter filteredCandidates[], bool (*reachable)(int)) {
	int i;
	for (i = 0; !isEndOfArray(&filteredCandidates[i]); i++)
		if (!reachable(filteredCandidates[i].instance))
			filteredCandidates[i].instance = 0;
	compressParameterArray(filteredCandidates);
}


/*
 * There are various ways the player can refer to things, some are
 * explicit, in which case they should be kept in the input. If he said
 * 'them', 'all' or some such the list is inferred so we must filter
 * it w.r.t what it can mean. A special case is when he said 'the ball'
 * and there is only one ball here, but multiple in the game.  We need
 * to be able to distinguish between these cases!!!  'them' is
 * explicit, 'all' is inferred, exceptions can never be inferred,
 * maybe 'all' is the only inferred?
 */

/*----------------------------------------------------------------------*/
static void disambiguateCandidatesForPosition(CONTEXT, ParameterPosition parameterPositions[],
		int position, Parameter candidates[]) {
	int i;
	Parameter *parameters = newParameterArray();

	convertPositionsToParameters(parameterPositions, parameters);
	for (i = 0; !isEndOfArray(&candidates[i]); i++) {
		if (candidates[i].instance != 0) { /* Already empty? */
			copyParameter(&parameters[position], &candidates[i]);
			// DISAMBIGUATION!!
			if (!reachable(candidates[i].instance)) {
				// Then remove this candidate from list
				candidates[i].instance = 0;
			} else {
				bool flag;
				FUNC3(possible, flag, current.verb, parameters, parameterPositions)
				if (!flag)
					candidates[i].instance = 0;
			}
		}
	}
	compressParameterArray(candidates);
	freeParameterArray(parameters);
}


/*----------------------------------------------------------------------*/
static bool parseAnyAdjectives(Parameter parameters[]) {
	bool adjectiveOrNounFound = FALSE;
	while (anotherAdjective(currentWordIndex)) {
		if (lastPossibleNoun(currentWordIndex))
			break;
		adjectiveOrNounFound = TRUE;
		currentWordIndex++;
	}
	return adjectiveOrNounFound;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Parse the input and note the word indices in the parameters,
   matching will be done by the match* functions */
static void parseAdjectivesAndNoun(CONTEXT, Parameter parameters[]) {
	int wordFirst, wordLast;
	bool adjectiveOrNounFound = FALSE;

	wordFirst = currentWordIndex;

	adjectiveOrNounFound = parseAnyAdjectives(parameters);

	if (!endOfWords(currentWordIndex)) {
		if (isNounWord(currentWordIndex)) {
			adjectiveOrNounFound = TRUE;
			currentWordIndex++;
		} else
			CALL1(error, M_NOUN)
	} else if (adjectiveOrNounFound) {
		/* Perhaps the last word could also be interpreted as a noun? */
		if (isNounWord(currentWordIndex - 1)) {
			// TODO When does this get executed? Maybe if conjunctions can be nouns? Or nouns be adjectives?
			printf("DEBUG: When does this get executed? Found adjective or Noun and the previous word could also be a noun...\n");
		} else
			CALL1(error, M_NOUN)
	}

	if (adjectiveOrNounFound) {
		wordLast = currentWordIndex - 1;

		parameters[0].firstWord = wordFirst;
		parameters[0].lastWord = wordLast;
		parameters[0].instance = 0; /* No instance yet! */
		setEndOfArray(&parameters[1]);
	} else
		setEndOfArray(&parameters[0]);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void parseReference(CONTEXT, Parameter parameters[]) {
	clearParameterArray(parameters);

	if (isLiteralWord(currentWordIndex)) {
		parseLiteral(parameters);
	} else if (isPronounWord(currentWordIndex)) {
		parsePronoun(parameters);
	} else {
		CALL1(parseAdjectivesAndNoun, parameters)
	}
}


/*----------------------------------------------------------------------*/
static void getPreviousMultipleParameters(Parameter parameters[]) {
	int i;
	for (i = 0; !isEndOfArray(&previousMultipleParameters[i]); i++) {
		parameters[i].candidates = ensureParameterArrayAllocated(parameters[i].candidates);
		setEndOfArray(&parameters[i].candidates[0]); /* No candidates */
		if (!reachable(previousMultipleParameters[i].instance))
			parameters[i].instance = 0;
		else
			parameters[i].instance = previousMultipleParameters[i].instance;
	}
	setEndOfArray(&parameters[i]);
	compressParameterArray(parameters);
}


/*----------------------------------------------------------------------*/
static void parseReferenceToPreviousMultipleParameters(Parameter parameters[]) {
	parameters[0].firstWord = parameters[0].lastWord = currentWordIndex++;
	parameters[0].instance = 0;
	parameters[0].isThem = TRUE;
	setEndOfArray(&parameters[1]);
}


/*----------------------------------------------------------------------*/
static bool parseOneParameter(CONTEXT, Parameter parameters[], int parameterIndex) {
	Parameter *parameter = newParameterArray();

	// TODO Maybe this should go in the complex()?
	if (isThemWord(currentWordIndex) && (!isPronounWord(currentWordIndex) ||
	                                     (isPronounWord(currentWordIndex) && lengthOfParameterArray(previousMultipleParameters) > 0))) {
		// "them" is also a common pronoun for some instances, but if there
		// are previous multiple parameters we give precedence to those
		parseReferenceToPreviousMultipleParameters(parameter);
	} else {
		R0CALL1(parseReference, parameter)
		if (lengthOfParameterArray(parameter) == 0) { /* Failed to find any exceptions! */
			freeParameterArray(parameter);
			return FALSE;
		}
	}

	/* Add the one we found to the parameters */
	parameters[parameterIndex] = parameter[0];
	setEndOfArray(&parameters[parameterIndex + 1]);
	freeParameterArray(parameter);
	return TRUE;
}


/*
 * A "simple" parameter is in one of three forms:
 *
 * 1) adjectives and nouns referencing a single instance (might
 * actually match multiple instances in the game...)
 *
 * 2) multiple of 1) separated by conjunctions ("a and b and c")
 *
 * 3) a pronoun referencing a single instance

 * We also need to handle "them" here since it is also a common
 * pronoun for some instances, e.g. scissor, trouser, money,
 * glasses, but it can also refer to the set of instances from the
 * previous command. If it is a pronoun but there are also multiple
 * parameter from the previous command, we give precedence to the
 * multiple previous.
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void simpleParameterParser(CONTEXT, Parameter parameters[]) {
	bool flag;

	/* This will loop until all references are collected (typically "a and b and c") */
	int parameterIndex;
	for (parameterIndex = 0;; parameterIndex++) {
		FUNC2(parseOneParameter, flag, parameters, parameterIndex)
		if (!flag)
			return;

		if (!endOfWords(currentWordIndex)
		        && (isConjunctionWord(currentWordIndex) && (isAdjectiveWord(currentWordIndex + 1)
		                || isNounWord(currentWordIndex + 1)))) {
			/* Since this is a conjunction and the next seems to be another instance reference,
			   let's continue with that by eating the conjunction */
			currentWordIndex++;
		} else {
			return;
		}
	}
}


/*----------------------------------------------------------------------*/
static void parseExceptions(CONTEXT, ParameterPosition *parameterPosition, ParameterParser simpleParameterParser) {
	int exceptWordIndex = currentWordIndex;
	currentWordIndex++;
	parameterPosition->exceptions = ensureParameterArrayAllocated(parameterPosition->exceptions);
	CALL1(simpleParameterParser, parameterPosition->exceptions)
	
	if (lengthOfParameterArray(parameterPosition->exceptions) == 0)
		errorAfterExcept(context, exceptWordIndex);
}


/*
 * Complex instance references are of the form:
 *
 * 1) all
 *
 * 2) all except
 *
 * 3) a simple (see above) instance reference(s)
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void complexParameterParserDelegate(CONTEXT, ParameterPosition *parameterPosition,
		ParameterParser simpleParameterParser) {
	parameterPosition->parameters = ensureParameterArrayAllocated(parameterPosition->parameters);

	parameterPosition->all = FALSE;
	parameterPosition->them = FALSE;
	parameterPosition->explicitMultiple = FALSE;

	if (isAllWord(currentWordIndex)) {
		parameterPosition->all = TRUE;
		parameterPosition->explicitMultiple = TRUE;
		parameterPosition->parameters[0].firstWord = currentWordIndex;
		parameterPosition->parameters[0].lastWord = currentWordIndex;
		currentWordIndex++;
		parameterPosition->parameters[0].instance = 0;
		setEndOfArray(&parameterPosition->parameters[1]);
		if (!endOfWords(currentWordIndex) && isExceptWord(currentWordIndex))
			CALL2(parseExceptions, parameterPosition, simpleParameterParser)
	} else {
		CALL1(simpleParameterParser, parameterPosition->parameters)
		if (lengthOfParameterArray(parameterPosition->parameters) > 1)
			parameterPosition->explicitMultiple = TRUE;
	}

}

/*----------------------------------------------------------------------*/
static void complexReferencesParser(CONTEXT, ParameterPosition *parameterPosition) {
	complexParameterParserDelegate(context, parameterPosition, simpleParameterParser);
}



/*----------------------------------------------------------------------*/
static char *classNameAndId(int classId) {
	static char buffer[1000] = "";

	if (classId != -1)
		sprintf(buffer, "%s[%d]", idOfClass(classId), classId);
	else
		sprintf(buffer, "Container");
	return buffer;
}


/*----------------------------------------------------------------------*/
static char *parameterNumberAndName(int parameterNumber) {
	static char buffer[1000] = "";
	/* HERE SHOULD BE current.syntax */
	char *parameterName = parameterNameInSyntax(current.syntax, parameterNumber);

	if (parameterName != NULL)
		sprintf(buffer, "%s(#%d)", parameterName, parameterNumber);
	else
		sprintf(buffer, "#%d", parameterNumber);
	return buffer;
}


/*----------------------------------------------------------------------*/
static void traceRestriction(RestrictionEntry *restriction, int classId, bool condition) {
	printf("\n<SYNTAX RESTRICTION WHERE parameter %s Isa %s, %s>\n",
	       parameterNumberAndName(restriction->parameterNumber),
	       classNameAndId(classId), condition ? "PASSED" : "FAILED:");
}


/*----------------------------------------------------------------------*/
static bool restrictionCheck(RestrictionEntry *restriction, int instance) {
	if (restriction->_class == RESTRICTIONCLASS_CONTAINER) {
		if (traceSectionOption)
			traceRestriction(restriction, -1, isAContainer(instance));
		return isAContainer(instance);
	} else {
		if (traceSectionOption)
			traceRestriction(restriction, restriction->_class, isA(instance, restriction->_class));
		return isA(instance, restriction->_class);
	}
}


/*----------------------------------------------------------------------*/
static void runRestriction(CONTEXT, RestrictionEntry *restriction, Parameter parameters[]) {
	if (restriction->stms) {
		setGlobalParameters(parameters);
		interpret(context, restriction->stms);
	} else {
		error(context, M_CANT0);
	}
}


/*----------------------------------------------------------------------*/
static int remapParameterOrder(int syntaxNumber, ParameterPosition parameterPositions[]) {
	/* Find the syntax map, use the verb code from it and remap the parameters */
	ParameterMapEntry *parameterMapTable;
	Aword *parameterMap;
	Aint parameterNumber;
	Common::Array<ParameterPosition> savedParameterPositions;
	savedParameterPositions.resize(MAXPARAMS + 1);

	for (parameterMapTable = (ParameterMapEntry *)pointerTo(header->parameterMapAddress); !isEndOfArray(parameterMapTable); parameterMapTable++)
		if (parameterMapTable->syntaxNumber == syntaxNumber)
			break;
	if (isEndOfArray(parameterMapTable))
		syserr("Could not find syntax in mapping table.");

	parameterMap = (Aword *)pointerTo(parameterMapTable->parameterMapping);

	copyParameterPositions(parameterPositions, &savedParameterPositions[0]);

	for (parameterNumber = 1; !savedParameterPositions[parameterNumber - 1].endOfList; parameterNumber++) {
		parameterPositions[parameterNumber - 1] = savedParameterPositions[parameterMap[parameterNumber - 1] - 1];
	}

	return parameterMapTable->verbCode;
}


/*----------------------------------------------------------------------*/
static bool hasBit(Aword flags, Aint bit) {
	return (flags & bit) != 0;
}

/*----------------------------------------------------------------------*/
static bool multipleAllowed(Aword flags) {
	return hasBit(flags, MULTIPLEBIT);
}



/*
 * There are a number of ways that the number of parameters might
 * be more than one:
 *
 * 1) Player used ALL and it matched more than one
 *
 * 2) Player explicitly refered to multiple objects
 *
 * 3) Player did a single (or multiple) reference that was ambiguous
 * in which case we need to disambiguate it (them). If we want to do
 * this after complete parsing we must be able to see the possible
 * parameters for each of these references, e.g.:
 *
 * > take the vase and the book
 *
 * In this case it is a single parameterPosition, but multiple
 * explicit references and each of those might match multiple
 * instances in the game.
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void parseParameterPosition(CONTEXT, ParameterPosition *parameterPosition,
		Aword flags, void (*complexReferencesParser)(CONTEXT, ParameterPosition *parameterPosition)) {
	parameterPosition->parameters = ensureParameterArrayAllocated(parameterPosition->parameters);

	CALL1(complexReferencesParser, parameterPosition)
	if (lengthOfParameterArray(parameterPosition->parameters) == 0) /* No object!? */
		CALL1(error, M_WHAT)

	if (parameterPosition->explicitMultiple && !multipleAllowed(flags))
		CALL1(error, M_MULTIPLE)
}

/*----------------------------------------------------------------------*/
static ElementEntry *elementForParameter(ElementEntry *elms) {
	/* Require a parameter if elms->code == 0! */
	while (!isEndOfArray(elms) && elms->code != 0)
		elms++;
	if (isEndOfArray(elms))
		return NULL;
	return elms;
}

/*----------------------------------------------------------------------*/
static ElementEntry *elementForEndOfSyntax(ElementEntry *elms) {
	while (!isEndOfArray(elms) && (Aword)elms->code != EOS)
		elms++;
	if (isEndOfArray(elms)) /* No match for EOS! */
		return NULL;
	return elms;
}

/*----------------------------------------------------------------------*/
static ElementEntry *elementForWord(ElementEntry *elms, Aint wordCode) {
	while (!isEndOfArray(elms) && elms->code != wordCode)
		elms++;
	if (isEndOfArray(elms))
		return NULL;
	return elms;
}


/*----------------------------------------------------------------------*/
static bool isInstanceReferenceWord(int wordIndex) {
	return isNounWord(wordIndex) || isAdjectiveWord(wordIndex) || isAllWord(wordIndex)
	       || isLiteralWord(wordIndex) || isItWord(wordIndex) || isThemWord(wordIndex) || isPronounWord(wordIndex);
}


/*----------------------------------------------------------------------*/
static bool endOfPlayerCommand(int wordIndex) {
	return endOfWords(wordIndex) || isConjunctionWord(wordIndex);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static ElementEntry *parseInputAccordingToSyntax(CONTEXT, SyntaxEntry *syntax, ParameterPosition parameterPositions[]) {
	ElementEntry *currentElement = elementTreeOf(syntax);
	ElementEntry *nextElement = currentElement;

	int parameterCount = 0;
	while (nextElement != NULL) {
		/* Traverse the possible branches of currentElement to find a match, let the actual input control what we look for */
		parameterPositions[parameterCount].endOfList = TRUE;

		if (endOfPlayerCommand(currentWordIndex)) {
			// TODO If a conjunction word is also some other type of word, like noun? What happens?
			return elementForEndOfSyntax(currentElement);
		}

		/* Or an instance reference ? */
		if (isInstanceReferenceWord(currentWordIndex)) {
			/* If so, save word info for this parameterPosition */
			nextElement = elementForParameter(currentElement);
			if (nextElement != NULL) {
				// Create parameter structure for the parameter position based on player words
				// but without resolving them
				ParameterPosition *parameterPosition = &parameterPositions[parameterCount];
				R0CALL3(parseParameterPosition, parameterPosition, nextElement->flags, complexReferencesParser)

				parameterPosition->flags = nextElement->flags;
				parameterPosition->endOfList = FALSE;

				currentElement = (ElementEntry *) pointerTo(nextElement->next);
				parameterCount++;
				continue;
			}
		}

		/* Or maybe preposition? */
		if (isPrepositionWord(currentWordIndex) || isVerbWord(currentWordIndex)) {
			/* A preposition? Or rather, an intermediate word? */
			nextElement = elementForWord(currentElement, dictionary[playerWords[currentWordIndex].code].code);
			if (nextElement != NULL) {
				currentWordIndex++; /* Word matched, go to next */
				currentElement = (ElementEntry *) pointerTo(nextElement->next);
				continue;
			}
		}

		/* Anything else is an error, but we'll handle 'but' specially here */
		if (isExceptWord(currentWordIndex))
			R0CALL1(errorButAfterAll, currentWordIndex)

		/* If we get here we couldn't match anything... */
		nextElement = NULL;
	}
	return NULL;
}


/*----------------------------------------------------------------------*/
static bool anyExplicitMultiple(ParameterPosition parameterPositions[]) {
	int i;

	for (i = 0; !parameterPositions[i].endOfList; i++)
		if (parameterPositions[i].explicitMultiple)
			return TRUE;
	return FALSE;
}


/*----------------------------------------------------------------------*/
static bool anyAll(ParameterPosition parameterPositions[]) {
	int i;

	for (i = 0; !parameterPositions[i].endOfList; i++)
		if (parameterPositions[i].all)
			return TRUE;
	return FALSE;
}


/*----------------------------------------------------------------------*/
static void checkRestrictedParameters(CONTEXT, ParameterPosition parameterPositions[], ElementEntry elms[]) {
	RestrictionEntry *restriction;
	static Parameter *localParameters = NULL;
	int i;

	localParameters = ensureParameterArrayAllocated(localParameters);
	clearParameterArray(localParameters);

	for (i = 0; !parameterPositions[i].endOfList; i++)
		addParameterToParameterArray(localParameters, &parameterPositions[i].parameters[0]);

	for (restriction = (RestrictionEntry *) pointerTo(elms->next); !isEndOfArray(restriction); restriction++) {
		ParameterPosition *parameterPosition = &parameterPositions[restriction->parameterNumber - 1];
		if (parameterPosition->explicitMultiple) {
			/* This was a multiple parameter position, so check all multipleCandidates */
			for (i = 0; !isEndOfArray(&parameterPosition->parameters[i]); i++) {
				copyParameter(&localParameters[restriction->parameterNumber - 1], &parameterPosition->parameters[i]);
				if (!restrictionCheck(restriction, parameterPosition->parameters[i].instance)) {
					/* Multiple could be both an explicit list of instance references and an expansion of ALL */
					if (!parameterPosition->all) {
						char marker[80];
						/* It wasn't ALL, we need to say something about it, so
						 * prepare a printout with $1/2/3
						 */
						sprintf(marker, "($%ld)", (unsigned long) restriction->parameterNumber);
						setGlobalParameters(localParameters);
						output(marker);
						CALL2(runRestriction, restriction, localParameters)
						para();
					}
					parameterPosition->parameters[i].instance = 0; /* In any case remove it from the list */
				}
			}
		} else {
			if (!restrictionCheck(restriction, parameterPosition->parameters[0].instance)) {
				CALL2(runRestriction, restriction, localParameters)
				CALL0(abortPlayerCommand)
			}
		}
		parameterPositions[restriction->parameterNumber - 1].checked = TRUE;
	}
	freeParameterArray(localParameters);
	localParameters = NULL;
}


/*----------------------------------------------------------------------*/
static void impossibleWith(CONTEXT, ParameterPosition parameterPositions[], int positionIndex) {
	if (isPreBeta2(header->version)) {
		error(context, M_CANT0);
	} else {
		printMessageWithInstanceParameter(M_IMPOSSIBLE_WITH, parameterPositions[positionIndex].parameters[0].instance);
		error(context, NO_MSG);
	}
}



/*----------------------------------------------------------------------*/
static void checkNonRestrictedParameters(CONTEXT, ParameterPosition parameterPositions[]) {
	int positionIndex;
	for (positionIndex = 0; !parameterPositions[positionIndex].endOfList; positionIndex++)
		if (!parameterPositions[positionIndex].checked) {
			if (parameterPositions[positionIndex].explicitMultiple) {
				/* This was a multiple parameter position, check all multiple candidates and remove failing */
				int i;
				for (i = 0; !isEndOfArray(&parameterPositions[positionIndex].parameters[i]); i++)
					if (parameterPositions[positionIndex].parameters[i].instance != 0) /* Skip any empty slots */
						if (!isAObject(parameterPositions[positionIndex].parameters[i].instance))
							parameterPositions[positionIndex].parameters[i].instance = 0;
			} else if (!isAObject(parameterPositions[positionIndex].parameters[0].instance))
				impossibleWith(context, parameterPositions, positionIndex);
		}
}


/*----------------------------------------------------------------------*/
static void restrictParametersAccordingToSyntax(CONTEXT, ParameterPosition parameterPositions[], ElementEntry *elms) {
	uncheckAllParameterPositions(parameterPositions);
	CALL2(checkRestrictedParameters, parameterPositions, elms)
	CALL1(checkNonRestrictedParameters, parameterPositions)

	int positionIndex;
	for (positionIndex = 0; !parameterPositions[positionIndex].endOfList; positionIndex++)
		compressParameterArray(parameterPositions[positionIndex].parameters);
}


/*----------------------------------------------------------------------*/
static void matchPronoun(CONTEXT, Parameter *parameter) {
	static Parameter *pronounInstances = NULL;
	pronounInstances = ensureParameterArrayAllocated(pronounInstances);

	int pronounCandidateCount = getPronounInstances(playerWords[parameter->firstWord].code, pronounInstances);
	if (pronounCandidateCount == 0) {
		CALL1(errorWhat, parameter->firstWord)
	} else if (pronounCandidateCount > 1) {
		CALL2(errorWhichPronoun, parameter->firstWord, pronounInstances)
	} else {
		parameter->candidates[0] = pronounInstances[0];
		setEndOfArray(&parameter->candidates[1]);
	}
}


/*----------------------------------------------------------------------*/
static void matchNounPhrase(Parameter *parameter, ReferencesFinder adjectiveReferencesFinder, ReferencesFinder nounReferencesFinder) {
	int i;

	for (i = parameter->firstWord; i < parameter->lastWord; i++)
		updateWithReferences(parameter->candidates, i, adjectiveReferencesFinder);
	updateWithReferences(parameter->candidates, parameter->lastWord, nounReferencesFinder);
}


/*----------------------------------------------------------------------*/
static void instanceMatcher(CONTEXT, Parameter *parameter) {
	Parameter *candidates = parameter->candidates;
	int i;

	if (parameter->isLiteral) {
		candidates[0].instance = instanceFromLiteral(playerWords[parameter->firstWord].code - dictionarySize);
		setEndOfArray(&candidates[1]);
	} else if (parameter->isPronoun) {
		CALL1(matchPronoun, parameter)
	} else
		matchNounPhrase(parameter, adjectiveReferencesForWord, nounReferencesForWord);

	// Ensure that every candidate have the words, even if there where no candidates
	candidates[0].firstWord = parameter->firstWord;
	candidates[0].lastWord = parameter->lastWord;
	for (i = 0; i < lengthOfParameterArray(candidates); i++) {
		candidates[i].firstWord = parameter->firstWord;
		candidates[i].lastWord = parameter->lastWord;
	}
}


/*----------------------------------------------------------------------*/
static void findCandidates(CONTEXT, Parameter parameters[], void (*instanceMatcher)(CONTEXT, Parameter *parameter)) {
	int i;

	for (i = 0; i < lengthOfParameterArray(parameters); i++) {
		parameters[i].candidates = ensureParameterArrayAllocated(parameters[i].candidates);
		CALL1(instanceMatcher, &parameters[i])

		parameters[i].candidates[0].isPronoun = parameters[i].isPronoun;
	}
}


/*----------------------------------------------------------------------*/
static void handleFailedParse(CONTEXT, ElementEntry *elms) {
	if (elms == NULL)
		error(context, M_WHAT);
	else if (elms->next == 0) { /* No verb code, verb not declared! */
		/* TODO Does this ever happen? */
		error(context, M_CANT0);
	}
}


/*----------------------------------------------------------------------*/
static void convertMultipleCandidatesToMultipleParameters(ParameterPosition parameterPositions[], Parameter multipleParameters[]) {
	int parameterIndex;
	for (parameterIndex = 0; !parameterPositions[parameterIndex].endOfList; parameterIndex++)
		if (parameterPositions[parameterIndex].explicitMultiple) {
			compressParameterArray(parameterPositions[parameterIndex].parameters);
			copyParameterArray(multipleParameters, parameterPositions[parameterIndex].parameters);
		}
}


/*----------------------------------------------------------------------*/
static ElementEntry *parseInput(CONTEXT, ParameterPosition *parameterPositions) {
	ElementEntry *element;
	SyntaxEntry *stx;

	R0FUNC1(findSyntaxTreeForVerb, stx, verbWordCode)
	R0FUNC2(parseInputAccordingToSyntax, element, stx, parameterPositions)
	R0CALL1(handleFailedParse, element)

	current.syntax = element->flags;
	current.verb = remapParameterOrder(current.syntax, parameterPositions);
	return element;
}


/*----------------------------------------------------------------------*/
static void findCandidatesForPlayerWords(CONTEXT, ParameterPosition *parameterPosition) {
	ParameterArray parameters = parameterPosition->parameters;

	if (!parameterArrayIsEmpty(parameters)) {
		if (parameters[0].isThem) {
			parameterPosition->them = TRUE;
			getPreviousMultipleParameters(parameters);
			if (lengthOfParameterArray(parameters) == 0)
				CALL1(errorWhat, parameters[0].firstWord)
			if (lengthOfParameterArray(parameters) > 1)
				parameterPosition->explicitMultiple = TRUE;
		} else if (parameterPosition->all) {
			CALL1(buildAllHere, parameters)
			if (!parameterArrayIsEmpty(parameterPosition->exceptions))
				CALL2(findCandidates, parameterPosition->exceptions, instanceMatcher)
		} else
			CALL2(findCandidates, parameters, instanceMatcher)
	}
}


/*----------------------------------------------------------------------*/
static void handleMultiplePosition(CONTEXT, ParameterPosition parameterPositions[]) {
	int multiplePosition = findMultipleParameterPosition(parameterPositions);
	if (anyAll(parameterPositions)) {
		/* If the player used ALL, try to find out what was applicable */
		CALL3(disambiguateCandidatesForPosition, parameterPositions, multiplePosition,
			parameterPositions[multiplePosition].parameters)
		
		if (lengthOfParameterArray(parameterPositions[multiplePosition].parameters) == 0)
			CALL1(errorWhat, parameterPositions[multiplePosition].parameters[0].firstWord)

		subtractParameterArrays(parameterPositions[multiplePosition].parameters, parameterPositions[multiplePosition].exceptions);
		if (lengthOfParameterArray(parameterPositions[multiplePosition].parameters) == 0)
			CALL1(error, M_NOT_MUCH)
	} else if (anyExplicitMultiple(parameterPositions)) {
		compressParameterArray(parameterPositions[multiplePosition].parameters);
		if (lengthOfParameterArray(parameterPositions[multiplePosition].parameters) == 0) {
			/* If there where multiple parameters but non left, exit without a */
			/* word, assuming we have already said enough */
			CALL0(abortPlayerCommand)
		}
	}
}


/*
 * Disambiguation is hard: there are a couple of different cases that
 * we want to handle: Omnipotent parameter position, multiple present
 * and non-present objects etc. The following table will show which
 * message we would like to give in the various situations.
 *
 * p = present, n = non-present, 1 = single, m = multiple
 * (1p1n = single present, single non-present)
 *
 * p, n, omni,  result,                 why?
 * -----------------------------------------------------------------
 * 0, 0, no,    errorNoSuch(w)/errorWhat(w)
 * 0, 1, no,    errorNoSuch(w)/errorWhat(w)
 * 0, m, no,    errorNoSuch(w)/errorWhat(w)
 * 1, 0, no,    ok(p)
 * 1, 1, no,    ok(p)
 * 1, m, no,    ok(p)
 * m, 0, no,    errorWhichOne(p)
 * m, 1, no,    errorWhichOne(p)    only present objects should be revealed
 * m, m, no,    errorWhichOne(p)    d:o

 * 0, 0, yes,   errorNoSuch(w)
 * 0, 1, yes,   ok(n)
 * 0, m, yes,   errorWhichOne(n)    already looking "beyond" presence, might reveal undiscovered distant objects
 * 1, 0, yes,   ok(p)
 * 1, 1, yes,   ok(p)               present objects have priority
 * 1, m, yes,   ok(p)               present objects have priority
 * m, 0, yes,   errorWhichOne(p)
 * m, 1, yes,   errorWhichOne(p)    present objects have priority, but only list present
 * m, m, yes,   errorWhichOne(p)    present objects have priority, but only list present
 */


typedef Parameter *DisambiguationHandler(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]);
typedef DisambiguationHandler *DisambiguationHandlerTable[3][3][2];

static Parameter *disambiguate00N(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	if (allCandidates[0].isPronoun)
		R0CALL1(errorWhat, allCandidates[0].firstWord)
	else
		R0CALL1(errorNoSuch, allCandidates[0]);
	return NULL;
}

static Parameter *disambiguate01N(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	if (allCandidates[0].isPronoun)
		R0CALL1(errorWhat, allCandidates[0].firstWord)
	else
		R0CALL1(errorNoSuch, allCandidates[0])
	return NULL;
}

static Parameter *disambiguate0MN(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	if (allCandidates[0].isPronoun)
		R0CALL1(errorWhat, allCandidates[0].firstWord)
	else
		R0CALL1(errorNoSuch, allCandidates[0])
	return NULL;
}

static Parameter *disambiguate10N(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	return presentCandidates;
}

static Parameter *disambiguate11N(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	return presentCandidates;
}

static Parameter *disambiguate1MN(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	return presentCandidates;
}
static Parameter *disambiguateM0N(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	R0CALL1(errorWhichOne, presentCandidates)
	return NULL;
}

static Parameter *disambiguateM1N(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	R0CALL1(errorWhichOne, presentCandidates)
	return NULL;
}

static Parameter *disambiguateMMN(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	R0CALL1(errorWhichOne, presentCandidates)
	return NULL;
}

static Parameter *disambiguate00Y(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	errorNoSuch(context, allCandidates[0]);
	return NULL;
}
static Parameter *disambiguate01Y(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	return allCandidates;
}
static Parameter *disambiguate0MY(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	R0CALL1(errorWhichOne, allCandidates)
	return NULL;
}
static Parameter *disambiguate10Y(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	return presentCandidates;
}
static Parameter *disambiguate11Y(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	return presentCandidates;
}
static Parameter *disambiguate1MY(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	return presentCandidates;
}
static Parameter *disambiguateM0Y(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	R0CALL1(errorWhichOne, presentCandidates)
	return NULL;
}
static Parameter *disambiguateM1Y(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	R0CALL1(errorWhichOne, presentCandidates)
	return NULL;
}
static Parameter *disambiguateMMY(CONTEXT, Parameter allCandidates[], Parameter presentCandidates[]) {
	R0CALL1(errorWhichOne, presentCandidates)
	return NULL;
}

static DisambiguationHandlerTable disambiguationHandlerTable = {
	{
		// Present == 0
		{
			// Distant == 0
			disambiguate00N, disambiguate00Y
		},
		{
			// Distant == 1
			disambiguate01N, disambiguate01Y
		},
		{
			// Distant == M
			disambiguate0MN, disambiguate0MY
		}
	},
	{
		//  Present == 1
		{
			// Distant == 0
			disambiguate10N, disambiguate10Y
		},
		{
			// Distant == 1
			disambiguate11N, disambiguate11Y
		},
		{
			// Distant == M
			disambiguate1MN, disambiguate1MY
		}
	},
	{
		// Present == M
		{
			// Distant == 0
			disambiguateM0N, disambiguateM0Y
		},
		{
			// Distant == 1
			disambiguateM1N, disambiguateM1Y
		},
		{
			// Distant == M
			disambiguateMMN, disambiguateMMY
		}
	}
};

/*----------------------------------------------------------------------*/
static void disambiguateCandidates(CONTEXT, Parameter *allCandidates, bool omnipotent, bool (*reachable)(int), DisambiguationHandlerTable handler) {
	static Parameter *presentCandidates = NULL;
	int present;
	int distant;
	Parameter *result;

	presentCandidates = ensureParameterArrayAllocated(presentCandidates);

	copyParameterArray(presentCandidates, allCandidates);
	filterOutNonReachable(presentCandidates, reachable);

	present = lengthOfParameterArray(presentCandidates);
	if (present > 1) present = 2; /* 2 = M */

	distant = lengthOfParameterArray(allCandidates) - present;
	if (distant > 1) distant = 2; /* 2 = M */

	FUNC2(handler[present][distant][omnipotent], result, allCandidates, presentCandidates)

	// If we returned then it's ok, use the single candidate found
	allCandidates[0] = result[0];
	setEndOfArray(&allCandidates[1]);
}


/*----------------------------------------------------------------------*/
static void disambiguate(CONTEXT, ParameterPosition parameterPositions[], ElementEntry *element) {
	/* The New Strategy! Parsing has only collected word indications,
	   not built anything, so we need to match parameters to instances here */

	int position;
	for (position = 0; !parameterPositions[position].endOfList; position++) {
		CALL1(findCandidatesForPlayerWords, &parameterPositions[position])
	}

	/* Now we have candidates for everything the player said, except
	   if he used ALL or THEM, then we have built those as parameters, or he
	   referred to the multiple parameters of the previous command
	   using 'them, if so, they too are stored as parameters */

	for (position = 0; !parameterPositions[position].endOfList; position++) {
		ParameterPosition *parameterPosition = &parameterPositions[position];
		bool omni = hasBit(parameterPosition->flags, OMNIBIT);
		if (!parameterPosition->all && !parameterPosition->them) {
			Parameter *parameters = parameterPosition->parameters;
			int p;
			for (p = 0; p < lengthOfParameterArray(parameters); p++) {
				Parameter *parameter = &parameters[p];
				Parameter *candidates = parameter->candidates;
				CALL4(disambiguateCandidates, candidates, omni, reachable, disambiguationHandlerTable)
				
				parameter->instance = candidates[0].instance;
			}
		}
		if (parameterPosition->all) {
			Parameter *exceptions = parameterPosition->exceptions;
			int p;
			for (p = 0; p < lengthOfParameterArray(exceptions); p++) {
				Parameter *parameter = &exceptions[p];
				Parameter *candidates = parameter->candidates;
				CALL4(disambiguateCandidates, candidates, omni, reachable, disambiguationHandlerTable)

				parameter->instance = candidates[0].instance;
			}
		}
	}

	CALL1(handleMultiplePosition, parameterPositions)

	/* Now perform restriction checks */
	CALL2(restrictParametersAccordingToSyntax, parameterPositions, element)
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void tryParam(CONTEXT, Parameter parameters[], Parameter multipleParameters[]) {
	ElementEntry *element;      /* Pointer to element list */
	static ParameterPosition *parameterPositions = NULL;
	if (parameterPositions != NULL)
		deallocateParameterPositions(parameterPositions);

	// TODO newParameterPositionArray()!!!! Or even reallocatePP.. or cleanPP..
	parameterPositions = (ParameterPosition *)allocate(sizeof(ParameterPosition) * (MAXPARAMS + 1));
	parameterPositions[0].endOfList = TRUE;

	FUNC1(parseInput, element, parameterPositions)
	CALL2(disambiguate, parameterPositions, element)

	// TODO: Now we need to convert back to legacy parameter and multipleParameter format
	convertPositionsToParameters(parameterPositions, parameters);
	markExplicitMultiple(parameterPositions, parameters);
	convertMultipleCandidatesToMultipleParameters(parameterPositions, multipleParameters);

	deallocateParameterPositions(parameterPositions);
	parameterPositions = NULL;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void parseOneCommand(CONTEXT, Parameter parameters[], Parameter multipleParameters[]) {
	// ... to understand what he said
	CALL2(tryParam, parameters, multipleParameters)

	/* More on this line? */
	if (!endOfWords(currentWordIndex)) {
		if (isConjunctionWord(currentWordIndex))
			currentWordIndex++; /* If so skip the conjunction */
		else
			error(context, M_WHAT);
	}
}

/*======================================================================*/
void initParsing(void) {
	currentWordIndex = 0;
	continued = FALSE;
	ensureSpaceForPlayerWords(0);
	clearWordList(playerWords);

	pronouns = allocatePronounArray(pronouns);
	globalParameters = ensureParameterArrayAllocated(globalParameters);
	previousMultipleParameters = ensureParameterArrayAllocated(previousMultipleParameters);
}

/*----------------------------------------------------------------------*/
static int pronounWordForInstance(int instance) {
	/* Scan through the dictionary to find any pronouns that can be used
	   for this instance */
	int w;

	for (w = 0; w < dictionarySize; w++)
		if (isPronoun(w)) {
			Aword *reference = (Aword *)pointerTo(dictionary[w].pronounRefs);
			while (*reference != EOD) {
				if (*reference == (Aword)instance)
					return dictionary[w].code;
				reference++;
			}
		}
	return 0;
}

/*----------------------------------------------------------------------*/
static void addPronounForInstance(int thePronoun, int instanceCode) {
	int i;

	for (i = 0; !endOfPronouns(i); i++)
		if (pronouns[i].pronoun == thePronoun && pronouns[i].instance == instanceCode)
			// Don't add the same instance twice for the same pronoun
			return;
	pronouns[i].pronoun = thePronoun;
	pronouns[i].instance = instanceCode;
	setEndOfArray(&pronouns[i + 1]);
}

/*----------------------------------------------------------------------*/
static void notePronounsForParameters(Parameter parameters[]) {
	/* For all parameters note which ones can be referred to by a pronoun */
	Parameter *p;

	clearPronounList(pronouns);
	for (p = parameters; !isEndOfArray(p); p++) {
		int pronoun = pronounWordForInstance(p->instance);
		if (pronoun > 0)
			addPronounForInstance(pronoun, p->instance);
	}
}


/*----------------------------------------------------------------------*/
static void parseVerbCommand(CONTEXT, Parameter parameters[], Parameter multipleParameters[]) {
	verbWord = playerWords[currentWordIndex].code;
	verbWordCode = dictionary[verbWord].code;
	if (isPreBeta2(header->version))
		/* Pre-beta2 didn't generate syntax elements for verb words,
		   need to skip first word which should be the verb */
		currentWordIndex++;
	CALL2(parseOneCommand, parameters, multipleParameters)
	notePronounsForParameters(parameters);
	fail = FALSE;
}


/*----------------------------------------------------------------------*/
static void parseInstanceCommand(CONTEXT, Parameter parameters[], Parameter multipleParameters[]) {
	/* Pick up the parse tree for the syntaxes that start with an
	   instance reference and parse according to that. The
	   verbWord code is set to 0 to indicate that it is not a verb
	   but an instance that starts the command. */
	verbWordCode = 0;
	CALL2(parseOneCommand, parameters, multipleParameters)
	notePronounsForParameters(parameters);
	fail = FALSE;
}


/*======================================================================*/
void parse(CONTEXT) {
	/* longjmp's ahead so these need to survive to not leak memory */
	static Parameter *parameters = NULL;
	static Parameter *multipleParameters = NULL;
	parameters = ensureParameterArrayAllocated(parameters);
	multipleParameters = ensureParameterArrayAllocated(multipleParameters);

	if (endOfWords(currentWordIndex)) {
		currentWordIndex = 0;
		CALL0(scan)
	} else if (anyOutput) {
		para();
	}

	capitalize = TRUE;

	firstWord = currentWordIndex;
	if (isVerbWord(currentWordIndex)) {
		CALL2(parseVerbCommand, parameters, multipleParameters);
		CALL3(action, current.verb, parameters, multipleParameters)
	} else if (isDirectionWord(currentWordIndex)) {
		clearParameterArray(previousMultipleParameters);
		clearPronounList(pronouns);
		CALL0(handleDirectionalCommand)
	} else if (isInstanceReferenceWord(currentWordIndex)) {
		CALL2(parseInstanceCommand, parameters, multipleParameters)
		CALL3(action, current.verb, parameters, multipleParameters)
	} else
		CALL1(error, M_WHAT)

	if (fail)
		CALL1(error, NO_MSG)

	lastWord = currentWordIndex - 1;
	if (isConjunctionWord(lastWord))
		lastWord--;

	if (lengthOfParameterArray(parameters) > 0)
		copyParameterArray(previousMultipleParameters, multipleParameters);
	else
		clearParameterArray(previousMultipleParameters);

	freeParameterArray(parameters);
	parameters = NULL;
	freeParameterArray(multipleParameters);
	multipleParameters = NULL;
}

} // End of namespace Alan3
} // End of namespace Glk
