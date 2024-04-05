#include "stdafx.h"
#pragma warning( disable : 4518 )

#include "Serialization\Serialization.h"

#include "FormulaString.h"

#include <boost\spirit\core.hpp>
#include <boost\bind.hpp>

#pragma warning( disable : 4503 )
#pragma inline_depth(255)
#pragma inline_recursion(on)

using namespace boost::spirit;
using boost::bind;

typedef FormulaString::ParserCallback ParserCallback;
typedef FormulaString::LookupFunction LookupFunction;

struct Calculator : grammar<Calculator> {
    Calculator (float x_value, LookupFunction lookup_function, ParserCallback _var_func, ParserCallback _bad_var_func, ParserCallback _op_func)
        : var_func_(_var_func)
        , bad_var_func_(_bad_var_func)
        , op_func_(_op_func)
		, lookup_func_(lookup_function)
		, bad_vars_(false)
		, x_value_(x_value)
	{
    }

	friend struct definition;

    float result() const {
        if (!values_.empty()) {
            return values_.back();
        } else {
            return 0.0f;
        }
    }

	bool haveUndefinedNames () const {
		return bad_vars_;
	}

    std::vector<float> values_;
	float x_value_;

    ParserCallback var_func_;
    ParserCallback bad_var_func_;
    ParserCallback op_func_;
	LookupFunction lookup_func_;
	bool bad_vars_;

    template<typename ScannerT>
    struct definition {
        definition(Calculator const& const_self) {
            using namespace boost;
            Calculator& self = const_cast<Calculator&>(const_self);
            first = (
                expression = term   >> *(('+' >> term)[bind(&Calculator::add_op, ref(self), _1, _2)] |
                                         ('-' >> term)[bind(&Calculator::subt_op, ref(self), _1, _2)]),

                term       = factor >> *(('*' >> factor)[bind(&Calculator::mult_op, ref(self), _1, _2)] |
										 ('%' >> factor)[bind(&Calculator::percent_op, ref(self), _1, _2)] |
                                         ('/' >> factor)[bind(&Calculator::div_op, ref(self), _1, _2)]),

                factor     = real_p[bind(&Calculator::push_op, ref(self), _1)] |
							 lexeme_d[(+(alpha_p | digit_p))[bind(&Calculator::var_op, ref(self), _1, _2)]] |
							 '\'' >> lexeme_d[(+(~ch_p('\'')))[bind(&Calculator::var_op, ref(self), _1, _2)]] >> '\'' |
                             '(' >> expression >> ')' | ('-' >> factor) | ('+' >> factor)
            );
		}

        subrule<0> expression;
        subrule<1> term;
        subrule<2> factor;

        rule<ScannerT> first;

        rule<ScannerT> const& start () const {
            return first;
        }
    };
private:
    float pop () {
        if (values_.empty ()) {
            return 0;
        } else {
            float result = values_.back();
            values_.pop_back();
            return result;
        }
    }
    
    void push_op(float value) {
        values_.push_back(value);
    }

    void var_op(const char* start, const char* end) {
        std::string var (start, end);
		if(stricmp(var.c_str(), "X") == 0){
			var_func_ (start, end);
			values_.push_back(x_value_);
		}
		else{
			float value;
			if(lookup_func_(var.c_str(), value)){
				var_func_(start, end);
				values_.push_back(value);
			}
			else{
				bad_vars_ = true;
				bad_var_func_(start, end);
				values_.push_back(0.0f);
			}
		}
    }

    void mult_op(const char* start, const char* end){
        op_func_ (start, start + 1);
        float b = pop();
        float a = pop();
        push_op (a * b);
    }

    void div_op(const char* start, const char* end) {
        op_func_ (start, start + 1);
        float b = pop();
        float a = pop();
		if (fabs(b) < FLT_COMPARE_TOLERANCE){
			push_op (FLT_INF);
		} else {
	        push_op (a / b);
		}
    }

	void percent_op (const char* start, const char* end) {
        op_func_ (start, start + 1);
        float b = pop();
        float a = pop();
		if (fabs(b) < FLT_COMPARE_TOLERANCE) {
			push_op (FLT_INF);
		} else {
	        push_op (a * b * 0.01f);
		}
    }

    void add_op (const char* start, const char* end) {
        op_func_ (start, start + 1);
        float b = pop();
        float a = pop();
        push_op (a + b);
    }

    void subt_op (const char* start, const char* end) {
        op_func_ (start, start + 1);
        float b = pop();
        float a = pop();
        push_op (a - b);
    }
};


void FormulaString::serialize (Archive& ar) 
{
    ar.serialize(formula_, "formula", "^<Формула");
}


FormulaString::EvalResult FormulaString::evaluate(float& result, float x, LookupFunction lookup_function, ParserCallback var_callback, ParserCallback badvar_callback, ParserCallback op_callback) const
{
	Calculator calc(x, lookup_function, var_callback, badvar_callback, op_callback);
	parse_info<> info = parse(c_str(), calc, space_p);
	if (info.full) {
		result = calc.result();
		if (calc.haveUndefinedNames ()) {
			return EVAL_UNDEFINED_NAME;
		} else {
			return EVAL_SUCCESS;
		}
	} else {
		return EVAL_SYNTAX_ERROR;
	}
}

