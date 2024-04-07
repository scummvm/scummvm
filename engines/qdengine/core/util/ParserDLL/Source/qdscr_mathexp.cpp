/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <crtdbg.h>

#include "qdscr_mathexp.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */

template <class Type> class qdscr_mtList
{
	int numElements;
	Type* firstElement;
	void test(int code);

public:

	qdscr_mtList();
	~qdscr_mtList();

	int size();
	Type* first();
	Type* last();

	void clear();
	void delete_all();

	void insert(Type* p);
	void append(Type* p);
	void insert(Type* pointer,Type* p);
	void append(Type* pointer,Type* p);
	void remove(Type* p);
	Type* search(int ID);
};

template <class Type>
inline void qdscr_mtList<Type>::test(int code)
{
#ifdef _XT_TEST_LIST_
	Type* p = first();
	int cnt = 0;
	while(p){
		cnt++;
		p = p -> next;
		}
	if(cnt != numElements)
		ErrH.Abort("List",XERR_USER,code);
#endif
}

template <class Type>
inline qdscr_mtList<Type>::qdscr_mtList()
{
	numElements = 0; firstElement = 0;
}

template <class Type>
inline qdscr_mtList<Type>::~qdscr_mtList()
{
	clear();
}

template <class Type>
inline void qdscr_mtList<Type>::clear()
{
	while(first())
		remove(first());
}

template <class Type>
inline void qdscr_mtList<Type>::delete_all()
{
	Type* p;
	while((p = first()) != 0){
		remove(p);
		delete p;
		}
}

template <class Type>
inline int qdscr_mtList<Type>::size()
{
	return numElements;
}

template <class Type>
inline Type* qdscr_mtList<Type>::first()
{
	return firstElement;
}

template <class Type>
inline Type* qdscr_mtList<Type>::last()
{
	return firstElement ? firstElement -> prev : 0;
}

template <class Type>
inline void qdscr_mtList<Type>::insert(Type* p)
{
	if(p -> list)
		ErrH.Abort("Element is already in list");
	numElements++;
	if(firstElement){
		p -> next = firstElement;
		p -> prev = firstElement -> prev;
		firstElement -> prev = p;
		}
	else{
		p -> prev = p;
		p -> next = 0;
		}
	firstElement = p;
	p -> list = this;
	test(0);
}

template <class Type>
inline void qdscr_mtList<Type>::insert(Type* pointer,Type* p)
{
	if(!firstElement || firstElement == pointer){
		insert(p);
		return;
		}
	if(!pointer){
		append(p);
		return;
		}

	if(p -> list)
		ErrH.Abort("Element is already in list");
	numElements++;
	p -> next = pointer;
	p -> prev = pointer -> prev;
	pointer -> prev -> next = p;
	pointer -> prev = p;
	p -> list = this;
	test(5);
}


template <class Type>
inline void qdscr_mtList<Type>::append(Type* p)
{
	if(p -> list)
		ErrH.Abort("Element is already in list");
	numElements++;
	if(firstElement){
		p -> next = 0;
		p -> prev = firstElement -> prev;
		firstElement -> prev -> next = p;
		firstElement -> prev = p;
		}
	else{
		p -> next = 0;
		p -> prev = firstElement = p;
		}
	p -> list = this;
	test(1);
}

template <class Type>
inline void qdscr_mtList<Type>::remove(Type* p)
{
	if(p -> list != this)
		ErrH.Abort("Removed element isn't in list");
	numElements--;
	if(p -> next)
		p -> next -> prev = p -> prev;
	else
		firstElement -> prev = p -> prev;

	if(p != firstElement)
		p -> prev -> next = p -> next;
	else{
		firstElement = p -> next;
		if(firstElement)
			firstElement -> prev = p -> prev;
		}

	p -> next = p -> prev = 0;
	p -> list = 0;
	test(2);
}

template <class Type>
inline Type* qdscr_mtList<Type>::search(int ID)
{
	Type* p = first();
	while(p){
		if(p -> ID == ID)
			return p;
		p = p -> next;
		}
	return 0;
}

// qdscr_mathExpAtom::type...
enum qdscr_mathExpAtomTypes
{
	MT_EXP_VAL,
	MT_EXP_ADD,
	MT_EXP_SUB,
	MT_EXP_MUL,
	MT_EXP_DIV,
	MT_EXP_AND,
	MT_EXP_OR,
	MT_EXP_XOR,

	MT_EXP_LBRACKET,
	MT_EXP_RBRACKET,

	MT_EXP_SHIFT_LEFT,
	MT_EXP_SHIFT_RIGHT,

	MT_EXP_MAX_ID
};

struct qdscr_mathExpAtom
{
	int type;
	int priority;

	union {
		int icode;
		double dcode;
	};

	void* list;
	qdscr_mathExpAtom* prev;
	qdscr_mathExpAtom* next;

	qdscr_mathExpAtom(int tp,int val){ type = tp; icode = val; list = 0; }
	qdscr_mathExpAtom(int tp,double val){ type = tp; dcode = val; list = 0; }
};

