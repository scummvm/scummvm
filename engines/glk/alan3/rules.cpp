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
static void clearRulesAdmin(int ruleCount) {
    int r;
    for (r = 0; r < ruleCount; r++) {
        rulesAdmin[r].lastEval = FALSE;
        rulesAdmin[r].alreadyRun = FALSE;
    }
}


/*----------------------------------------------------------------------*/
static void initRulesAdmin(int ruleCount) {
    int r;

    rulesAdmin = (RulesAdmin *)allocate(ruleCount*sizeof(RulesAdmin)+sizeof(EOF));
    for (r = 0; r < ruleCount; r++)
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
static void traceRuleStart(int rule, char *what) {
    printf("\n<RULE %d", rule);
    if (current.location != 0) {
        printf(" (at ");
        traceSay(current.location);
    } else
        printf(" (nowhere");
    printf("[%d]), %s", current.location, what);
}

static bool detailedTraceOn() {
    return traceInstructionOption || traceSourceOption || tracePushOption || traceStackOption;
}


/*----------------------------------------------------------------------*/
static void traceRuleEvaluation(int rule) {
    if (traceSectionOption) {
        if (detailedTraceOn()) {
            traceRuleStart(rule, "Evaluating:>");
            if (!traceInstructionOption)
                printf("\n");
        } else {
            traceRuleStart(rule, "Evaluating to ");
        }
    }
}

/*----------------------------------------------------------------------*/
static void traceRuleResult(int rule, bool result) {
    if (traceSectionOption) {
        if (detailedTraceOn())
            printf("<RULE %d %s%s", rule, "Evaluated to ", result?": true>\n":": false>\n");
        else
            printf(result?"true":"false");
    }
}

/*----------------------------------------------------------------------*/
static void traceRuleExecution(int rule) {
    if (traceSectionOption) {
        if (!traceInstructionOption && !traceSourceOption)
            printf(", Executing:>\n");
        else {
            traceRuleStart(rule, "Executing:>");
            if (!traceInstructionOption)
                printf("\n");
        }
    }
}



/*----------------------------------------------------------------------*/
static void evaluateRulesPreBeta2(void)
{
    bool change = TRUE;
    int i;

    for (i = 1; !isEndOfArray(&rules[i-1]); i++)
        rules[i-1].alreadyRun = FALSE;

    while (change) {
        change = FALSE;
        for (i = 1; !isEndOfArray(&rules[i-1]); i++)
            if (!rules[i-1].alreadyRun) {
                traceRuleEvaluation(i);
                if (evaluate(rules[i-1].exp)) {
                    change = TRUE;
                    rules[i-1].alreadyRun = TRUE;
                    traceRuleExecution(i);
                    interpret(rules[i-1].stms);
                } else if (traceSectionOption && !traceInstructionOption)
                    printf(":>\n");
            }
    }
}


/*----------------------------------------------------------------------*/
static void evaluateRulesBeta2New(void) {
    int i;

    for (i = 1; !isEndOfArray(&rules[i-1]); i++)
        rules[i-1].alreadyRun = FALSE;

    current.location = NOWHERE;
    current.actor = 0;

    anyRuleRun = FALSE;

    for (i = 1; !isEndOfArray(&rules[i-1]); i++) {
        bool evaluated_value = evaluate(rules[i-1].exp);
        traceRuleEvaluation(i);
        rules[i-1].alreadyRun = evaluated_value;
    }
    for (i = 1; !isEndOfArray(&rules[i-1]); i++) {
        if (rules[i-1].alreadyRun) {
            traceRuleExecution(i);
            interpret(rules[i-1].stms);
            anyRuleRun = TRUE;
        }
    }
}


/*----------------------------------------------------------------------*/
/* This is how beta2 thought rules should be evaluated:
 */
static void evaluateRulesBeta2(void)
{
    bool change = TRUE;
    int i;

    for (i = 1; !isEndOfArray(&rules[i-1]); i++)
        rules[i-1].alreadyRun = FALSE;

    current.location = NOWHERE;
    current.actor = 0;

    while (change) {
        change = FALSE;
        for (i = 1; !isEndOfArray(&rules[i-1]); i++)
            if (!rules[i-1].alreadyRun) {
                traceRuleEvaluation(i);
                bool triggered = evaluate(rules[i-1].exp);
                if (triggered) {
                    if (rulesAdmin[i-1].lastEval == false) {
                        change = TRUE;
                        rules[i-1].alreadyRun = TRUE;
                        traceRuleExecution(i);
                        interpret(rules[i-1].stms);
                    }
                    rulesAdmin[i-1].lastEval = triggered;
                } else {
                    rulesAdmin[i-1].lastEval = false;
                    if (traceSectionOption && !traceInstructionOption)
                        printf(":>\n");
                }
        }
    }
}


/*======================================================================*/
void resetRules() {
    int i;
    for (i = 1; !isEndOfArray(&rules[i-1]); i++) {
        rulesAdmin[i-1].alreadyRun = FALSE;
    }
}


/*======================================================================*/
void evaluateRules(RuleEntry rules[]) {
    bool change = TRUE;
    int rule;

    current.location = NOWHERE;
    current.actor = 0;

    while (change) {
        change = FALSE;
        for (rule = 1; !isEndOfArray(&rules[rule-1]); rule++) {
            traceRuleEvaluation(rule);
            bool evaluated_value = evaluate(rules[rule-1].exp);
            traceRuleResult(rule, evaluated_value);
            if (evaluated_value == true && rulesAdmin[rule-1].lastEval == false
                && !rulesAdmin[rule-1].alreadyRun) {
                change = TRUE;
                traceRuleExecution(rule);
                interpret(rules[rule-1].stms);
                rulesAdmin[rule-1].alreadyRun = TRUE;
                anyRuleRun = TRUE;
            } else {
                if (traceSectionOption && !(traceInstructionOption || traceSourceOption))
                    printf(":>\n");
            }
            rulesAdmin[rule-1].lastEval = evaluated_value;
        }
    }
}


/*=======================================================================*/
void resetAndEvaluateRules(RuleEntry rules[], char *version) {
    if (isPreBeta2(version))
        evaluateRulesPreBeta2();
    else if (isPreBeta3(version))
        evaluateRulesBeta2();
    else {
        resetRules();
        evaluateRules(rules);
    }
}

} // End of namespace Alan3
} // End of namespace Glk
