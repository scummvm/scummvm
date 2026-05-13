#-------------------------------------------------------------------------
# CxxTest: A lightweight C++ unit testing library.
# Copyright (c) 2008 Sandia Corporation.
# This software is distributed under the LGPL License v3
# For more information, see the COPYING file in the top CxxTest directory.
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.
#-------------------------------------------------------------------------

# vim: fileencoding=utf-8

#
# This is a PLY parser for the entire ANSI C++ grammar.  This grammar was
# adapted from the FOG grammar developed by E. D. Willink.  See
#
#    http://www.computing.surrey.ac.uk/research/dsrg/fog/
#
# for further details.
#
# The goal of this grammar is to extract information about class, function and
# class method declarations, along with their associated scope.  Thus, this
# grammar can be used to analyze classes in an inheritance heirarchy, and then
# enumerate the methods in a derived class.
#
# This grammar parses blocks of <>, (), [] and {} in a generic manner.  Thus,
# There are several capabilities that this grammar does not support:
#
# 1. Ambiguous template specification.  This grammar cannot parse template
#       specifications that do not have paired <>'s in their declaration.  In
#       particular, ambiguous declarations like
#
#           foo<A, c<3 >();
#
#       cannot be correctly parsed.
#
# 2. Template class specialization.  Although the goal of this grammar is to
#       extract class information, specialization of templated classes is
#       not supported.  When a template class definition is parsed, it's
#       declaration is archived without information about the template
#       parameters.  Class specializations will be stored separately, and
#       thus they can be processed after the fact.  However, this grammar
#       does not attempt to correctly process properties of class inheritence
#       when template class specialization is employed.
#

#
# TODO: document usage of this file
#



import os
import ply.lex as lex
import ply.yacc as yacc
import re
try:
    from collections import OrderedDict
except ImportError:             #pragma: no cover
    from ordereddict import OrderedDict

# global data
lexer = None
scope_lineno = 0
identifier_lineno = {}
_parse_info=None
_parsedata=None
noExceptionLogic = True


def ply_init(data):
    global _parsedata
    _parsedata=data


class Scope(object):

    def __init__(self,name,abs_name,scope_t,base_classes,lineno):
        self.function=[]
        self.name=name
        self.scope_t=scope_t
        self.sub_scopes=[]
        self.base_classes=base_classes
        self.abs_name=abs_name
        self.lineno=lineno

    def insert(self,scope):
        self.sub_scopes.append(scope)


class CppInfo(object):

    def __init__(self, filter=None):
        self.verbose=0
        if filter is None:
            self.filter=re.compile("[Tt][Ee][Ss][Tt]|createSuite|destroySuite")
        else:
            self.filter=filter
        self.scopes=[""]
        self.index=OrderedDict()
        self.index[""]=Scope("","::","namespace",[],1)
        self.function=[]

    def push_scope(self,ns,scope_t,base_classes=[]):
        name = self.scopes[-1]+"::"+ns
        if self.verbose>=2:
            print("-- Starting "+scope_t+" "+name)
        self.scopes.append(name)
        self.index[name] = Scope(ns,name,scope_t,base_classes,scope_lineno-1)

    def pop_scope(self):
        scope = self.scopes.pop()
        if self.verbose>=2:
            print("-- Stopping "+scope)
        return scope

    def add_function(self, fn):
        fn = str(fn)
        if self.filter.search(fn):
            self.index[self.scopes[-1]].function.append((fn, identifier_lineno.get(fn,lexer.lineno-1)))
            tmp = self.scopes[-1]+"::"+fn
            if self.verbose==2:
                print("-- Function declaration "+fn+"  "+tmp)
            elif self.verbose==1:
                print("-- Function declaration "+tmp)

    def get_functions(self,name,quiet=False):
        if name == "::":
            name = ""
        scope = self.index[name]
        fns=scope.function
        for key in scope.base_classes:
            cname = self.find_class(key,scope)
            if cname is None:
                if not quiet:
                    print("Defined classes: ",list(self.index.keys()))
                    print("WARNING: Unknown class "+key)
            else:
                fns += self.get_functions(cname,quiet)
        return fns

    def find_class(self,name,scope):
        if ':' in name:
            if name in self.index:
                return name
            else:
                return None
        tmp = scope.abs_name.split(':')
        name1 = ":".join(tmp[:-1] + [name])
        if name1 in self.index:
            return name1
        name2 = "::"+name
        if name2 in self.index:
            return name2
        return None

    def __repr__(self):
        return str(self)

    def is_baseclass(self,cls,base):
        '''Returns true if base is a base-class of cls'''
        if cls in self.index:
            bases = self.index[cls]
        elif "::"+cls in self.index:
            bases = self.index["::"+cls]
        else:
            return False
            #raise IOError, "Unknown class "+cls
        if base in bases.base_classes:
            return True
        for name in bases.base_classes:
            if self.is_baseclass(name,base):
                return True
        return False

    def __str__(self):
        ans=""
        keys = list(self.index.keys())
        keys.sort()
        for key in keys:
            scope = self.index[key]
            ans += scope.scope_t+" "+scope.abs_name+"\n"
            if scope.scope_t == "class":
                ans += "  Base Classes: "+str(scope.base_classes)+"\n"
                for fn in self.get_functions(scope.abs_name):
                    ans += "  "+fn+"\n"
            else:
                for fn in scope.function:
                    ans += "  "+fn+"\n"
        return ans


def flatten(x):
    """Flatten nested list"""
    try:
        strtypes = str
    except: # for python3 etc
        strtypes = (str, bytes)

    result = []
    for el in x:
        if hasattr(el, "__iter__") and not isinstance(el, strtypes):
            result.extend(flatten(el))
        else:
            result.append(el)
    return result

#
# The lexer (and/or a preprocessor) is expected to identify the following
#
#  Punctuation:
#
#
literals = "+-*/%^&|~!<>=:()?.\'\"\\@$;,"

#
reserved = {
    'private' : 'PRIVATE',
    'protected' : 'PROTECTED',
    'public' : 'PUBLIC',

    'bool' : 'BOOL',
    'char' : 'CHAR',
    'double' : 'DOUBLE',
    'float' : 'FLOAT',
    'int' : 'INT',
    'long' : 'LONG',
    'short' : 'SHORT',
    'signed' : 'SIGNED',
    'unsigned' : 'UNSIGNED',
    'void' : 'VOID',
    'wchar_t' : 'WCHAR_T',

    'class' : 'CLASS',
    'enum' : 'ENUM',
    'namespace' : 'NAMESPACE',
    'struct' : 'STRUCT',
    'typename' : 'TYPENAME',
    'union' : 'UNION',

    'const' : 'CONST',
    'volatile' : 'VOLATILE',

    'auto' : 'AUTO',
    'explicit' : 'EXPLICIT',
    'export' : 'EXPORT',
    'extern' : 'EXTERN',
    '__extension__' : 'EXTENSION',
    'friend' : 'FRIEND',
    'inline' : 'INLINE',
    'mutable' : 'MUTABLE',
    'register' : 'REGISTER',
    'static' : 'STATIC',
    'template' : 'TEMPLATE',
    'typedef' : 'TYPEDEF',
    'using' : 'USING',
    'virtual' : 'VIRTUAL',

    'asm' : 'ASM',
    'break' : 'BREAK',
    'case' : 'CASE',
    'catch' : 'CATCH',
    'const_cast' : 'CONST_CAST',
    'continue' : 'CONTINUE',
    'default' : 'DEFAULT',
    'delete' : 'DELETE',
    'do' : 'DO',
    'dynamic_cast' : 'DYNAMIC_CAST',
    'else' : 'ELSE',
    'false' : 'FALSE',
    'for' : 'FOR',
    'goto' : 'GOTO',
    'if' : 'IF',
    'new' : 'NEW',
    'operator' : 'OPERATOR',
    'reinterpret_cast' : 'REINTERPRET_CAST',
    'return' : 'RETURN',
    'sizeof' : 'SIZEOF',
    'static_cast' : 'STATIC_CAST',
    'switch' : 'SWITCH',
    'this' : 'THIS',
    'throw' : 'THROW',
    'true' : 'TRUE',
    'try' : 'TRY',
    'typeid' : 'TYPEID',
    'while' : 'WHILE',
    '"C"' : 'CLiteral',
    '"C++"' : 'CppLiteral',

    '__attribute__' : 'ATTRIBUTE',
    '__cdecl__' : 'CDECL',
    '__typeof' : 'uTYPEOF',
    'typeof' : 'TYPEOF',

    'CXXTEST_STD' : 'CXXTEST_STD'
}

