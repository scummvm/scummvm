/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __PARSE_H
#define __PARSE_H

namespace Gob {

int16 parse_parseExpr(char stopToken, byte *resultPtr);
void parse_skipExpr(char stopToken);
int16 parse_parseValExpr(void);
int16 parse_parseVarIndex(void);
void parse_printExpr(char stopToken);
void parse_printVarIndex(void);

}				// End of namespace Gob

#endif