// qdscr_mathExp::flags...
#define MT_EXP_DOUBLE_VALUES	0x01
#define MT_EXP_FAILED		0x02

struct qdscr_mathExp
{
	int flags;
	int status;
	qdscr_mtList<qdscr_mathExpAtom>* exprData;

	union {
		int ivalue;
		double dvalue;
	};

	void init(int val_type,void* ptr);
	void parse(char* expr);
	void evaluate(void);

	qdscr_mathExp(void);
	~qdscr_mathExp(void);
};

/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

int mth_isDigit(int v);
int mth_isMathOp(int v);
int mth_MathOpLen(int code);
int mth_isSpace(int v);

int mth_Str2INT(char* p);

/* --------------------------- DEFINITION SECTION --------------------------- */

static int mathExpPriorityDelta[] = { 0, 3, 3, 4, 4, 2, 2, 2, 5, -5, 1, 1 };
static char* mathExpAtomTypeData[] = { "+", "-", "*", "/", "&", "|", "^", "(", ")", "<<", ">>" };

qdscr_mathExp::qdscr_mathExp(void)
{
	flags = 0;
	status = MT_EXP_OK;
	exprData = new qdscr_mtList<qdscr_mathExpAtom>;
}

qdscr_mathExp::~qdscr_mathExp(void)
{
	qdscr_mathExpAtom* p;

	p = exprData -> first();
	while(p){
		exprData -> remove(p);
		delete p;
		p = exprData -> first();
	}
	delete exprData;
}

void qdscr_mathExp::init(int val_type,void* ptr)
{
	int src_len,dest_len,cur_index,dest_index,code,len;
	char* src_str,*dest_str;

	if(val_type) flags |= MT_EXP_DOUBLE_VALUES;

	src_str = (char*)ptr;
	src_len = strlen((char*)src_str);

	dest_len = src_len * 3 + 1;
	dest_str = new char[dest_len];
	memset(dest_str,0,dest_len);

	cur_index = dest_index = 0;
	while(mth_isSpace(src_str[cur_index])){
		cur_index ++;
		if(cur_index >= src_len) break;
	}

	while(cur_index < src_len){
		code = mth_isMathOp(src_str[cur_index]);
		if(code){
			len = mth_MathOpLen(code);

			if(dest_index && !mth_isSpace(dest_str[dest_index - 1])){
				dest_str[dest_index] = ' ';
				dest_index ++;
			}

			memcpy(dest_str + dest_index,src_str + cur_index,len);
			cur_index += len;
			dest_index += len;

			if(cur_index < src_len && !mth_isSpace(src_str[cur_index])){
				dest_str[dest_index] = ' ';
				dest_index ++;
			}
		}
		else {
			dest_str[dest_index] = src_str[cur_index];
			cur_index ++;
			dest_index ++;
		}
	}

	parse(dest_str);
	delete dest_str;
}

void qdscr_mathExp::parse(char* expr)
{
	int i,expr_len,op_code,pr;

	int ival;
	double dval;

	qdscr_mathExpAtom* p,*p1;

	expr_len = strlen(expr);
	for(i = 0; i < expr_len; i ++){
		if(mth_isSpace(expr[i])) expr[i] = 0;
	}

	i = 0;
	while(i < expr_len){
		while(!expr[i]){
			i ++;
			if(i >= expr_len) break;
		}

		op_code = mth_isMathOp(expr[i]);

		if(op_code){
			p = new qdscr_mathExpAtom(op_code,op_code);
		}
		else {
			if(flags & MT_EXP_DOUBLE_VALUES){
				dval = atof(expr + i);
				p = new qdscr_mathExpAtom(op_code,dval);
			}
			else {
				ival = mth_Str2INT(expr + i);
				p = new qdscr_mathExpAtom(op_code,ival);
			}
		}
		exprData -> append(p);
		while(expr[i]){
			i ++;
			if(i >= expr_len) break;
		}
		if(i >= expr_len) break;
	}

	pr = 0;
	p = exprData -> first();
	while(p){
		p -> priority = 0;
		if(p -> type){
			if(p -> type == MT_EXP_LBRACKET || p -> type == MT_EXP_RBRACKET)
				pr += mathExpPriorityDelta[p -> type];

			p -> priority = pr + mathExpPriorityDelta[p -> type];
		}
		p = p -> next;
	}

	p = exprData -> first();
	while(p){
		p1 = p -> next;
		if(p -> type){
			if(p -> type == MT_EXP_LBRACKET || p -> type == MT_EXP_RBRACKET){
				exprData -> remove(p);
				delete p;
			}
		}
		p = p1;
	}
}