tokens = [
    "CharacterLiteral",
    "FloatingLiteral",
    "Identifier",
    "IntegerLiteral",
    "StringLiteral",
 "RBRACE",
 "LBRACE",
 "RBRACKET",
 "LBRACKET",
 "ARROW",
 "ARROW_STAR",
 "DEC",
 "EQ",
 "GE",
 "INC",
 "LE",
 "LOG_AND",
 "LOG_OR",
 "NE",
 "SHL",
 "SHR",
 "ASS_ADD",
 "ASS_AND",
 "ASS_DIV",
 "ASS_MOD",
 "ASS_MUL",
 "ASS_OR",
 "ASS_SHL",
 "ASS_SHR",
 "ASS_SUB",
 "ASS_XOR",
 "DOT_STAR",
 "ELLIPSIS",
 "SCOPE",
] + list(reserved.values())

t_ignore = " \t\r"

t_LBRACE = r"(\{)|(<%)"
t_RBRACE = r"(\})|(%>)"
t_LBRACKET = r"(\[)|(<:)"
t_RBRACKET = r"(\])|(:>)"
t_ARROW = r"->"
t_ARROW_STAR = r"->\*"
t_DEC = r"--"
t_EQ = r"=="
t_GE = r">="
t_INC = r"\+\+"
t_LE = r"<="
t_LOG_AND = r"&&"
t_LOG_OR = r"\|\|"
t_NE = r"!="
t_SHL = r"<<"
t_SHR = r">>"
t_ASS_ADD = r"\+="
t_ASS_AND = r"&="
t_ASS_DIV = r"/="
t_ASS_MOD = r"%="
t_ASS_MUL = r"\*="
t_ASS_OR  = r"\|="
t_ASS_SHL = r"<<="
t_ASS_SHR = r">>="
t_ASS_SUB = r"-="
t_ASS_XOR = r"^="
t_DOT_STAR = r"\.\*"
t_ELLIPSIS = r"\.\.\."
t_SCOPE = r"::"

# Discard comments
def t_COMMENT(t):
    r'(/\*(.|\n)*?\*/)|(//.*?\n)|(\#.*?\n)'
    t.lexer.lineno += t.value.count("\n")

t_IntegerLiteral = r'(0x[0-9A-F]+)|([0-9]+(L){0,1})'
t_FloatingLiteral = r"[0-9]+[eE\.\+-]+[eE\.\+\-0-9]+"
t_CharacterLiteral = r'\'([^\'\\]|\\.)*\''
#t_StringLiteral = r'"([^"\\]|\\.)*"'
def t_StringLiteral(t):
    r'"([^"\\]|\\.)*"'
    t.type = reserved.get(t.value,'StringLiteral')
    return t

def t_Identifier(t):
    r"[a-zA-Z_][a-zA-Z_0-9\.]*"
    t.type = reserved.get(t.value,'Identifier')
    return t


def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    #raise IOError, "Parse error"
    #t.lexer.skip()

def t_newline(t):
    r'[\n]+'
    t.lexer.lineno += len(t.value)

precedence = (
    ( 'right', 'SHIFT_THERE', 'REDUCE_HERE_MOSTLY', 'SCOPE'),
    ( 'nonassoc', 'ELSE', 'INC', 'DEC', '+', '-', '*', '&', 'LBRACKET', 'LBRACE', '<', ':', ')')
    )

start = 'translation_unit'

#
#  The %prec resolves the 14.2-3 ambiguity:
#  Identifier '<' is forced to go through the is-it-a-template-name test
#  All names absorb TEMPLATE with the name, so that no template_test is
#  performed for them.  This requires all potential declarations within an
#  expression to perpetuate this policy and thereby guarantee the ultimate
#  coverage of explicit_instantiation.
#
#  The %prec also resolves a conflict in identifier : which is forced to be a
#  shift of a label for a labeled-statement rather than a reduction for the
#  name of a bit-field or generalised constructor.  This is pretty dubious
#  syntactically but correct for all semantic possibilities.  The shift is
#  only activated when the ambiguity exists at the start of a statement.
#  In this context a bit-field declaration or constructor definition are not
#  allowed.
#

def p_identifier(p):
    '''identifier : Identifier
    |               CXXTEST_STD '(' Identifier ')'
    '''
    if p[1][0] in ('t','T','c','d'):
        identifier_lineno[p[1]] = p.lineno(1)
    p[0] = p[1]

def p_id(p):
    '''id :                         identifier %prec SHIFT_THERE
    |                               template_decl
    |                               TEMPLATE id
    '''
    p[0] = get_rest(p)

def p_global_scope(p):
    '''global_scope :               SCOPE
    '''
    p[0] = get_rest(p)

def p_id_scope(p):
    '''id_scope : id SCOPE'''
    p[0] = get_rest(p)

def p_id_scope_seq(p):
    '''id_scope_seq :                id_scope
    |                                id_scope id_scope_seq
    '''
    p[0] = get_rest(p)

#
#  A :: B :: C; is ambiguous How much is type and how much name ?
#  The %prec maximises the (type) length which is the 7.1-2 semantic constraint.
#
def p_nested_id(p):
    '''nested_id :                  id %prec SHIFT_THERE
    |                               id_scope nested_id
    '''
    p[0] = get_rest(p)

def p_scoped_id(p):
    '''scoped_id :                  nested_id
    |                               global_scope nested_id
    |                               id_scope_seq
    |                               global_scope id_scope_seq
    '''
    global scope_lineno
    scope_lineno = lexer.lineno
    data = flatten(get_rest(p))
    if data[0] != None:
        p[0] = "".join(data)

#
#  destructor_id has to be held back to avoid a conflict with a one's
#  complement as per 5.3.1-9, It gets put back only when scoped or in a
#  declarator_id, which is only used as an explicit member name.
#  Declarations of an unscoped destructor are always parsed as a one's
#  complement.
#
def p_destructor_id(p):
    '''destructor_id :              '~' id
    |                               TEMPLATE destructor_id
    '''
    p[0]=get_rest(p)

#def p_template_id(p):
#    '''template_id :                empty
#    |                               TEMPLATE
#    '''
#    pass

def p_template_decl(p):
    '''template_decl :              identifier '<' nonlgt_seq_opt '>'
    '''
    #
    # WEH: should we include the lt/gt symbols to indicate that this is a
    # template class?  How is that going to be used later???
    #
    #p[0] = [p[1] ,"<",">"]
    p[0] = p[1]

def p_special_function_id(p):
    '''special_function_id :        conversion_function_id
    |                               operator_function_id
    |                               TEMPLATE special_function_id
    '''
    p[0]=get_rest(p)

def p_nested_special_function_id(p):
    '''nested_special_function_id : special_function_id
    |                               id_scope destructor_id
    |                               id_scope nested_special_function_id
    '''
    p[0]=get_rest(p)

def p_scoped_special_function_id(p):
    '''scoped_special_function_id : nested_special_function_id
    |                               global_scope nested_special_function_id
    '''
    p[0]=get_rest(p)

# declarator-id is all names in all scopes, except reserved words
def p_declarator_id(p):
    '''declarator_id :              scoped_id
    |                               scoped_special_function_id
    |                               destructor_id
    '''
    p[0]=p[1]

#
# The standard defines pseudo-destructors in terms of type-name, which is
# class/enum/typedef, of which class-name is covered by a normal destructor.
# pseudo-destructors are supposed to support ~int() in templates, so the
# grammar here covers built-in names. Other names are covered by the lack
# of identifier/type discrimination.
#
def p_built_in_type_id(p):
    '''built_in_type_id :           built_in_type_specifier
    |                               built_in_type_id built_in_type_specifier
    '''
    pass

