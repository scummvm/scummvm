#ifndef __FORMULA_STRING_H_INCLUDED__
#define __FORMULA_STRING_H_INCLUDED__

#include <string>
#include "XTL\Functor.h"

class Archive;

class FormulaString{
	friend struct  Calculator;
public:

	enum EvalResult {
		EVAL_SUCCESS,
		EVAL_SYNTAX_ERROR,
		EVAL_UNDEFINED_NAME
	};

	typedef Functor2<void, const char*, const char*> ParserCallback;
	typedef Functor2<bool, const char*, float&> LookupFunction;

    FormulaString (const char* formula = "X")
    : formula_ (formula)
    {
    }
    const char* c_str() const{
        return formula_.c_str();
    }
    void set (const char* formula){
        formula_ = formula;
    }
    void operator= (const char* formula){
        set (formula);
    }

	struct Dummy{
		inline void operator()(const char*, const char*)
		{
		}
	};

	EvalResult evaluate(float& result, float x, LookupFunction lookup_function,
						ParserCallback var_callback = Dummy(),
						ParserCallback badvar_callback = Dummy(),
						ParserCallback op_callback = Dummy()) const;

	void serialize(Archive& ar);
private:
    std::string formula_;
};

#endif
