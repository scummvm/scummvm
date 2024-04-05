
#ifndef __QDSCR_MATHEXP_H__
#define __QDSCR_MATHEXP_H__

// Return values...
#define MT_EXP_OK			0
#define MT_EXP_PARSE_FAILED		1
#define MT_EXP_EVALUATION_FAILED	2
#define MT_EXP_BAD_OPERAND		3

int i_parseMathExpr(void* expr,int& out);
int d_parseMathExpr(void* expr,double& out);

#endif /* __QDSCR_MATHEXP_H__ */