def p_pseudo_destructor_id(p):
    '''pseudo_destructor_id :       built_in_type_id SCOPE '~' built_in_type_id
    |                               '~' built_in_type_id
    |                               TEMPLATE pseudo_destructor_id
    '''
    pass

def p_nested_pseudo_destructor_id(p):
    '''nested_pseudo_destructor_id : pseudo_destructor_id
    |                               id_scope nested_pseudo_destructor_id
    '''
    pass

def p_scoped_pseudo_destructor_id(p):
    '''scoped_pseudo_destructor_id : nested_pseudo_destructor_id
    |                               global_scope scoped_pseudo_destructor_id
    '''
    pass

#-------------------------------------------------------------------------------
# A.2 Lexical conventions
#-------------------------------------------------------------------------------
#

def p_literal(p):
    '''literal :                    IntegerLiteral
    |                               CharacterLiteral
    |                               FloatingLiteral
    |                               StringLiteral
    |                               TRUE
    |                               FALSE
    '''
    pass

#-------------------------------------------------------------------------------
# A.3 Basic concepts
#-------------------------------------------------------------------------------
def p_translation_unit(p):
    '''translation_unit :           declaration_seq_opt
    '''
    pass

#-------------------------------------------------------------------------------
# A.4 Expressions
#-------------------------------------------------------------------------------
#
#  primary_expression covers an arbitrary sequence of all names with the
#  exception of an unscoped destructor, which is parsed as its unary expression
#  which is the correct disambiguation (when ambiguous).  This eliminates the
#  traditional A(B) meaning A B ambiguity, since we never have to tack an A
#  onto the front of something that might start with (. The name length got
#  maximised ab initio. The downside is that semantic interpretation must split
#  the names up again.
#
#  Unification of the declaration and expression syntax means that unary and
#  binary pointer declarator operators:
#      int * * name
#  are parsed as binary and unary arithmetic operators (int) * (*name). Since
#  type information is not used
#  ambiguities resulting from a cast
#      (cast)*(value)
#  are resolved to favour the binary rather than the cast unary to ease AST
#  clean-up. The cast-call ambiguity must be resolved to the cast to ensure
#  that (a)(b)c can be parsed.
#
#  The problem of the functional cast ambiguity
#      name(arg)
#  as call or declaration is avoided by maximising the name within the parsing
#  kernel. So  primary_id_expression picks up
#      extern long int const var = 5;
#  as an assignment to the syntax parsed as "extern long int const var". The
#  presence of two names is parsed so that "extern long into const" is
#  distinguished from "var" considerably simplifying subsequent
#  semantic resolution.
#
#  The generalised name is a concatenation of potential type-names (scoped
#  identifiers or built-in sequences) plus optionally one of the special names
#  such as an operator-function-id, conversion-function-id or destructor as the
#  final name.
#

def get_rest(p):
    return [p[i] for i in range(1, len(p))]

def p_primary_expression(p):
    '''primary_expression :         literal
    |                               THIS
    |                               suffix_decl_specified_ids
    |                               abstract_expression %prec REDUCE_HERE_MOSTLY
    '''
    p[0] = get_rest(p)

#
#  Abstract-expression covers the () and [] of abstract-declarators.
#
def p_abstract_expression(p):
    '''abstract_expression :        parenthesis_clause
    |                               LBRACKET bexpression_opt RBRACKET
    |                               TEMPLATE abstract_expression
    '''
    pass

def p_postfix_expression(p):
    '''postfix_expression :         primary_expression
    |                               postfix_expression parenthesis_clause
    |                               postfix_expression LBRACKET bexpression_opt RBRACKET
    |                               postfix_expression LBRACKET bexpression_opt RBRACKET attributes
    |                               postfix_expression '.' declarator_id
    |                               postfix_expression '.' scoped_pseudo_destructor_id
    |                               postfix_expression ARROW declarator_id
    |                               postfix_expression ARROW scoped_pseudo_destructor_id
    |                               postfix_expression INC
    |                               postfix_expression DEC
    |                               DYNAMIC_CAST '<' nonlgt_seq_opt '>' '(' expression ')'
    |                               STATIC_CAST '<' nonlgt_seq_opt '>' '(' expression ')'
    |                               REINTERPRET_CAST '<' nonlgt_seq_opt '>' '(' expression ')'
    |                               CONST_CAST '<' nonlgt_seq_opt '>' '(' expression ')'
    |                               TYPEID parameters_clause
    '''
    #print "HERE",str(p[1])
    p[0] = get_rest(p)

def p_bexpression_opt(p):
    '''bexpression_opt :            empty
    |                               bexpression
    '''
    pass

def p_bexpression(p):
    '''bexpression :                nonbracket_seq
    |                               nonbracket_seq bexpression_seq bexpression_clause nonbracket_seq_opt
    |                               bexpression_seq bexpression_clause nonbracket_seq_opt
    '''
    pass

def p_bexpression_seq(p):
    '''bexpression_seq :            empty
    |                               bexpression_seq bexpression_clause nonbracket_seq_opt
    '''
    pass

def p_bexpression_clause(p):
    '''bexpression_clause :          LBRACKET bexpression_opt RBRACKET
    '''
    pass



def p_expression_list_opt(p):
    '''expression_list_opt :        empty
    |                               expression_list
    '''
    pass

def p_expression_list(p):
    '''expression_list :            assignment_expression
    |                               expression_list ',' assignment_expression
    '''
    pass

def p_unary_expression(p):
    '''unary_expression :           postfix_expression
    |                               INC cast_expression
    |                               DEC cast_expression
    |                               ptr_operator cast_expression
    |                               suffix_decl_specified_scope star_ptr_operator cast_expression
    |                               '+' cast_expression
    |                               '-' cast_expression
    |                               '!' cast_expression
    |                               '~' cast_expression
    |                               SIZEOF unary_expression
    |                               new_expression
    |                               global_scope new_expression
    |                               delete_expression
    |                               global_scope delete_expression
    '''
    p[0] = get_rest(p)

def p_delete_expression(p):
    '''delete_expression :          DELETE cast_expression
    '''
    pass

def p_new_expression(p):
    '''new_expression :             NEW new_type_id new_initializer_opt
    |                               NEW parameters_clause new_type_id new_initializer_opt
    |                               NEW parameters_clause
    |                               NEW parameters_clause parameters_clause new_initializer_opt
    '''
    pass

def p_new_type_id(p):
    '''new_type_id :                type_specifier ptr_operator_seq_opt
    |                               type_specifier new_declarator
    |                               type_specifier new_type_id
    '''
    pass

def p_new_declarator(p):
    '''new_declarator :             ptr_operator new_declarator
    |                               direct_new_declarator
    '''
    pass

def p_direct_new_declarator(p):
    '''direct_new_declarator :      LBRACKET bexpression_opt RBRACKET
    |                               direct_new_declarator LBRACKET bexpression RBRACKET
    '''
    pass

def p_new_initializer_opt(p):
    '''new_initializer_opt :        empty
    |                               '(' expression_list_opt ')'
    '''
    pass

#
# cast-expression is generalised to support a [] as well as a () prefix. This covers the omission of
# DELETE[] which when followed by a parenthesised expression was ambiguous. It also covers the gcc
# indexed array initialisation for free.
#
def p_cast_expression(p):
    '''cast_expression :            unary_expression
    |                               abstract_expression cast_expression
    '''
    p[0] = get_rest(p)

def p_pm_expression(p):
    '''pm_expression :              cast_expression
    |                               pm_expression DOT_STAR cast_expression
    |                               pm_expression ARROW_STAR cast_expression
    '''
    p[0] = get_rest(p)

def p_multiplicative_expression(p):
    '''multiplicative_expression :  pm_expression
    |                               multiplicative_expression star_ptr_operator pm_expression
    |                               multiplicative_expression '/' pm_expression
    |                               multiplicative_expression '%' pm_expression
    '''
    p[0] = get_rest(p)

def p_additive_expression(p):
    '''additive_expression :        multiplicative_expression
    |                               additive_expression '+' multiplicative_expression
    |                               additive_expression '-' multiplicative_expression
    '''
    p[0] = get_rest(p)