void qdscr_mathExp::evaluate(void)
{
	int pr,flag;

	int i_val,i_val1,i_val2;
	double d_val,d_val1,d_val2;

	qdscr_mathExpAtom* p,*p1,*prev_ptr,*next_ptr;

	p = exprData -> first();
	pr = p -> priority;
	while(p){
		if(p -> priority > pr){
			pr = p -> priority;
		}
		p = p -> next;
	}

	while(pr >= 0){
		flag = 1;
		while(flag){
			flag = 0;
			p = exprData -> first();
			while(p){
				p1 = p -> next;
				if(p -> type && p -> priority == pr){
					if(p == exprData -> first()){
						prev_ptr = NULL;
						next_ptr = p -> next;
					}
					else {
						prev_ptr = p -> prev;
						next_ptr = p -> next;
					}

					if(!next_ptr){
						flags |= MT_EXP_FAILED;
						status = MT_EXP_EVALUATION_FAILED;
						return;
					}
					if(next_ptr -> type){
						p = next_ptr;
						p1 = p -> next;
						prev_ptr = p -> prev;
						next_ptr = p -> next;
					}
					if(prev_ptr && prev_ptr -> type){
						prev_ptr = NULL;
					}
					if(flags & MT_EXP_DOUBLE_VALUES){
						d_val1 = (prev_ptr) ? prev_ptr -> dcode : 0.0;
						d_val2 = next_ptr -> dcode;

						switch(p -> type){
							case MT_EXP_ADD:
								d_val = d_val1 + d_val2;
								break;
							case MT_EXP_SUB:
								d_val = d_val1 - d_val2;
								break;
							case MT_EXP_MUL:
								d_val = d_val1 * d_val2;
								break;
							case MT_EXP_DIV:
								d_val = d_val1 / d_val2;
								break;
							case MT_EXP_AND:
							case MT_EXP_OR:
							case MT_EXP_XOR:
								flags |= MT_EXP_FAILED;
								status = MT_EXP_BAD_OPERAND;
								return;
						}
						next_ptr -> dcode = d_val;
					}
					else {
						i_val1 = (prev_ptr) ? prev_ptr -> icode : 0;
						i_val2 = next_ptr -> icode;

						switch(p -> type){
							case MT_EXP_ADD:
								i_val = i_val1 + i_val2;
								break;
							case MT_EXP_SUB:
								i_val = i_val1 - i_val2;
								break;
							case MT_EXP_MUL:
								i_val = i_val1 * i_val2;
								break;
							case MT_EXP_DIV:
								i_val = i_val1 / i_val2;
								break;
							case MT_EXP_AND:
								i_val = i_val1 & i_val2;
								break;
							case MT_EXP_OR:
								i_val = i_val1 | i_val2;
								break;
							case MT_EXP_XOR:
								i_val = i_val1 ^ i_val2;
								break;
							case MT_EXP_SHIFT_LEFT:
								i_val = i_val1 << i_val2;
								break;
							case MT_EXP_SHIFT_RIGHT:
								i_val = i_val1 >> i_val2;
								break;
						}
						next_ptr -> icode = i_val;
					}

					if(prev_ptr){
						exprData -> remove(prev_ptr);
						delete prev_ptr;
					}

					exprData -> remove(p);
					delete p;

					flag = 1;
					break;
				}
				p = p1;
			}
		}
		pr --;
	}
	p = exprData -> first();
	if(flags & MT_EXP_DOUBLE_VALUES)
		dvalue = p -> dcode;
	else
		ivalue = p -> icode;
}

int mth_isDigit(int v)
{
	return isdigit(v);
}

int mth_isMathOp(int v)
{
	int i;
	for(i = 0; i < MT_EXP_MAX_ID - 1; i ++){
//		if(v == mathExpAtomTypeData[i]) return i + 1;
		if(v == mathExpAtomTypeData[i][0]) return i + 1;
	}
	return 0;
}

int mth_isSpace(int v)
{
	return isspace(v);
}

int i_parseMathExpr(void* expr,int& out)
{
	int st;
	qdscr_mathExp* p = new qdscr_mathExp;
	p -> init(0,expr);
	p -> evaluate();

	st = p -> status;

	out = p -> ivalue;
	delete p;

	return st;
}

int d_parseMathExpr(void* expr,double& out)
{
	int st;
	qdscr_mathExp* p = new qdscr_mathExp;
	p -> init(1,expr);
	p -> evaluate();

	st = p -> status;

	out = p -> dvalue;
	delete p;

	return st;
}

int mth_Str2INT(char* p)
{
	int len,val = 0;
	char* stop_str;

	len = strlen(p);
	if(len > 2 && p[0] == '0'){
		switch(p[1]){
			case 'x':
			case 'X':
				val = strtol(p + 2,&stop_str,16);
				break;
			case 'o':
			case 'O':
				val = strtol(p + 2,&stop_str,8);
				break;
			case 'b':
			case 'B':
				val = strtol(p + 2,&stop_str,2);
				break;
			default:
				val = strtol(p,&stop_str,10);
				break;
		}
	}
	else {
		val = strtol(p,&stop_str,10);
	}
	return val;
}

int mth_MathOpLen(int code)
{
	return strlen(mathExpAtomTypeData[code - 1]);
}

