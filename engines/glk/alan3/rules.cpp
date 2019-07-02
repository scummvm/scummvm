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

#include "glk/alan3/types.h"
#include "glk/alan3/rules.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/debug.h"
#include "glk/alan3/current.h"
#include "glk/alan3/options.h"
#include "glk/alan3/compatibility.h"

#ifdef HAVE_GLK
#include "glk/alan3/glkio.h"
#endif

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA: */
RuleEntry *rules;         /* Rule table pointer */
bool anyRuleRun;


/* PRIVATE TYPES: */
typedef struct RulesAdmin {
	bool lastEval;
	bool alreadyRun;
} RulesAdmin;

/* PRIVATE DATA: */
static int ruleCount;
static RulesAdmin *rulesAdmin; /* Table for administration of the rules */

/*----------------------------------------------------------------------*/
static void clearRulesAdmin(int numRules) {
	int r;
	for (r = 0; r < numRules; r++) {
		rulesAdmin[r].lastEval = FALSE;
		rulesAdmin[r].alreadyRun = FALSE;
	}
}


/*----------------------------------------------------------------------*/
static void initRulesAdmin(int numRules) {
	int r;

	rulesAdmin = (RulesAdmin *)allocate(numRules * sizeof(RulesAdmin) + sizeof(EOD));
	for (r = 0; r < numRules; r++)
		;
	setEndOfArray(&rulesAdmin[r]);
}


/*======================================================================*/
void initRules(Aaddr ruleTableAddress) {

	rules = (RuleEntry *) pointerTo(ruleTableAddress);

	if (ruleCount == 0) {       /* Not initiated */
		for (ruleCount = 0; !isEndOfArray(&rules[ruleCount]); ruleCount++)
			;
		initRulesAdmin(ruleCount);
	}
	clearRulesAdmin(ruleCount);
}


/*----------------------------------------------------------------------*/
static void traceRuleStart(CONTEXT, int rule, const char *what) {
	printf("\n<RULE %d", rule);
	if (current.location != 0) {
		printf(" (at ");
		CALL1(traceSay, current.location)
	} else
		printf(" (nowhere");
	printf("[%d]), %s", current.location, what);
}

static bool detailedTraceOn() {
	return traceInstructionOption || traceSourceOption || tracePushOption || traceStackOption;
}


/*----------------------------------------------------------------------*/
static void traceRuleEvaluation(CONTEXT, int rule) {
	if (traceSectionOption) {
		if (detailedTraceOn()) {
			CALL2(traceRuleStart, rule, "Evaluating:>")
			if (!traceInstructionOption)
				printf("\n");
		} else {
			CALL2(traceRuleStart, rule, "Evaluating to ")
		}
	}
}

/*----------------------------------------------------------------------*/
static void traceRuleResult(int rule, bool result) {
	if (traceSectionOption) {
		if (detailedTraceOn())
			printf("<RULE %d %s%s", rule, "Evaluated to ", result ? ": true>\n" : ": false>\n");
		else
			printf(result ? "true" : "false");
	}
}

/*----------------------------------------------------------------------*/
static void traceRuleExecution(CONTEXT, int rule) {
	if (traceSectionOption) {
		if (!traceInstructionOption && !traceSourceOption)
			printf(", Executing:>\n");
		else {
			CALL2(traceRuleStart, rule, "Executing:>")
			if (!traceInstructionOption)
				printf("\n");
		}
	}
}



/*----------------------------------------------------------------------*/
static void evaluateRulesPreBeta2(CONTEXT) {
	bool change = TRUE;
	bool flag;
	int i;

	for (i = 1; !isEndOfArray(&rules[i - 1]); i++)
		rules[i - 1].alreadyRun = FALSE;

	while (change) {
		change = FALSE;
		for (i = 1; !isEndOfArray(&rules[i - 1]); i++)
			if (!rules[i - 1].alreadyRun) {
				CALL1(traceRuleEvaluation, i)
				FUNC1(evaluate, flag, rules[i - 1].exp)
				if (flag) {
					change = TRUE;
					rules[i - 1].alreadyRun = TRUE;
					CALL1(traceRuleExecution, i)
					CALL1(interpret, rules[i - 1].stms)
				} else if (traceSectionOption && !traceInstructionOption)
					printf(":>\n");
			}
	}
}


/*----------------------------------------------------------------------*/
/* This is how beta2 thought rules should be evaluated:
 */
static void evaluateRulesBeta2(CONTEXT) {
	bool change = TRUE;
	bool triggered;
	int i;

	for (i = 1; !isEndOfArray(&rules[i - 1]); i++)
		rules[i - 1].alreadyRun = FALSE;

	current.location = NOWHERE;
	current.actor = 0;

	while (change) {
		change = FALSE;
		for (i = 1; !isEndOfArray(&rules[i - 1]); i++)
			if (!rules[i - 1].alreadyRun) {
				CALL1(traceRuleEvaluation, i)
				FUNC1(evaluate, triggered, rules[i - 1].exp)

				if (triggered) {
					if (rulesAdmin[i - 1].lastEval == false) {
						change = TRUE;
						rules[i - 1].alreadyRun = TRUE;
						CALL1(traceRuleExecution, i)
						CALL1(interpret, rules[i - 1].stms)
					}
					rulesAdmin[i - 1].lastEval = triggered;
				} else {
					rulesAdmin[i - 1].lastEval = false;
					if (traceSectionOption && !traceInstructionOption)
						printf(":>\n");
				}
			}
	}
}


/*======================================================================*/
void resetRules() {
	int i;
	for (i = 1; !isEndOfArray(&rules[i - 1]); i++) {
		rulesAdmin[i - 1].alreadyRun = FALSE;
	}
}


/*======================================================================*/
void evaluateRules(CONTEXT, RuleEntry ruleList[]) {
	bool change = TRUE;
	bool evaluated_value;
	int rule;

	current.location = NOWHERE;
	current.actor = 0;

	while (change) {
		change = FALSE;
		for (rule = 1; !isEndOfArray(&ruleList[rule - 1]); rule++) {
			CALL1(traceRuleEvaluation, rule)
			FUNC1(evaluate, evaluated_value, ruleList[rule - 1].exp)
			traceRuleResult(rule, evaluated_value);

			if (evaluated_value == true && rulesAdmin[rule - 1].lastEval == false
			        && !rulesAdmin[rule - 1].alreadyRun) {
				change = TRUE;
				CALL1(traceRuleExecution, rule)
				CALL1(interpret, ruleList[rule - 1].stms)
				rulesAdmin[rule - 1].alreadyRun = TRUE;
				anyRuleRun = TRUE;
			} else {
				if (traceSectionOption && !(traceInstructionOption || traceSourceOption))
					printf(":>\n");
			}
			rulesAdmin[rule - 1].lastEval = evaluated_value;
		}
	}
}


/*=======================================================================*/
void resetAndEvaluateRules(CONTEXT, RuleEntry ruleList[], const byte *version) {
	if (isPreBeta2(version))
		evaluateRulesPreBeta2(context);
	else if (isPreBeta3(version))
		evaluateRulesBeta2(context);
	else {
		resetRules();
		evaluateRules(context, ruleList);
	}
}

} // End of namespace Alan3
} // End of namespace Glk