def p_shift_expression(p):
    '''shift_expression :           additive_expression
    |                               shift_expression SHL additive_expression
    |                               shift_expression SHR additive_expression
    '''
    p[0] = get_rest(p)

#    |                               relational_expression '<' shift_expression
#    |                               relational_expression '>' shift_expression
#    |                               relational_expression LE shift_expression
#    |                               relational_expression GE shift_expression
def p_relational_expression(p):
    '''relational_expression :      shift_expression
    '''
    p[0] = get_rest(p)

def p_equality_expression(p):
    '''equality_expression :        relational_expression
    |                               equality_expression EQ relational_expression
    |                               equality_expression NE relational_expression
    '''
    p[0] = get_rest(p)

def p_and_expression(p):
    '''and_expression :             equality_expression
    |                               and_expression '&' equality_expression
    '''
    p[0] = get_rest(p)

def p_exclusive_or_expression(p):
    '''exclusive_or_expression :    and_expression
    |                               exclusive_or_expression '^' and_expression
    '''
    p[0] = get_rest(p)

def p_inclusive_or_expression(p):
    '''inclusive_or_expression :    exclusive_or_expression
    |                               inclusive_or_expression '|' exclusive_or_expression
    '''
    p[0] = get_rest(p)

def p_logical_and_expression(p):
    '''logical_and_expression :     inclusive_or_expression
    |                               logical_and_expression LOG_AND inclusive_or_expression
    '''
    p[0] = get_rest(p)

def p_logical_or_expression(p):
    '''logical_or_expression :      logical_and_expression
    |                               logical_or_expression LOG_OR logical_and_expression
    '''
    p[0] = get_rest(p)

def p_conditional_expression(p):
    '''conditional_expression :     logical_or_expression
    |                               logical_or_expression '?' expression ':' assignment_expression
    '''
    p[0] = get_rest(p)


#
# assignment-expression is generalised to cover the simple assignment of a braced initializer in order to
# contribute to the coverage of parameter-declaration and init-declaration.
#
#    |                               logical_or_expression assignment_operator assignment_expression
def p_assignment_expression(p):
    '''assignment_expression :      conditional_expression
    |                               logical_or_expression assignment_operator nonsemicolon_seq
    |                               logical_or_expression '=' braced_initializer
    |                               throw_expression
    '''
    p[0]=get_rest(p)

def p_assignment_operator(p):
    '''assignment_operator :        '='
                           | ASS_ADD
                           | ASS_AND
                           | ASS_DIV
                           | ASS_MOD
                           | ASS_MUL
                           | ASS_OR
                           | ASS_SHL
                           | ASS_SHR
                           | ASS_SUB
                           | ASS_XOR
    '''
    pass

#
# expression is widely used and usually single-element, so the reductions are arranged so that a
# single-element expression is returned as is. Multi-element expressions are parsed as a list that
# may then behave polymorphically as an element or be compacted to an element.
#

def p_expression(p):
    '''expression :                 assignment_expression
    |                               expression_list ',' assignment_expression
    '''
    p[0] = get_rest(p)

def p_constant_expression(p):
    '''constant_expression :        conditional_expression
    '''
    pass

#---------------------------------------------------------------------------------------------------
# A.5 Statements
#---------------------------------------------------------------------------------------------------
# Parsing statements is easy once simple_declaration has been generalised to cover expression_statement.
#
#
# The use of extern here is a hack.  The 'extern "C" {}' block gets parsed
# as a function, so when nested 'extern "C"' declarations exist, they don't
# work because the block is viewed as a list of statements... :(
#
def p_statement(p):
    '''statement :                  compound_statement
    |                               declaration_statement
    |                               try_block
    |                               labeled_statement
    |                               selection_statement
    |                               iteration_statement
    |                               jump_statement
    '''
    pass

def p_compound_statement(p):
    '''compound_statement :         LBRACE statement_seq_opt RBRACE
    '''
    pass

def p_statement_seq_opt(p):
    '''statement_seq_opt :          empty
    |                               statement_seq_opt statement
    '''
    pass

#
#  The dangling else conflict is resolved to the innermost if.
#
def p_selection_statement(p):
    '''selection_statement :        IF '(' condition ')' statement    %prec SHIFT_THERE
    |                               IF '(' condition ')' statement ELSE statement
    |                               SWITCH '(' condition ')' statement
    '''
    pass

def p_condition_opt(p):
    '''condition_opt :              empty
    |                               condition
    '''
    pass

def p_condition(p):
    '''condition :                  nonparen_seq
    |                               nonparen_seq condition_seq parameters_clause nonparen_seq_opt
    |                               condition_seq parameters_clause nonparen_seq_opt
    '''
    pass

def p_condition_seq(p):
    '''condition_seq :              empty
    |                               condition_seq parameters_clause nonparen_seq_opt
    '''
    pass

def p_labeled_statement(p):
    '''labeled_statement :          identifier ':' statement
    |                               CASE constant_expression ':' statement
    |                               DEFAULT ':' statement
    '''
    pass

def p_try_block(p):
    '''try_block :                  TRY compound_statement handler_seq
    '''
    global noExceptionLogic
    noExceptionLogic=False

def p_jump_statement(p):
    '''jump_statement :             BREAK ';'
    |                               CONTINUE ';'
    |                               RETURN nonsemicolon_seq ';'
    |                               GOTO identifier ';'
    '''
    pass

def p_iteration_statement(p):
    '''iteration_statement :        WHILE '(' condition ')' statement
    |                               DO statement WHILE '(' expression ')' ';'
    |                               FOR '(' nonparen_seq_opt ')' statement
    '''
    pass

def p_declaration_statement(p):
    '''declaration_statement :      block_declaration
    '''
    pass

#---------------------------------------------------------------------------------------------------
# A.6 Declarations
#---------------------------------------------------------------------------------------------------
def p_compound_declaration(p):
    '''compound_declaration :       LBRACE declaration_seq_opt RBRACE
    '''
    pass

def p_declaration_seq_opt(p):
    '''declaration_seq_opt :        empty
    |                               declaration_seq_opt declaration
    '''
    pass

def p_declaration(p):
    '''declaration :                block_declaration
    |                               function_definition
    |                               template_declaration
    |                               explicit_specialization
    |                               specialised_declaration
    '''
    pass

def p_specialised_declaration(p):
    '''specialised_declaration :    linkage_specification
    |                               namespace_definition
    |                               TEMPLATE specialised_declaration
    '''
    pass

def p_block_declaration(p):
    '''block_declaration :          simple_declaration
    |                               specialised_block_declaration
    '''
    pass

def p_specialised_block_declaration(p):
    '''specialised_block_declaration :      asm_definition
    |                               namespace_alias_definition
    |                               using_declaration
    |                               using_directive
    |                               TEMPLATE specialised_block_declaration
    '''
    pass

def p_simple_declaration(p):
    '''simple_declaration :         ';'
    |                               init_declaration ';'
    |                               init_declarations ';'
    |                               decl_specifier_prefix simple_declaration
    '''
    global _parse_info
    if len(p) == 3:
        if p[2] == ";":
            decl = p[1]
        else:
            decl = p[2]
        if decl is not None:
            fp = flatten(decl)
            if len(fp) >= 2 and fp[0] is not None and fp[0]!="operator" and fp[1] == '(':
                p[0] = fp[0]
                _parse_info.add_function(fp[0])

#
#  A decl-specifier following a ptr_operator provokes a shift-reduce conflict for * const name which is resolved in favour of the pointer, and implemented by providing versions of decl-specifier guaranteed not to start with a cv_qualifier.  decl-specifiers are implemented type-centrically. That is the semantic constraint that there must be a type is exploited to impose structure, but actually eliminate very little syntax. built-in types are multi-name and so need a different policy.
#
#  non-type decl-specifiers are bound to the left-most type in a decl-specifier-seq, by parsing from the right and attaching suffixes to the right-hand type. Finally residual prefixes attach to the left.
#
def p_suffix_built_in_decl_specifier_raw(p):
    '''suffix_built_in_decl_specifier_raw : built_in_type_specifier
    |                               suffix_built_in_decl_specifier_raw built_in_type_specifier
    |                               suffix_built_in_decl_specifier_raw decl_specifier_suffix
    '''
    pass

def p_suffix_built_in_decl_specifier(p):
    '''suffix_built_in_decl_specifier :     suffix_built_in_decl_specifier_raw
    |                               TEMPLATE suffix_built_in_decl_specifier
    '''
    pass

#    |                                       id_scope_seq
#    |                                       SCOPE id_scope_seq
def p_suffix_named_decl_specifier(p):
    '''suffix_named_decl_specifier :        scoped_id
    |                               elaborate_type_specifier
    |                               suffix_named_decl_specifier decl_specifier_suffix
    '''
    p[0]=get_rest(p)

def p_suffix_named_decl_specifier_bi(p):
    '''suffix_named_decl_specifier_bi :     suffix_named_decl_specifier
    |                               suffix_named_decl_specifier suffix_built_in_decl_specifier_raw
    '''
    p[0] = get_rest(p)
    #print "HERE",get_rest(p)

def p_suffix_named_decl_specifiers(p):
    '''suffix_named_decl_specifiers :       suffix_named_decl_specifier_bi
    |                               suffix_named_decl_specifiers suffix_named_decl_specifier_bi
    '''
    p[0] = get_rest(p)

def p_suffix_named_decl_specifiers_sf(p):
    '''suffix_named_decl_specifiers_sf :    scoped_special_function_id
    |                               suffix_named_decl_specifiers
    |                               suffix_named_decl_specifiers scoped_special_function_id
    '''
    #print "HERE",get_rest(p)
    p[0] = get_rest(p)

def p_suffix_decl_specified_ids(p):
    '''suffix_decl_specified_ids :          suffix_built_in_decl_specifier
    |                               suffix_built_in_decl_specifier suffix_named_decl_specifiers_sf
    |                               suffix_named_decl_specifiers_sf
    '''
    if len(p) == 3:
        p[0] = p[2]
    else:
        p[0] = p[1]

def p_suffix_decl_specified_scope(p):
    '''suffix_decl_specified_scope : suffix_named_decl_specifiers SCOPE
    |                               suffix_built_in_decl_specifier suffix_named_decl_specifiers SCOPE
    |                               suffix_built_in_decl_specifier SCOPE
    '''
    p[0] = get_rest(p)

def p_decl_specifier_affix(p):
    '''decl_specifier_affix :       storage_class_specifier
    |                               function_specifier
    |                               FRIEND
    |                               TYPEDEF
    |                               cv_qualifier
    '''
    pass

def p_decl_specifier_suffix(p):
    '''decl_specifier_suffix :      decl_specifier_affix
    '''
    pass

def p_decl_specifier_prefix(p):
    '''decl_specifier_prefix :      decl_specifier_affix
    |                               TEMPLATE decl_specifier_prefix
    '''
    pass

def p_storage_class_specifier(p):
    '''storage_class_specifier :    REGISTER
    |                               STATIC
    |                               MUTABLE
    |                               EXTERN                  %prec SHIFT_THERE
    |                               EXTENSION
    |                               AUTO
    '''
    pass

def p_function_specifier(p):
    '''function_specifier :         EXPLICIT
    |                               INLINE
    |                               VIRTUAL
    '''
    pass

def p_type_specifier(p):
    '''type_specifier :             simple_type_specifier
    |                               elaborate_type_specifier
    |                               cv_qualifier
    '''
    pass

def p_elaborate_type_specifier(p):
    '''elaborate_type_specifier :   class_specifier
    |                               enum_specifier
    |                               elaborated_type_specifier
    |                               TEMPLATE elaborate_type_specifier
    '''
    pass

def p_simple_type_specifier(p):
    '''simple_type_specifier :      scoped_id
    |                               scoped_id attributes
    |                               built_in_type_specifier
    '''
    p[0] = p[1]

def p_built_in_type_specifier(p):
    '''built_in_type_specifier : Xbuilt_in_type_specifier
    |                            Xbuilt_in_type_specifier attributes
    '''
    pass

def p_attributes(p):
    '''attributes :                 attribute
    |                               attributes attribute
    '''
    pass

def p_attribute(p):
    '''attribute :                  ATTRIBUTE '(' parameters_clause ')'
    '''

def p_Xbuilt_in_type_specifier(p):
    '''Xbuilt_in_type_specifier :    CHAR
    | WCHAR_T
    | BOOL
    | SHORT
    | INT
    | LONG
    | SIGNED
    | UNSIGNED
    | FLOAT
    | DOUBLE
    | VOID
    | uTYPEOF parameters_clause
    | TYPEOF parameters_clause
    '''
    pass

#
#  The over-general use of declaration_expression to cover decl-specifier-seq_opt declarator in a function-definition means that
#      class X { };
#  could be a function-definition or a class-specifier.
#      enum X { };
#  could be a function-definition or an enum-specifier.
#  The function-definition is not syntactically valid so resolving the false conflict in favour of the
#  elaborated_type_specifier is correct.
#
def p_elaborated_type_specifier(p):
    '''elaborated_type_specifier :  class_key scoped_id %prec SHIFT_THERE
    |                               elaborated_enum_specifier
    |                               TYPENAME scoped_id
    '''
    pass

def p_elaborated_enum_specifier(p):
    '''elaborated_enum_specifier :  ENUM scoped_id   %prec SHIFT_THERE
    '''
    pass

def p_enum_specifier(p):
    '''enum_specifier :             ENUM scoped_id enumerator_clause
    |                               ENUM enumerator_clause
    '''
    pass

def p_enumerator_clause(p):
    '''enumerator_clause :          LBRACE enumerator_list_ecarb
    |                               LBRACE enumerator_list enumerator_list_ecarb
    |                               LBRACE enumerator_list ',' enumerator_definition_ecarb
    '''
    pass

def p_enumerator_list_ecarb(p):
    '''enumerator_list_ecarb :      RBRACE
    '''
    pass

def p_enumerator_definition_ecarb(p):
    '''enumerator_definition_ecarb :        RBRACE
    '''
    pass

def p_enumerator_definition_filler(p):
    '''enumerator_definition_filler :       empty
    '''
    pass

def p_enumerator_list_head(p):
    '''enumerator_list_head :       enumerator_definition_filler
    |                               enumerator_list ',' enumerator_definition_filler
    '''
    pass

def p_enumerator_list(p):
    '''enumerator_list :            enumerator_list_head enumerator_definition
    '''
    pass

def p_enumerator_definition(p):
    '''enumerator_definition :      enumerator
    |                               enumerator '=' constant_expression
    '''
    pass

def p_enumerator(p):
    '''enumerator :                 identifier
    '''
    pass

def p_namespace_definition(p):
    '''namespace_definition :       NAMESPACE scoped_id push_scope compound_declaration
    |                               NAMESPACE push_scope compound_declaration
    '''
    global _parse_info
    scope = _parse_info.pop_scope()

def p_namespace_alias_definition(p):
    '''namespace_alias_definition : NAMESPACE scoped_id '=' scoped_id ';'
    '''
    pass

def p_push_scope(p):
    '''push_scope :                 empty'''
    global _parse_info
    if p[-2] == "namespace":
        scope=p[-1]
    else:
        scope=""
    _parse_info.push_scope(scope,"namespace")

def p_using_declaration(p):
    '''using_declaration :          USING declarator_id ';'
    |                               USING TYPENAME declarator_id ';'
    '''
    pass

def p_using_directive(p):
    '''using_directive :            USING NAMESPACE scoped_id ';'
    '''
    pass

#    '''asm_definition :             ASM '(' StringLiteral ')' ';'
def p_asm_definition(p):
    '''asm_definition :             ASM '(' nonparen_seq_opt ')' ';'
    '''
    pass

def p_linkage_specification(p):
    '''linkage_specification :      EXTERN CLiteral declaration
    |                               EXTERN CLiteral compound_declaration
    |                               EXTERN CppLiteral declaration
    |                               EXTERN CppLiteral compound_declaration
    '''
    pass

#---------------------------------------------------------------------------------------------------
# A.7 Declarators
#---------------------------------------------------------------------------------------------------
#
# init-declarator is named init_declaration to reflect the embedded decl-specifier-seq_opt
#

def p_init_declarations(p):
    '''init_declarations :          assignment_expression ',' init_declaration
    |                               init_declarations ',' init_declaration
    '''
    p[0]=get_rest(p)

def p_init_declaration(p):
    '''init_declaration :           assignment_expression
    '''
    p[0]=get_rest(p)

def p_star_ptr_operator(p):
    '''star_ptr_operator :          '*'
    |                               star_ptr_operator cv_qualifier
    '''
    pass

def p_nested_ptr_operator(p):
    '''nested_ptr_operator :        star_ptr_operator
    |                               id_scope nested_ptr_operator
    '''
    pass

def p_ptr_operator(p):
    '''ptr_operator :               '&'
    |                               nested_ptr_operator
    |                               global_scope nested_ptr_operator
    '''
    pass

def p_ptr_operator_seq(p):
    '''ptr_operator_seq :           ptr_operator
    |                               ptr_operator ptr_operator_seq
    '''
    pass

#
# Independently coded to localise the shift-reduce conflict: sharing just needs another %prec
#
def p_ptr_operator_seq_opt(p):
    '''ptr_operator_seq_opt :       empty %prec SHIFT_THERE
    |                               ptr_operator ptr_operator_seq_opt
    '''
    pass

def p_cv_qualifier_seq_opt(p):
    '''cv_qualifier_seq_opt :       empty
    |                               cv_qualifier_seq_opt cv_qualifier
    '''
    pass

# TODO: verify that we should include attributes here
def p_cv_qualifier(p):
    '''cv_qualifier :               CONST
    |                               VOLATILE
    |                               attributes
    '''
    pass

def p_type_id(p):
    '''type_id :                    type_specifier abstract_declarator_opt
    |                               type_specifier type_id
    '''
    pass

def p_abstract_declarator_opt(p):
    '''abstract_declarator_opt :    empty
    |                               ptr_operator abstract_declarator_opt
    |                               direct_abstract_declarator
    '''
    pass

def p_direct_abstract_declarator_opt(p):
    '''direct_abstract_declarator_opt :     empty
    |                               direct_abstract_declarator
    '''
    pass

def p_direct_abstract_declarator(p):
    '''direct_abstract_declarator : direct_abstract_declarator_opt parenthesis_clause
    |                               direct_abstract_declarator_opt LBRACKET RBRACKET
    |                               direct_abstract_declarator_opt LBRACKET bexpression RBRACKET
    '''
    pass

def p_parenthesis_clause(p):
    '''parenthesis_clause :         parameters_clause cv_qualifier_seq_opt
    |                               parameters_clause cv_qualifier_seq_opt exception_specification
    '''
    p[0] = ['(',')']

def p_parameters_clause(p):
    '''parameters_clause :          '(' condition_opt ')'
    '''
    p[0] = ['(',')']

#
# A typed abstract qualifier such as
#      Class * ...
# looks like a multiply, so pointers are parsed as their binary operation equivalents that
# ultimately terminate with a degenerate right hand term.
#
def p_abstract_pointer_declaration(p):
    '''abstract_pointer_declaration :       ptr_operator_seq
    |                               multiplicative_expression star_ptr_operator ptr_operator_seq_opt
    '''
    pass

def p_abstract_parameter_declaration(p):
    '''abstract_parameter_declaration :     abstract_pointer_declaration
    |                               and_expression '&'
    |                               and_expression '&' abstract_pointer_declaration
    '''
    pass

def p_special_parameter_declaration(p):
    '''special_parameter_declaration :      abstract_parameter_declaration
    |                               abstract_parameter_declaration '=' assignment_expression
    |                               ELLIPSIS
    '''
    pass

def p_parameter_declaration(p):
    '''parameter_declaration :      assignment_expression
    |                               special_parameter_declaration
    |                               decl_specifier_prefix parameter_declaration
    '''
    pass

#
# function_definition includes constructor, destructor, implicit int definitions too.  A local destructor is successfully parsed as a function-declaration but the ~ was treated as a unary operator.  constructor_head is the prefix ambiguity between a constructor and a member-init-list starting with a bit-field.
#
def p_function_definition(p):
    '''function_definition :        ctor_definition
    |                               func_definition
    '''
    pass

def p_func_definition(p):
    '''func_definition :            assignment_expression function_try_block
    |                               assignment_expression function_body
    |                               decl_specifier_prefix func_definition
    '''
    global _parse_info
    if p[2] is not None and p[2][0] == '{':
        decl = flatten(p[1])
        #print "HERE",decl
        if decl[-1] == ')':
            decl=decl[-3]
        else:
            decl=decl[-1]
        p[0] = decl
        if decl != "operator":
            _parse_info.add_function(decl)
    else:
        p[0] = p[2]

def p_ctor_definition(p):
    '''ctor_definition :            constructor_head function_try_block
    |                               constructor_head function_body
    |                               decl_specifier_prefix ctor_definition
    '''
    if p[2] is None or p[2][0] == "try" or p[2][0] == '{':
        p[0]=p[1]
    else:
        p[0]=p[1]

def p_constructor_head(p):
    '''constructor_head :           bit_field_init_declaration
    |                               constructor_head ',' assignment_expression
    '''
    p[0]=p[1]

def p_function_try_block(p):
    '''function_try_block :         TRY function_block handler_seq
    '''
    global noExceptionLogic
    noExceptionLogic=False
    p[0] = ['try']

def p_function_block(p):
    '''function_block :             ctor_initializer_opt function_body
    '''
    pass

def p_function_body(p):
    '''function_body :              LBRACE nonbrace_seq_opt RBRACE
    '''
    p[0] = ['{','}']

def p_initializer_clause(p):
    '''initializer_clause :         assignment_expression
    |                               braced_initializer
    '''
    pass

def p_braced_initializer(p):
    '''braced_initializer :         LBRACE initializer_list RBRACE
    |                               LBRACE initializer_list ',' RBRACE
    |                               LBRACE RBRACE
    '''
    pass

def p_initializer_list(p):
    '''initializer_list :           initializer_clause
    |                               initializer_list ',' initializer_clause
    '''
    pass

#---------------------------------------------------------------------------------------------------
# A.8 Classes
#---------------------------------------------------------------------------------------------------
#
#  An anonymous bit-field declaration may look very like inheritance:
#      const int B = 3;
#      class A : B ;
#  The two usages are too distant to try to create and enforce a common prefix so we have to resort to
#  a parser hack by backtracking. Inheritance is much the most likely so we mark the input stream context
#  and try to parse a base-clause. If we successfully reach a { the base-clause is ok and inheritance was
#  the correct choice so we unmark and continue. If we fail to find the { an error token causes
#  back-tracking to the alternative parse in elaborated_type_specifier which regenerates the : and
#  declares unconditional success.
#

def p_class_specifier_head(p):
    '''class_specifier_head :       class_key scoped_id ':' base_specifier_list LBRACE
    |                               class_key ':' base_specifier_list LBRACE
    |                               class_key scoped_id LBRACE
    |                               class_key LBRACE
    '''
    global _parse_info
    base_classes=[]
    if len(p) == 6:
        scope = p[2]
        base_classes = p[4]
    elif len(p) == 4:
        scope = p[2]
    elif len(p) == 5:
        base_classes = p[3]
    else:
        scope = ""
    _parse_info.push_scope(scope,p[1],base_classes)


def p_class_key(p):
    '''class_key :                  CLASS
    | STRUCT
    | UNION
    '''
    p[0] = p[1]

def p_class_specifier(p):
    '''class_specifier :            class_specifier_head member_specification_opt RBRACE
    '''
    scope = _parse_info.pop_scope()

def p_member_specification_opt(p):
    '''member_specification_opt :   empty
    |                               member_specification_opt member_declaration
    '''
    pass

def p_member_declaration(p):
    '''member_declaration :         accessibility_specifier
    |                               simple_member_declaration
    |                               function_definition
    |                               using_declaration
    |                               template_declaration
    '''
    p[0] = get_rest(p)
    #print "Decl",get_rest(p)

#
#  The generality of constructor names (there need be no parenthesised argument list) means that that
#          name : f(g), h(i)
#  could be the start of a constructor or the start of an anonymous bit-field. An ambiguity is avoided by
#  parsing the ctor-initializer of a function_definition as a bit-field.
#
def p_simple_member_declaration(p):
    '''simple_member_declaration :  ';'
    |                               assignment_expression ';'
    |                               constructor_head ';'
    |                               member_init_declarations ';'
    |                               decl_specifier_prefix simple_member_declaration
    '''
    global _parse_info
    decl = flatten(get_rest(p))
    if len(decl) >= 4 and decl[-3] == "(":
        _parse_info.add_function(decl[-4])

def p_member_init_declarations(p):
    '''member_init_declarations :   assignment_expression ',' member_init_declaration
    |                               constructor_head ',' bit_field_init_declaration
    |                               member_init_declarations ',' member_init_declaration
    '''
    pass

def p_member_init_declaration(p):
    '''member_init_declaration :    assignment_expression
    |                               bit_field_init_declaration
    '''
    pass

def p_accessibility_specifier(p):
    '''accessibility_specifier :    access_specifier ':'
    '''
    pass

def p_bit_field_declaration(p):
    '''bit_field_declaration :      assignment_expression ':' bit_field_width
    |                               ':' bit_field_width
    '''
    if len(p) == 4:
        p[0]=p[1]

def p_bit_field_width(p):
    '''bit_field_width :            logical_or_expression
    |                               logical_or_expression '?' bit_field_width ':' bit_field_width
    '''
    pass

def p_bit_field_init_declaration(p):
    '''bit_field_init_declaration : bit_field_declaration
    |                               bit_field_declaration '=' initializer_clause
    '''
    pass

#---------------------------------------------------------------------------------------------------
# A.9 Derived classes
#---------------------------------------------------------------------------------------------------
def p_base_specifier_list(p):
    '''base_specifier_list :        base_specifier
    |                               base_specifier_list ',' base_specifier
    '''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1]+[p[3]]

def p_base_specifier(p):
    '''base_specifier :             scoped_id
    |                               access_specifier base_specifier
    |                               VIRTUAL base_specifier
    '''
    if len(p) == 2:
        p[0] = p[1]
    else:
        p[0] = p[2]

def p_access_specifier(p):
    '''access_specifier :           PRIVATE
    |                               PROTECTED
    |                               PUBLIC
    '''
    pass

#---------------------------------------------------------------------------------------------------
# A.10 Special member functions
#---------------------------------------------------------------------------------------------------
def p_conversion_function_id(p):
    '''conversion_function_id :     OPERATOR conversion_type_id
    '''
    p[0] = ['operator']

def p_conversion_type_id(p):
    '''conversion_type_id :         type_specifier ptr_operator_seq_opt
    |                               type_specifier conversion_type_id
    '''
    pass

#
#  Ctor-initialisers can look like a bit field declaration, given the generalisation of names:
#      Class(Type) : m1(1), m2(2) { }
#      NonClass(bit_field) : int(2), second_variable, ...
#  The grammar below is used within a function_try_block or function_definition.
#  See simple_member_declaration for use in normal member function_definition.
#
def p_ctor_initializer_opt(p):
    '''ctor_initializer_opt :       empty
    |                               ctor_initializer
    '''
    pass

def p_ctor_initializer(p):
    '''ctor_initializer :           ':' mem_initializer_list
    '''
    pass

def p_mem_initializer_list(p):
    '''mem_initializer_list :       mem_initializer
    |                               mem_initializer_list_head mem_initializer
    '''
    pass

def p_mem_initializer_list_head(p):
    '''mem_initializer_list_head :  mem_initializer_list ','
    '''
    pass

def p_mem_initializer(p):
    '''mem_initializer :            mem_initializer_id '(' expression_list_opt ')'
    '''
    pass

def p_mem_initializer_id(p):
    '''mem_initializer_id :         scoped_id
    '''
    pass

#---------------------------------------------------------------------------------------------------
# A.11 Overloading
#---------------------------------------------------------------------------------------------------

def p_operator_function_id(p):
    '''operator_function_id :       OPERATOR operator
    |                               OPERATOR '(' ')'
    |                               OPERATOR LBRACKET RBRACKET
    |                               OPERATOR '<'
    |                               OPERATOR '>'
    |                               OPERATOR operator '<' nonlgt_seq_opt '>'
    '''
    p[0] = ["operator"]

#
#  It is not clear from the ANSI standard whether spaces are permitted in delete[]. If not then it can
#  be recognised and returned as DELETE_ARRAY by the lexer. Assuming spaces are permitted there is an
#  ambiguity created by the over generalised nature of expressions. operator new is a valid delarator-id
#  which we may have an undimensioned array of. Semantic rubbish, but syntactically valid. Since the
#  array form is covered by the declarator consideration we can exclude the operator here. The need
#  for a semantic rescue can be eliminated at the expense of a couple of shift-reduce conflicts by
#  removing the comments on the next four lines.
#
def p_operator(p):
    '''operator :                   NEW
    |                               DELETE
    |                               '+'
    |                               '-'
    |                               '*'
    |                               '/'
    |                               '%'
    |                               '^'
    |                               '&'
    |                               '|'
    |                               '~'
    |                               '!'
    |                               '='
    |                               ASS_ADD
    |                               ASS_SUB
    |                               ASS_MUL
    |                               ASS_DIV
    |                               ASS_MOD
    |                               ASS_XOR
    |                               ASS_AND
    |                               ASS_OR
    |                               SHL
    |                               SHR
    |                               ASS_SHR
    |                               ASS_SHL
    |                               EQ
    |                               NE
    |                               LE
    |                               GE
    |                               LOG_AND
    |                               LOG_OR
    |                               INC
    |                               DEC
    |                               ','
    |                               ARROW_STAR
    |                               ARROW
    '''
    p[0]=p[1]

#    |                               IF
#    |                               SWITCH
#    |                               WHILE
#    |                               FOR
#    |                               DO
def p_reserved(p):
    '''reserved :                   PRIVATE
    |                               CLiteral
    |                               CppLiteral
    |                               IF
    |                               SWITCH
    |                               WHILE
    |                               FOR
    |                               DO
    |                               PROTECTED
    |                               PUBLIC
    |                               BOOL
    |                               CHAR
    |                               DOUBLE
    |                               FLOAT
    |                               INT
    |                               LONG
    |                               SHORT
    |                               SIGNED
    |                               UNSIGNED
    |                               VOID
    |                               WCHAR_T
    |                               CLASS
    |                               ENUM
    |                               NAMESPACE
    |                               STRUCT
    |                               TYPENAME
    |                               UNION
    |                               CONST
    |                               VOLATILE
    |                               AUTO
    |                               EXPLICIT
    |                               EXPORT
    |                               EXTERN
    |                               FRIEND
    |                               INLINE
    |                               MUTABLE
    |                               REGISTER
    |                               STATIC
    |                               TEMPLATE
    |                               TYPEDEF
    |                               USING
    |                               VIRTUAL
    |                               ASM
    |                               BREAK
    |                               CASE
    |                               CATCH
    |                               CONST_CAST
    |                               CONTINUE
    |                               DEFAULT
    |                               DYNAMIC_CAST
    |                               ELSE
    |                               FALSE
    |                               GOTO
    |                               OPERATOR
    |                               REINTERPRET_CAST
    |                               RETURN
    |                               SIZEOF
    |                               STATIC_CAST
    |                               THIS
    |                               THROW
    |                               TRUE
    |                               TRY
    |                               TYPEID
    |                               ATTRIBUTE
    |                               CDECL
    |                               TYPEOF
    |                               uTYPEOF
    '''
    if p[1] in ('try', 'catch', 'throw'):
        global noExceptionLogic
        noExceptionLogic=False

#---------------------------------------------------------------------------------------------------
# A.12 Templates
#---------------------------------------------------------------------------------------------------
def p_template_declaration(p):
    '''template_declaration :       template_parameter_clause declaration
    |                               EXPORT template_declaration
    '''
    pass

def p_template_parameter_clause(p):
    '''template_parameter_clause :  TEMPLATE '<' nonlgt_seq_opt '>'
    '''
    pass

#
#  Generalised naming makes identifier a valid declaration, so TEMPLATE identifier is too.
#  The TEMPLATE prefix is therefore folded into all names, parenthesis_clause and decl_specifier_prefix.
#
# explicit_instantiation:           TEMPLATE declaration
#
def p_explicit_specialization(p):
    '''explicit_specialization :    TEMPLATE '<' '>' declaration
    '''
    pass

#---------------------------------------------------------------------------------------------------
# A.13 Exception Handling
#---------------------------------------------------------------------------------------------------
def p_handler_seq(p):
    '''handler_seq :                handler
    |                               handler handler_seq
    '''
    pass

def p_handler(p):
    '''handler :                    CATCH '(' exception_declaration ')' compound_statement
    '''
    global noExceptionLogic
    noExceptionLogic=False

def p_exception_declaration(p):
    '''exception_declaration :      parameter_declaration
    '''
    pass

def p_throw_expression(p):
    '''throw_expression :           THROW
    |                               THROW assignment_expression
    '''
    global noExceptionLogic
    noExceptionLogic=False

def p_exception_specification(p):
    '''exception_specification :    THROW '(' ')'
    |                               THROW '(' type_id_list ')'
    '''
    global noExceptionLogic
    noExceptionLogic=False

def p_type_id_list(p):
    '''type_id_list :               type_id
    |                               type_id_list ',' type_id
    '''
    pass

#---------------------------------------------------------------------------------------------------
# Misc productions
#---------------------------------------------------------------------------------------------------
def p_nonsemicolon_seq(p):
    '''nonsemicolon_seq :           empty
    |                               nonsemicolon_seq nonsemicolon
    '''
    pass

def p_nonsemicolon(p):
    '''nonsemicolon :               misc
    |                               '('
    |                               ')'
    |                               '<'
    |                               '>'
    |                               LBRACKET nonbracket_seq_opt RBRACKET
    |                               LBRACE nonbrace_seq_opt RBRACE
    '''
    pass

def p_nonparen_seq_opt(p):
    '''nonparen_seq_opt :           empty
    |                               nonparen_seq_opt nonparen
    '''
    pass

def p_nonparen_seq(p):
    '''nonparen_seq :               nonparen
    |                               nonparen_seq nonparen
    '''
    pass

def p_nonparen(p):
    '''nonparen :                   misc
    |                               '<'
    |                               '>'
    |                               ';'
    |                               LBRACKET nonbracket_seq_opt RBRACKET
    |                               LBRACE nonbrace_seq_opt RBRACE
    '''
    pass

def p_nonbracket_seq_opt(p):
    '''nonbracket_seq_opt :         empty
    |                               nonbracket_seq_opt nonbracket
    '''
    pass

def p_nonbracket_seq(p):
    '''nonbracket_seq :             nonbracket
    |                               nonbracket_seq nonbracket
    '''
    pass

def p_nonbracket(p):
    '''nonbracket :                 misc
    |                               '<'
    |                               '>'
    |                               '('
    |                               ')'
    |                               ';'
    |                               LBRACKET nonbracket_seq_opt RBRACKET
    |                               LBRACE nonbrace_seq_opt RBRACE
    '''
    pass

def p_nonbrace_seq_opt(p):
    '''nonbrace_seq_opt :           empty
    |                               nonbrace_seq_opt nonbrace
    '''
    pass

def p_nonbrace(p):
    '''nonbrace :                   misc
    |                               '<'
    |                               '>'
    |                               '('
    |                               ')'
    |                               ';'
    |                               LBRACKET nonbracket_seq_opt RBRACKET
    |                               LBRACE nonbrace_seq_opt RBRACE
    '''
    pass

def p_nonlgt_seq_opt(p):
    '''nonlgt_seq_opt :             empty
    |                               nonlgt_seq_opt nonlgt
    '''
    pass

def p_nonlgt(p):
    '''nonlgt :                     misc
    |                               '('
    |                               ')'
    |                               LBRACKET nonbracket_seq_opt RBRACKET
    |                               '<' nonlgt_seq_opt '>'
    |                               ';'
    '''
    pass

def p_misc(p):
    '''misc :                       operator
    |                               identifier
    |                               IntegerLiteral
    |                               CharacterLiteral
    |                               FloatingLiteral
    |                               StringLiteral
    |                               reserved
    |                               '?'
    |                               ':'
    |                               '.'
    |                               SCOPE
    |                               ELLIPSIS
    |                               EXTENSION
    '''
    pass

def p_empty(p):
    '''empty : '''
    pass



#
# Compute column.
#     input is the input text string
#     token is a token instance
#
def _find_column(input,token):
    ''' TODO '''
    i = token.lexpos
    while i > 0:
        if input[i] == '\n': break
        i -= 1
    column = (token.lexpos - i)+1
    return column

def p_error(p):
    if p is None:
        tmp = "Syntax error at end of file."
    else:
        tmp = "Syntax error at token "
        if p.type == "":
            tmp = tmp + "''"
        else:
            tmp = tmp + str(p.type)
        tmp = tmp + " with value '"+str(p.value)+"'"
        tmp = tmp + " in line " + str(lexer.lineno-1)
        tmp = tmp + " at column "+str(_find_column(_parsedata,p))
    raise IOError( tmp )



#
# The function that performs the parsing
#
def parse_cpp(data=None, filename=None, debug=0, optimize=0, verbose=False, func_filter=None):
    #
    # Reset global data
    #
    global lexer
    lexer = None
    global scope_lineno
    scope_lineno = 0
    global indentifier_lineno
    identifier_lineno = {}
    global _parse_info
    _parse_info=None
    global _parsedata
    _parsedata=None
    global noExceptionLogic
    noExceptionLogic = True
    #
    if debug > 0:
        print("Debugging parse_cpp!")
        #
        # Always remove the parser.out file, which is generated to create debugging
        #
        if os.path.exists("parser.out"):
            os.remove("parser.out")
        #
        # Remove the parsetab.py* files.  These apparently need to be removed
        # to ensure the creation of a parser.out file.
        #
        if os.path.exists("parsetab.py"):
           os.remove("parsetab.py")
        if os.path.exists("parsetab.pyc"):
           os.remove("parsetab.pyc")
        global debugging
        debugging=True
    #
    # Build lexer
    #
    lexer = lex.lex()
    #
    # Initialize parse object
    #
    _parse_info = CppInfo(filter=func_filter)
    _parse_info.verbose=verbose
    #
    # Build yaccer
    #
    write_table = not os.path.exists("parsetab.py")
    yacc.yacc(debug=debug, optimize=optimize, write_tables=write_table)
    #
    # Parse the file
    #
    if not data is None:
        _parsedata=data
        ply_init(_parsedata)
        yacc.parse(data,debug=debug)
    elif not filename is None:
        f = open(filename)
        data = f.read()
        f.close()
        _parsedata=data
        ply_init(_parsedata)
        yacc.parse(data, debug=debug)
    else:
        return None
    #
    if not noExceptionLogic:
        _parse_info.noExceptionLogic = False
    else:
        for key in identifier_lineno:
            if 'ASSERT_THROWS' in key:
                _parse_info.noExceptionLogic = False
                break
        _parse_info.noExceptionLogic = True
    #
    return _parse_info



import sys

if __name__ == '__main__':  #pragma: no cover
    #
    # This MAIN routine parses a sequence of files provided at the command
    # line.  If '-v' is included, then a verbose parsing output is
    # generated.
    #
    for arg in sys.argv[1:]:
        if arg == "-v":
            continue
        print("Parsing file '"+arg+"'")
        if '-v' in sys.argv:
            parse_cpp(filename=arg,debug=2,verbose=2)
        else:
            parse_cpp(filename=arg,verbose=2)
        #
        # Print the _parse_info object summary for this file.
        # This illustrates how class inheritance can be used to
        # deduce class members.
        #
        print(str(_parse_info))
