/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         Aqlparse
#define yylex           Aqllex
#define yyerror         Aqlerror
#define yydebug         Aqldebug
#define yynerrs         Aqlnerrs


/* Copy the first part of user declarations.  */
#line 9 "arangod/Aql/grammar.y" /* yacc.c:339  */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <Basics/Common.h>
#include <Basics/conversions.h>
#include <Basics/tri-strings.h>

#include "Aql/AstNode.h"
#include "Aql/Function.h"
#include "Aql/Parser.h"

#line 86 "arangod/Aql/grammar.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "grammar.hpp".  */
#ifndef YY_AQL_ARANGOD_AQL_GRAMMAR_HPP_INCLUDED
# define YY_AQL_ARANGOD_AQL_GRAMMAR_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int Aqldebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_END = 0,
    T_FOR = 258,
    T_LET = 259,
    T_FILTER = 260,
    T_RETURN = 261,
    T_COLLECT = 262,
    T_SORT = 263,
    T_LIMIT = 264,
    T_ASC = 265,
    T_DESC = 266,
    T_IN = 267,
    T_WITH = 268,
    T_INTO = 269,
    T_FROM = 270,
    T_GRAPH = 271,
    T_TRAVERSE = 272,
    T_STEPS = 273,
    T_REMOVE = 274,
    T_INSERT = 275,
    T_UPDATE = 276,
    T_REPLACE = 277,
    T_UPSERT = 278,
    T_NULL = 279,
    T_TRUE = 280,
    T_FALSE = 281,
    T_STRING = 282,
    T_QUOTED_STRING = 283,
    T_INTEGER = 284,
    T_DOUBLE = 285,
    T_PARAMETER = 286,
    T_ASSIGN = 287,
    T_NOT = 288,
    T_AND = 289,
    T_OR = 290,
    T_EQ = 291,
    T_NE = 292,
    T_LT = 293,
    T_GT = 294,
    T_LE = 295,
    T_GE = 296,
    T_PLUS = 297,
    T_MINUS = 298,
    T_TIMES = 299,
    T_DIV = 300,
    T_MOD = 301,
    T_QUESTION = 302,
    T_COLON = 303,
    T_SCOPE = 304,
    T_RANGE = 305,
    T_COMMA = 306,
    T_OPEN = 307,
    T_CLOSE = 308,
    T_OBJECT_OPEN = 309,
    T_OBJECT_CLOSE = 310,
    T_ARRAY_OPEN = 311,
    T_ARRAY_CLOSE = 312,
    T_OUTBOUND = 313,
    T_INBOUND = 314,
    T_ANY = 315,
    T_NIN = 316,
    UMINUS = 317,
    UPLUS = 318,
    FUNCCALL = 319,
    REFERENCE = 320,
    INDEXED = 321,
    EXPANSION = 322
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 23 "arangod/Aql/grammar.y" /* yacc.c:355  */

  triagens::aql::AstNode*  node;
  char*                    strval;
  bool                     boolval;
  int64_t                  intval;

#line 202 "arangod/Aql/grammar.cpp" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int Aqlparse (triagens::aql::Parser* parser);

#endif /* !YY_AQL_ARANGOD_AQL_GRAMMAR_HPP_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 30 "arangod/Aql/grammar.y" /* yacc.c:358  */


using namespace triagens::aql;

////////////////////////////////////////////////////////////////////////////////
/// @brief forward for lexer function defined in Aql/tokens.ll
////////////////////////////////////////////////////////////////////////////////

int Aqllex (YYSTYPE*, 
            YYLTYPE*, 
            void*);
 
////////////////////////////////////////////////////////////////////////////////
/// @brief register parse error
////////////////////////////////////////////////////////////////////////////////

void Aqlerror (YYLTYPE* locp, 
               triagens::aql::Parser* parser,
               char const* message) {
  parser->registerParseError(TRI_ERROR_QUERY_PARSE, message, locp->first_line, locp->first_column);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief shortcut macro for signalling out of memory
////////////////////////////////////////////////////////////////////////////////

#define ABORT_OOM                                   \
  parser->registerError(TRI_ERROR_OUT_OF_MEMORY);   \
  YYABORT;

#define scanner parser->scanner()


#line 265 "arangod/Aql/grammar.cpp" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   889

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  80
/* YYNRULES -- Number of rules.  */
#define YYNRULES  177
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  307

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    68,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   217,   217,   219,   221,   223,   225,   227,   232,   234,
     239,   243,   249,   251,   256,   258,   260,   262,   264,   266,
     271,   277,   282,   287,   295,   303,   308,   310,   315,   322,
     332,   332,   346,   362,   389,   416,   448,   478,   480,   485,
     492,   495,   501,   515,   532,   532,   546,   546,   557,   560,
     566,   572,   575,   578,   581,   587,   592,   599,   607,   610,
     616,   627,   638,   647,   659,   664,   673,   685,   690,   693,
     699,   699,   751,   754,   757,   760,   763,   766,   772,   779,
     796,   796,   808,   811,   814,   820,   823,   826,   829,   832,
     835,   838,   841,   844,   847,   850,   853,   856,   859,   862,
     868,   874,   876,   881,   884,   884,   903,   906,   912,   915,
     921,   921,   930,   932,   937,   940,   946,   949,   963,   963,
     972,   974,   979,   981,   986,   989,   992,  1007,  1010,  1016,
    1019,  1025,  1028,  1031,  1037,  1040,  1046,  1049,  1056,  1060,
    1067,  1073,  1072,  1081,  1085,  1094,  1097,  1100,  1106,  1109,
    1115,  1157,  1160,  1163,  1170,  1180,  1180,  1196,  1211,  1225,
    1239,  1239,  1290,  1293,  1299,  1306,  1316,  1319,  1322,  1325,
    1328,  1334,  1341,  1348,  1362,  1368,  1375,  1384
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of query string\"", "error", "$undefined", "\"FOR declaration\"",
  "\"LET declaration\"", "\"FILTER declaration\"",
  "\"RETURN declaration\"", "\"COLLECT declaration\"",
  "\"SORT declaration\"", "\"LIMIT declaration\"", "\"ASC keyword\"",
  "\"DESC keyword\"", "\"IN keyword\"", "\"WITH keyword\"",
  "\"INTO keyword\"", "\"FROM keyword\"", "\"GRAPH keyword\"",
  "\"TRAVERSE keyword\"", "\"STEPS keyword\"", "\"REMOVE command\"",
  "\"INSERT command\"", "\"UPDATE command\"", "\"REPLACE command\"",
  "\"UPSERT command\"", "\"null\"", "\"true\"", "\"false\"",
  "\"identifier\"", "\"quoted string\"", "\"integer number\"",
  "\"number\"", "\"bind parameter\"", "\"assignment\"", "\"not operator\"",
  "\"and operator\"", "\"or operator\"", "\"== operator\"",
  "\"!= operator\"", "\"< operator\"", "\"> operator\"", "\"<= operator\"",
  "\">= operator\"", "\"+ operator\"", "\"- operator\"", "\"* operator\"",
  "\"/ operator\"", "\"% operator\"", "\"?\"", "\":\"", "\"::\"", "\"..\"",
  "\",\"", "\"(\"", "\")\"", "\"{\"", "\"}\"", "\"[\"", "\"]\"",
  "\"outbound direction\"", "\"inbound direction\"", "\"any direction\"",
  "T_NIN", "UMINUS", "UPLUS", "FUNCCALL", "REFERENCE", "INDEXED",
  "EXPANSION", "'.'", "$accept", "query",
  "optional_post_modification_lets", "optional_post_modification_block",
  "optional_statement_block_statements", "statement_block_statement",
  "for_statement", "filter_statement", "let_statement", "let_list",
  "let_element", "count_into", "collect_variable_list", "$@1",
  "collect_statement", "collect_list", "collect_element", "optional_into",
  "variable_list", "keep", "$@2", "sort_statement", "$@3", "sort_list",
  "sort_element", "sort_direction", "limit_statement", "return_statement",
  "in_or_into_collection", "remove_statement", "insert_statement",
  "update_parameters", "update_statement", "replace_parameters",
  "replace_statement", "update_or_replace", "upsert_statement", "$@4",
  "expression", "function_name", "function_call", "$@5", "operator_unary",
  "operator_binary", "operator_ternary",
  "optional_function_call_arguments", "expression_or_query", "$@6",
  "function_arguments_list", "compound_value", "array", "$@7",
  "optional_array_elements", "array_elements_list", "options", "object",
  "$@8", "optional_object_elements", "object_elements_list",
  "object_element", "array_filter_operator", "optional_array_filter",
  "optional_array_limit", "optional_array_return", "graph_collection",
  "graph_collection_list", "graph_subject", "$@9", "graph_direction",
  "graph_direction_steps", "reference", "$@10", "$@11", "simple_value",
  "numeric_value", "value_literal", "collection_name", "bind_parameter",
  "object_element_name", "variable_name", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,    46
};
# endif

#define YYPACT_NINF -270

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-270)))

#define YYTABLE_NINF -174

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -270,    30,   866,  -270,     8,     8,   805,   805,    35,  -270,
     151,   805,   805,   805,   805,  -270,  -270,  -270,  -270,  -270,
     103,  -270,  -270,  -270,  -270,    63,    63,    63,    63,    63,
    -270,     2,    31,  -270,    56,  -270,  -270,  -270,    12,  -270,
    -270,  -270,  -270,   805,   805,   805,   805,  -270,  -270,   656,
      76,  -270,  -270,  -270,  -270,  -270,  -270,  -270,   -36,  -270,
    -270,  -270,   656,    65,    85,     8,   805,    82,  -270,  -270,
     547,   547,  -270,   475,  -270,   493,   805,     8,    85,   100,
     117,  -270,  -270,  -270,  -270,  -270,   719,     8,     8,   805,
     111,   111,   111,   336,  -270,    23,   805,   805,   135,   805,
     805,   805,   805,   805,   805,   805,   805,   805,   805,   805,
     805,   805,   805,   805,   122,   116,   739,    32,   149,   119,
    -270,   123,  -270,   139,   132,  -270,   380,   151,   825,    -4,
      85,    85,   805,    85,   805,    85,   583,   152,  -270,   119,
      85,  -270,  -270,  -270,  -270,  -270,  -270,   264,  -270,   805,
       6,  -270,   656,  -270,   133,  -270,  -270,   137,   805,   148,
     153,  -270,   158,   656,   150,   159,    96,   805,   692,   674,
     416,   416,   112,   112,   112,   112,   120,   120,   111,   111,
     111,   601,     0,  -270,   772,  -270,   155,   169,  -270,  -270,
       8,  -270,     8,   805,   805,  -270,  -270,  -270,  -270,  -270,
      22,    25,    62,  -270,  -270,  -270,  -270,  -270,  -270,  -270,
     547,  -270,   547,  -270,   805,   805,     8,  -270,  -270,   436,
     719,     8,  -270,   805,   300,  -270,    23,   805,  -270,   805,
      96,   805,   656,   162,  -270,  -270,   165,  -270,  -270,   213,
    -270,  -270,   656,  -270,    85,    85,   529,   638,   168,  -270,
      83,  -270,   171,  -270,  -270,   264,   805,   214,   656,   179,
    -270,   656,   656,   656,  -270,  -270,   805,   805,   219,  -270,
    -270,  -270,  -270,   805,  -270,     8,  -270,  -270,  -270,   436,
     719,   805,  -270,   656,   805,   223,   547,  -270,    50,  -270,
     805,   656,   400,   805,   173,    85,  -270,   180,   436,   805,
     656,  -270,  -270,    50,  -270,   656,  -270
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      12,     0,     0,     1,     0,     0,     0,     0,    30,    46,
       0,     0,     0,     0,     0,    70,    13,    14,    16,    15,
      40,    17,    18,    19,     2,    10,    10,    10,    10,    10,
     177,     0,    25,    26,     0,   168,   169,   170,   150,   166,
     164,   165,   174,     0,     0,     0,   155,   118,   110,    24,
      80,   153,    72,    73,    74,   151,   108,   109,    76,   167,
      75,   152,    57,     0,   116,     0,     0,    55,   162,   163,
       0,     0,    64,     0,    67,     0,     0,     0,   116,   116,
       0,     3,     4,     5,     6,     7,     0,     0,     0,     0,
      84,    82,    83,     0,    12,   120,   112,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      32,    31,    37,     0,    47,    48,    51,     0,     0,     0,
     116,   116,     0,   116,     0,   116,     0,    41,    33,    44,
     116,    34,     9,    11,   145,   146,   147,    20,   148,     0,
       0,    27,    28,   154,     0,   175,   176,     0,     0,     0,
     121,   122,     0,   114,     0,   113,    98,     0,    86,    85,
      92,    93,    94,    95,    96,    97,    87,    88,    89,    90,
      91,     0,    77,    79,   104,   127,     0,   160,   157,   158,
       0,   117,     0,     0,     0,    52,    53,    50,    54,    56,
     150,   166,   174,    58,   171,   172,   173,    59,    60,    61,
       0,    62,     0,    65,     0,     0,     0,    35,   149,     0,
       0,     0,   156,     0,     0,   119,     0,     0,   111,     0,
      99,     0,   103,     0,   106,    12,   102,   159,   128,   129,
      29,    38,    39,    49,   116,   116,     0,   116,    45,    42,
       0,   136,   140,    21,   137,     0,     0,     0,   126,     0,
     123,   124,   115,   100,    81,   105,   104,     0,   131,    63,
      66,    68,    69,     0,    36,     0,   144,   143,   141,     0,
       0,     0,   107,   130,     0,   134,     0,    43,     0,    22,
       0,   125,   132,     0,     0,   116,   138,   142,     0,     0,
     135,   161,    71,     0,    23,   133,   139
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -270,   -84,  -270,    58,  -270,  -270,  -270,  -270,   154,  -270,
     144,   216,  -270,  -270,  -270,  -270,    45,  -270,  -270,  -270,
    -270,  -270,  -270,  -270,    44,  -270,  -270,   160,   -62,  -270,
    -270,  -270,  -270,  -270,  -270,  -270,  -270,  -270,    -6,  -270,
    -270,  -270,  -270,  -270,  -270,  -270,   -27,  -270,  -270,  -270,
    -270,  -270,  -270,  -270,   -75,   -98,  -270,  -270,  -270,    15,
    -270,  -270,  -270,  -270,  -269,  -270,  -262,  -270,  -131,  -208,
    -270,  -270,  -270,    33,  -270,     5,   114,    -8,  -270,    43
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    80,    81,     2,    16,    17,    18,    19,    32,
      33,    64,    20,    65,    21,   121,   122,    79,   248,   140,
     216,    22,    66,   124,   125,   197,    23,    24,   130,    25,
      26,    72,    27,    74,    28,   273,    29,    76,   126,    50,
      51,   115,    52,    53,    54,   233,   234,   235,   236,    55,
      56,    96,   164,   165,   120,    57,    95,   159,   160,   161,
     187,   268,   285,   294,   252,   297,   253,   288,   148,   149,
      58,    94,   239,    67,    59,    60,   203,    61,   162,    34
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      49,    62,    69,   138,   141,    70,    71,    73,    75,   131,
     154,   133,   256,   135,    86,    68,   218,   289,   220,   296,
     116,   191,  -171,   204,   205,  -172,  -171,   206,  -171,  -172,
       3,  -172,   117,    98,   306,    30,   304,    90,    91,    92,
      93,   191,   107,   108,   109,   110,   111,    31,    63,  -171,
     155,   156,  -172,    87,   157,   208,   209,   221,   211,   188,
     213,   -78,  -173,    42,   -78,   217,  -173,    -8,  -173,    -8,
     136,   -78,   290,  -171,   -78,  -171,  -172,   251,  -172,   158,
     147,    42,    88,   152,    82,    83,    84,    85,    89,  -173,
     163,   166,   118,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   123,   189,
     186,   276,   119,  -173,    42,  -173,    63,    77,    69,    69,
     137,     5,   166,     7,   218,   114,   210,   139,   212,    98,
     150,    68,    68,   127,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   219,    98,    98,   113,   167,   244,   183,
     245,   265,   224,    98,   107,   108,   109,   110,   111,   198,
     199,   230,   113,   190,   109,   110,   111,    97,   184,   269,
     270,   193,   274,    47,   192,    35,    36,    37,   232,    39,
      40,    41,    42,   194,   215,   223,   222,   242,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   225,   226,   113,   227,   228,   246,   247,
     229,   254,   237,   238,   255,   264,   266,   258,   267,   275,
     302,   261,   278,   262,   295,   263,   280,   281,   284,   293,
     301,   303,   151,   240,   142,   123,    78,   241,   243,   282,
     143,   260,   277,   207,     0,     0,     0,     0,     0,     0,
     279,     0,     0,     0,     0,     0,     0,     0,     0,   249,
     232,   283,     0,     0,   257,     0,     0,   286,     0,     0,
       0,   254,     0,     0,   255,   291,    97,     0,   292,     0,
     254,     0,     0,     0,   298,     0,     0,   300,     0,     0,
     254,     0,     0,   305,     0,   254,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    97,     0,   113,     0,     0,     0,   287,     0,
       0,     0,   144,   145,   146,     0,     0,     0,     0,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    97,     0,
     113,     0,     0,     0,     0,     0,     0,   259,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,   153,
     195,   196,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    35,    36,    37,     0,    39,    40,
      41,    42,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    97,     0,
     113,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    97,    98,
     113,   299,   250,     0,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   251,     0,     0,   113,    42,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,   128,   132,   129,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   128,   134,   129,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    97,     0,   113,     0,     0,     0,     0,     0,     0,
     271,   272,     0,     0,     0,     0,     0,     0,     0,   128,
       0,   129,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    97,     0,   113,     0,     0,
       0,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   231,
      97,   113,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   119,     0,     0,    97,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    97,     0,   113,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    97,     0,   113,    98,    99,     0,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   113,    98,     0,     0,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   113,    35,    36,    37,    38,    39,    40,    41,
      42,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,    44,    45,    35,    36,    37,    38,    39,    40,    41,
      42,    46,    43,    47,     0,    48,     0,   144,   145,   146,
       0,    44,    45,   185,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,    48,    35,    36,    37,    38,
      39,    40,    41,    42,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,    44,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,  -101,    47,     0,    48,    35,
      36,    37,    38,    39,    40,    41,    42,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,    44,    45,    35,
      36,    37,   200,   201,    40,    41,   202,    46,    43,    47,
       0,    48,     0,     0,     0,     0,     0,    44,    45,     4,
       5,     6,     7,     8,     9,    10,     0,    46,     0,    47,
       0,    48,     0,     0,     0,    11,    12,    13,    14,    15
};

static const yytype_int16 yycheck[] =
{
       6,     7,    10,    78,    79,    11,    12,    13,    14,    71,
      94,    73,   220,    75,    12,    10,   147,   279,    12,   288,
      56,   119,     0,    27,    28,     0,     4,    31,     6,     4,
       0,     6,    68,    33,   303,    27,   298,    43,    44,    45,
      46,   139,    42,    43,    44,    45,    46,     4,    13,    27,
      27,    28,    27,    51,    31,   130,   131,    51,   133,    27,
     135,    49,     0,    31,    52,   140,     4,     4,     6,     6,
      76,    49,   280,    51,    52,    53,    51,    27,    53,    56,
      86,    31,    51,    89,    26,    27,    28,    29,    32,    27,
      96,    97,    27,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,    65,   117,
     116,    28,    27,    51,    31,    53,    13,    14,   126,   127,
      77,     4,   128,     6,   255,    49,   132,    27,   134,    33,
      87,   126,   127,    51,    38,    39,    40,    41,    42,    43,
      44,    45,    46,   149,    33,    33,    50,    12,   210,    27,
     212,   235,   158,    33,    42,    43,    44,    45,    46,   126,
     127,   167,    50,    14,    44,    45,    46,    12,    52,   244,
     245,    32,   247,    54,    51,    24,    25,    26,   184,    28,
      29,    30,    31,    51,    32,    48,    53,   193,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    55,    51,    50,    48,    57,   214,   215,
      51,   219,    57,    44,   220,    53,    51,   223,     5,    51,
     295,   227,    51,   229,   286,   231,    12,    48,     9,     6,
      57,    51,    88,   190,    80,   192,    20,   192,   194,   266,
      80,   226,   250,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     256,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   216,
     266,   267,    -1,    -1,   221,    -1,    -1,   273,    -1,    -1,
      -1,   279,    -1,    -1,   280,   281,    12,    -1,   284,    -1,
     288,    -1,    -1,    -1,   290,    -1,    -1,   293,    -1,    -1,
     298,    -1,    -1,   299,    -1,   303,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    12,    -1,    50,    -1,    -1,    -1,   275,    -1,
      -1,    -1,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    12,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    -1,    50,    -1,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    -1,    28,    29,
      30,    31,    12,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    12,    -1,
      50,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    12,    33,
      50,    51,    16,    -1,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    27,    -1,    -1,    50,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    -1,    50,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    12,    13,    14,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    -1,    50,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    12,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      21,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,
      -1,    14,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    -1,    50,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    12,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    -1,    50,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      12,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    12,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    12,    -1,    50,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    12,    -1,    50,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    -1,    -1,    -1,    50,    33,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    -1,
      -1,    -1,    50,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    24,    25,    26,    27,    28,    29,    30,
      31,    52,    33,    54,    -1,    56,    -1,    58,    59,    60,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    54,    -1,    56,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    24,
      25,    26,    27,    28,    29,    30,    31,    52,    33,    54,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    42,    43,     3,
       4,     5,     6,     7,     8,     9,    -1,    52,    -1,    54,
      -1,    56,    -1,    -1,    -1,    19,    20,    21,    22,    23
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    70,    73,     0,     3,     4,     5,     6,     7,     8,
       9,    19,    20,    21,    22,    23,    74,    75,    76,    77,
      81,    83,    90,    95,    96,    98,    99,   101,   103,   105,
      27,   148,    78,    79,   148,    24,    25,    26,    27,    28,
      29,    30,    31,    33,    42,    43,    52,    54,    56,   107,
     108,   109,   111,   112,   113,   118,   119,   124,   139,   143,
     144,   146,   107,    13,    80,    82,    91,   142,   144,   146,
     107,   107,   100,   107,   102,   107,   106,    14,    80,    86,
      71,    72,    72,    72,    72,    72,    12,    51,    51,    32,
     107,   107,   107,   107,   140,   125,   120,    12,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    50,    49,   110,    56,    68,    27,    27,
     123,    84,    85,   148,    92,    93,   107,    51,    12,    14,
      97,    97,    13,    97,    13,    97,   107,   148,   123,    27,
      88,   123,    77,    96,    58,    59,    60,   107,   137,   138,
     148,    79,   107,    53,    70,    27,    28,    31,    56,   126,
     127,   128,   147,   107,   121,   122,   107,    12,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,    27,    52,    44,   107,   129,    27,   146,
      14,   124,    51,    32,    51,    10,    11,    94,   142,   142,
      27,    28,    31,   145,    27,    28,    31,   145,   123,   123,
     107,   123,   107,   123,    20,    32,    89,   123,   137,   107,
      12,    51,    53,    48,   107,    55,    51,    48,    57,    51,
     107,    48,   107,   114,   115,   116,   117,    57,    44,   141,
     148,    85,   107,    93,    97,    97,   107,   107,    87,   148,
      16,    27,   133,   135,   146,   107,   138,   148,   107,    57,
     128,   107,   107,   107,    53,    70,    51,     5,   130,   123,
     123,    21,    22,   104,   123,    51,    28,   146,    51,   107,
      12,    48,   115,   107,     9,   131,   107,   148,   136,   135,
     138,   107,   107,     6,   132,    97,   133,   134,   107,    51,
     107,    57,   123,    51,   135,   107,   133
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    69,    70,    70,    70,    70,    70,    70,    71,    71,
      72,    72,    73,    73,    74,    74,    74,    74,    74,    74,
      75,    75,    75,    75,    76,    77,    78,    78,    79,    80,
      82,    81,    83,    83,    83,    83,    83,    84,    84,    85,
      86,    86,    87,    87,    89,    88,    91,    90,    92,    92,
      93,    94,    94,    94,    94,    95,    95,    96,    97,    97,
      98,    99,   100,   100,   101,   102,   102,   103,   104,   104,
     106,   105,   107,   107,   107,   107,   107,   107,   108,   108,
     110,   109,   111,   111,   111,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     113,   114,   114,   115,   116,   115,   117,   117,   118,   118,
     120,   119,   121,   121,   122,   122,   123,   123,   125,   124,
     126,   126,   127,   127,   128,   128,   128,   129,   129,   130,
     130,   131,   131,   131,   132,   132,   133,   133,   134,   134,
     135,   136,   135,   135,   135,   137,   137,   137,   138,   138,
     139,   139,   139,   139,   139,   140,   139,   139,   139,   139,
     141,   139,   142,   142,   143,   143,   144,   144,   144,   144,
     144,   145,   145,   145,   146,   147,   147,   148
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     3,     3,     3,     3,     0,     2,
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       4,     6,     8,    10,     2,     2,     1,     3,     3,     4,
       0,     3,     3,     3,     3,     4,     6,     1,     3,     3,
       0,     2,     1,     3,     0,     3,     0,     3,     1,     3,
       2,     0,     1,     1,     1,     2,     4,     2,     2,     2,
       4,     4,     3,     5,     2,     3,     5,     2,     1,     1,
       0,     9,     1,     1,     1,     1,     1,     3,     1,     3,
       0,     5,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       5,     0,     1,     1,     0,     2,     1,     3,     1,     1,
       0,     4,     0,     1,     1,     3,     0,     2,     0,     4,
       0,     1,     1,     3,     3,     5,     3,     1,     2,     0,
       2,     0,     2,     4,     0,     2,     1,     1,     1,     3,
       1,     0,     4,     2,     2,     1,     1,     1,     1,     2,
       1,     1,     1,     1,     3,     0,     4,     3,     3,     4,
       0,     8,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, parser, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, parser); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, triagens::aql::Parser* parser)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (parser);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, triagens::aql::Parser* parser)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, parser);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, triagens::aql::Parser* parser)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , parser);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, parser); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, triagens::aql::Parser* parser)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (parser);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (triagens::aql::Parser* parser)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 217 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1817 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 3:
#line 219 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1824 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 4:
#line 221 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1831 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 5:
#line 223 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1838 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 6:
#line 225 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1845 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 7:
#line 227 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1852 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 8:
#line 232 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1859 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 9:
#line 234 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1866 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 10:
#line 239 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // still need to close the scope opened by the data-modification statement
      parser->ast()->scopes()->endNested();
    }
#line 1875 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 11:
#line 243 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // the RETURN statement will close the scope opened by the data-modification statement
    }
#line 1883 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 12:
#line 249 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1890 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 13:
#line 251 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1897 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 14:
#line 256 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1904 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 15:
#line 258 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1911 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 16:
#line 260 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1918 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 17:
#line 262 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1925 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 18:
#line 264 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1932 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 19:
#line 266 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1939 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 20:
#line 271 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
     
      auto node = parser->ast()->createNodeFor((yyvsp[-2].strval), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1950 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 21:
#line 277 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
      auto node = parser->ast()->createNodeTraversal((yyvsp[-4].strval), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1960 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 22:
#line 282 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
      auto node = parser->ast()->createNodeTraversal((yyvsp[-6].strval), (yyvsp[-4].strval), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1970 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 23:
#line 287 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
      auto node = parser->ast()->createNodeTraversal((yyvsp[-8].strval), (yyvsp[-6].strval), (yyvsp[-4].strval), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1980 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 24:
#line 295 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // operand is a reference. can use it directly
      auto node = parser->ast()->createNodeFilter((yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1990 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 25:
#line 303 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1997 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 26:
#line 308 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2004 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 27:
#line 310 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2011 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 28:
#line 315 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeLet((yyvsp[-2].strval), (yyvsp[0].node), true);
      parser->ast()->addOperation(node);
    }
#line 2020 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 29:
#line 322 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! TRI_CaseEqualString((yyvsp[-2].strval), "COUNT")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'COUNT'", (yyvsp[-2].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 2032 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 30:
#line 332 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2041 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 31:
#line 335 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      auto list = static_cast<AstNode*>(parser->popStack());

      if (list == nullptr) {
        ABORT_OOM
      }
      (yyval.node) = list;
    }
#line 2054 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 32:
#line 346 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto scopes = parser->ast()->scopes();

      // check if we are in the main scope
      bool reRegisterVariables = (scopes->type() != triagens::aql::AQL_SCOPE_MAIN); 

      if (reRegisterVariables) {
        // end the active scopes
        scopes->endNested();
        // start a new scope
        scopes->start(triagens::aql::AQL_SCOPE_COLLECT);
      }

      auto node = parser->ast()->createNodeCollectCount(parser->ast()->createNodeArray(), (yyvsp[-1].strval), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2075 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 33:
#line 362 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto scopes = parser->ast()->scopes();

      // check if we are in the main scope
      bool reRegisterVariables = (scopes->type() != triagens::aql::AQL_SCOPE_MAIN); 

      if (reRegisterVariables) {
        // end the active scopes
        scopes->endNested();
        // start a new scope
        scopes->start(triagens::aql::AQL_SCOPE_COLLECT);

        size_t const n = (yyvsp[-2].node)->numMembers();
        for (size_t i = 0; i < n; ++i) {
          auto member = (yyvsp[-2].node)->getMember(i);

          if (member != nullptr) {
            TRI_ASSERT(member->type == NODE_TYPE_ASSIGN);
            auto v = static_cast<Variable*>(member->getMember(0)->getData());
            scopes->addVariable(v);
          }
        }
      }

      auto node = parser->ast()->createNodeCollectCount((yyvsp[-2].node), (yyvsp[-1].strval), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2107 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 34:
#line 389 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto scopes = parser->ast()->scopes();

      // check if we are in the main scope
      bool reRegisterVariables = (scopes->type() != triagens::aql::AQL_SCOPE_MAIN); 

      if (reRegisterVariables) {
        // end the active scopes
        scopes->endNested();
        // start a new scope
        scopes->start(triagens::aql::AQL_SCOPE_COLLECT);

        size_t const n = (yyvsp[-2].node)->numMembers();
        for (size_t i = 0; i < n; ++i) {
          auto member = (yyvsp[-2].node)->getMember(i);

          if (member != nullptr) {
            TRI_ASSERT(member->type == NODE_TYPE_ASSIGN);
            auto v = static_cast<Variable*>(member->getMember(0)->getData());
            scopes->addVariable(v);
          }
        }
      }

      auto node = parser->ast()->createNodeCollect((yyvsp[-2].node), (yyvsp[-1].strval), nullptr, (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2139 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 35:
#line 416 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto scopes = parser->ast()->scopes();

      // check if we are in the main scope
      bool reRegisterVariables = (scopes->type() != triagens::aql::AQL_SCOPE_MAIN); 

      if (reRegisterVariables) {
        // end the active scopes
        scopes->endNested();
        // start a new scope
        scopes->start(triagens::aql::AQL_SCOPE_COLLECT);

        size_t const n = (yyvsp[-3].node)->numMembers();
        for (size_t i = 0; i < n; ++i) {
          auto member = (yyvsp[-3].node)->getMember(i);

          if (member != nullptr) {
            TRI_ASSERT(member->type == NODE_TYPE_ASSIGN);
            auto v = static_cast<Variable*>(member->getMember(0)->getData());
            scopes->addVariable(v);
          }
        }
      }

      if ((yyvsp[-2].strval) == nullptr && 
          (yyvsp[-1].node) != nullptr) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "use of 'KEEP' without 'INTO'", yylloc.first_line, yylloc.first_column);
      } 

      auto node = parser->ast()->createNodeCollect((yyvsp[-3].node), (yyvsp[-2].strval), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2176 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 36:
#line 448 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto scopes = parser->ast()->scopes();

      // check if we are in the main scope
      bool reRegisterVariables = (scopes->type() != triagens::aql::AQL_SCOPE_MAIN); 

      if (reRegisterVariables) {
        // end the active scopes
        scopes->endNested();
        // start a new scope
        scopes->start(triagens::aql::AQL_SCOPE_COLLECT);

        size_t const n = (yyvsp[-5].node)->numMembers();
        for (size_t i = 0; i < n; ++i) {
          auto member = (yyvsp[-5].node)->getMember(i);

          if (member != nullptr) {
            TRI_ASSERT(member->type == NODE_TYPE_ASSIGN);
            auto v = static_cast<Variable*>(member->getMember(0)->getData());
            scopes->addVariable(v);
          }
        }
      }

      auto node = parser->ast()->createNodeCollectExpression((yyvsp[-5].node), (yyvsp[-3].strval), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2208 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 37:
#line 478 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2215 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 38:
#line 480 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2222 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 39:
#line 485 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeAssign((yyvsp[-2].strval), (yyvsp[0].node));
      parser->pushArrayElement(node);
    }
#line 2231 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 40:
#line 492 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = nullptr;
    }
#line 2239 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 41:
#line 495 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);
    }
#line 2247 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 42:
#line 501 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->ast()->scopes()->existsVariable((yyvsp[0].strval))) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "use of unknown variable '%s' for KEEP", (yyvsp[0].strval), yylloc.first_line, yylloc.first_column);
      }
        
      auto node = parser->ast()->createNodeReference((yyvsp[0].strval));
      if (node == nullptr) {
        ABORT_OOM
      }

      // indicate the this node is a reference to the variable name, not the variable value
      node->setFlag(FLAG_KEEP_VARIABLENAME);
      parser->pushArrayElement(node);
    }
#line 2266 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 43:
#line 515 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->ast()->scopes()->existsVariable((yyvsp[0].strval))) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "use of unknown variable '%s' for KEEP", (yyvsp[0].strval), yylloc.first_line, yylloc.first_column);
      }
        
      auto node = parser->ast()->createNodeReference((yyvsp[0].strval));
      if (node == nullptr) {
        ABORT_OOM
      }

      // indicate the this node is a reference to the variable name, not the variable value
      node->setFlag(FLAG_KEEP_VARIABLENAME);
      parser->pushArrayElement(node);
    }
#line 2285 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 44:
#line 532 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! TRI_CaseEqualString((yyvsp[0].strval), "KEEP")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'KEEP'", (yyvsp[0].strval), yylloc.first_line, yylloc.first_column);
      }

      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2298 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 45:
#line 539 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode*>(parser->popStack());
      (yyval.node) = list;
    }
#line 2307 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 46:
#line 546 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2316 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 47:
#line 549 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode const*>(parser->popStack());
      auto node = parser->ast()->createNodeSort(list);
      parser->ast()->addOperation(node);
    }
#line 2326 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 48:
#line 557 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2334 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 49:
#line 560 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2342 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 50:
#line 566 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeSortElement((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 2350 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 51:
#line 572 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 2358 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 52:
#line 575 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 2366 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 53:
#line 578 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(false);
    }
#line 2374 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 54:
#line 581 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2382 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 55:
#line 587 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto offset = parser->ast()->createNodeValueInt(0);
      auto node = parser->ast()->createNodeLimit(offset, (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2392 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 56:
#line 592 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeLimit((yyvsp[-2].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2401 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 57:
#line 599 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeReturn((yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->ast()->scopes()->endNested();
    }
#line 2411 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 58:
#line 607 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2419 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 59:
#line 610 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       (yyval.node) = (yyvsp[0].node);
     }
#line 2427 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 60:
#line 616 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REMOVE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }
      auto node = parser->ast()->createNodeRemove((yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2440 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 61:
#line 627 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_INSERT, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }
      auto node = parser->ast()->createNodeInsert((yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2453 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 62:
#line 638 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_UPDATE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeUpdate(nullptr, (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2467 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 63:
#line 647 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_UPDATE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeUpdate((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2481 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 64:
#line 659 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2488 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 65:
#line 664 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REPLACE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeReplace(nullptr, (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2502 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 66:
#line 673 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REPLACE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeReplace((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2516 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 67:
#line 685 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2523 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 68:
#line 690 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = static_cast<int64_t>(NODE_TYPE_UPDATE);
    }
#line 2531 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 69:
#line 693 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = static_cast<int64_t>(NODE_TYPE_REPLACE);
    }
#line 2539 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 70:
#line 699 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      // reserve a variable named "$OLD", we might need it in the update expression
      // and in a later return thing
      parser->pushStack(parser->ast()->createNodeVariable(Variable::NAME_OLD, true));
    }
#line 2549 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 71:
#line 703 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_UPSERT, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* variableNode = static_cast<AstNode*>(parser->popStack());
      
      auto scopes = parser->ast()->scopes();
      
      scopes->start(triagens::aql::AQL_SCOPE_SUBQUERY);
      parser->ast()->startSubQuery();
      
      scopes->start(triagens::aql::AQL_SCOPE_FOR);
      std::string const variableName = parser->ast()->variables()->nextName();
      auto forNode = parser->ast()->createNodeFor(variableName.c_str(), (yyvsp[-1].node), false);
      parser->ast()->addOperation(forNode);

      auto filterNode = parser->ast()->createNodeUpsertFilter(parser->ast()->createNodeReference(variableName.c_str()), (yyvsp[-6].node));
      parser->ast()->addOperation(filterNode);
      
      auto offsetValue = parser->ast()->createNodeValueInt(0);
      auto limitValue = parser->ast()->createNodeValueInt(1);
      auto limitNode = parser->ast()->createNodeLimit(offsetValue, limitValue);
      parser->ast()->addOperation(limitNode);
      
      auto refNode = parser->ast()->createNodeReference(variableName.c_str());
      auto returnNode = parser->ast()->createNodeReturn(refNode);
      parser->ast()->addOperation(returnNode);
      scopes->endNested();

      AstNode* subqueryNode = parser->ast()->endSubQuery();
      scopes->endCurrent();
      
      std::string const subqueryName = parser->ast()->variables()->nextName();
      auto subQuery = parser->ast()->createNodeLet(subqueryName.c_str(), subqueryNode, false);
      parser->ast()->addOperation(subQuery);
      
      auto index = parser->ast()->createNodeValueInt(0);
      auto firstDoc = parser->ast()->createNodeLet(variableNode, parser->ast()->createNodeIndexedAccess(parser->ast()->createNodeReference(subqueryName.c_str()), index));
      parser->ast()->addOperation(firstDoc);

      auto node = parser->ast()->createNodeUpsert(static_cast<AstNodeType>((yyvsp[-3].intval)), parser->ast()->createNodeReference(Variable::NAME_OLD), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2599 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 72:
#line 751 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2607 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 73:
#line 754 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2615 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 74:
#line 757 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2623 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 75:
#line 760 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2631 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 76:
#line 763 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2639 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 77:
#line 766 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeRange((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2647 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 78:
#line 772 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);

      if ((yyval.strval) == nullptr) {
        ABORT_OOM
      }
    }
#line 2659 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 79:
#line 779 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[-2].strval) == nullptr || (yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      std::string temp((yyvsp[-2].strval));
      temp.append("::");
      temp.append((yyvsp[0].strval));
      (yyval.strval) = parser->query()->registerString(temp.c_str(), temp.size(), false);

      if ((yyval.strval) == nullptr) {
        ABORT_OOM
      }
    }
#line 2678 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 80:
#line 796 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushStack((yyvsp[0].strval));

      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2689 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 81:
#line 801 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode const*>(parser->popStack());
      (yyval.node) = parser->ast()->createNodeFunctionCall(static_cast<char const*>(parser->popStack()), list);
    }
#line 2698 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 82:
#line 808 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_PLUS, (yyvsp[0].node));
    }
#line 2706 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 83:
#line 811 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_MINUS, (yyvsp[0].node));
    }
#line 2714 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 84:
#line 814 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_NOT, (yyvsp[0].node));
    }
#line 2722 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 85:
#line 820 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_OR, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2730 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 86:
#line 823 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_AND, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2738 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 87:
#line 826 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_PLUS, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2746 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 88:
#line 829 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_MINUS, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2754 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 89:
#line 832 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_TIMES, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2762 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 90:
#line 835 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_DIV, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2770 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 91:
#line 838 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_MOD, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2778 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 92:
#line 841 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_EQ, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2786 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 93:
#line 844 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_NE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2794 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 94:
#line 847 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_LT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2802 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 95:
#line 850 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_GT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2810 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 96:
#line 853 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_LE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2818 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 97:
#line 856 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_GE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2826 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 98:
#line 859 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_IN, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2834 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 99:
#line 862 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_NIN, (yyvsp[-3].node), (yyvsp[0].node));
    }
#line 2842 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 100:
#line 868 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeTernaryOperator((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2850 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 101:
#line 874 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2857 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 102:
#line 876 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2864 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 103:
#line 881 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2872 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 104:
#line 884 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (parser->isModificationQuery()) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected subquery after data-modification operation", yylloc.first_line, yylloc.first_column);
      }
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_SUBQUERY);
      parser->ast()->startSubQuery();
    }
#line 2884 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 105:
#line 890 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      AstNode* node = parser->ast()->endSubQuery();
      parser->ast()->scopes()->endCurrent();

      std::string const variableName = parser->ast()->variables()->nextName();
      auto subQuery = parser->ast()->createNodeLet(variableName.c_str(), node, false);
      parser->ast()->addOperation(subQuery);

      (yyval.node) = parser->ast()->createNodeReference(variableName.c_str());
    }
#line 2899 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 106:
#line 903 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2907 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 107:
#line 906 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2915 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 108:
#line 912 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2923 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 109:
#line 915 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2931 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 110:
#line 921 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2940 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 111:
#line 924 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = static_cast<AstNode*>(parser->popStack());
    }
#line 2948 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 112:
#line 930 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2955 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 113:
#line 932 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2962 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 114:
#line 937 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2970 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 115:
#line 940 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2978 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 116:
#line 946 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 2986 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 117:
#line 949 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[-1].strval) == nullptr || (yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }

      if (! TRI_CaseEqualString((yyvsp[-1].strval), "OPTIONS")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'OPTIONS'", (yyvsp[-1].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.node) = (yyvsp[0].node);
    }
#line 3002 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 118:
#line 963 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeObject();
      parser->pushStack(node);
    }
#line 3011 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 119:
#line 966 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = static_cast<AstNode*>(parser->popStack());
    }
#line 3019 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 120:
#line 972 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 3026 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 121:
#line 974 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 3033 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 122:
#line 979 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 3040 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 123:
#line 981 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 3047 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 124:
#line 986 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushObjectElement((yyvsp[-2].strval), (yyvsp[0].node));
    }
#line 3055 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 125:
#line 989 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushObjectElement((yyvsp[-3].node), (yyvsp[0].node));
    }
#line 3063 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 126:
#line 992 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[-2].strval) == nullptr) {
        ABORT_OOM
      }
      
      if (strlen((yyvsp[-2].strval)) < 1 || (yyvsp[-2].strval)[0] == '@') {
        parser->registerParseError(TRI_ERROR_QUERY_BIND_PARAMETER_TYPE, TRI_errno_string(TRI_ERROR_QUERY_BIND_PARAMETER_TYPE), (yyvsp[-2].strval), yylloc.first_line, yylloc.first_column);
      }

      auto param = parser->ast()->createNodeParameter((yyvsp[-2].strval));
      parser->pushObjectElement(param, (yyvsp[0].node));
    }
#line 3080 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 127:
#line 1007 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = 1;
    }
#line 3088 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 128:
#line 1010 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = (yyvsp[-1].intval) + 1;
    }
#line 3096 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 129:
#line 1016 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3104 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 130:
#line 1019 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3112 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 131:
#line 1025 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3120 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 132:
#line 1028 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeArrayLimit(nullptr, (yyvsp[0].node));
    }
#line 3128 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 133:
#line 1031 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeArrayLimit((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 3136 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 134:
#line 1037 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3144 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 135:
#line 1040 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3152 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 136:
#line 1046 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval));
    }
#line 3160 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 137:
#line 1049 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // TODO FIXME check @s
      (yyval.node) = (yyvsp[0].node);
    }
#line 3169 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 138:
#line 1056 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       auto node = static_cast<AstNode*>(parser->peekStack());
       node->addMember((yyvsp[0].node));
     }
#line 3178 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 139:
#line 1060 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       auto node = static_cast<AstNode*>(parser->peekStack());
       node->addMember((yyvsp[0].node));
     }
#line 3187 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 140:
#line 1067 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      node->addMember((yyvsp[0].node));
      (yyval.node) = parser->ast()->createNodeCollectionList(node);
    }
#line 3197 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 141:
#line 1073 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
      node->addMember((yyvsp[-1].node));
    }
#line 3207 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 142:
#line 1077 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = static_cast<AstNode*>(parser->popStack());
      (yyval.node) = parser->ast()->createNodeCollectionList(node);
    }
#line 3216 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 143:
#line 1081 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // graph name
      (yyval.node) = (yyvsp[0].node);
    }
#line 3225 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 144:
#line 1085 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // graph name
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval));
    }
#line 3234 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 145:
#line 1094 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = 2;
    }
#line 3242 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 146:
#line 1097 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = 1;
    }
#line 3250 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 147:
#line 1100 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = 0; 
    }
#line 3258 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 148:
#line 1106 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeDirection((yyvsp[0].intval), 1);
    }
#line 3266 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 149:
#line 1109 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeDirection((yyvsp[0].intval), (yyvsp[-1].node));
    }
#line 3274 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 150:
#line 1115 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // variable or collection
      auto ast = parser->ast();
      AstNode* node = nullptr;

      auto variable = ast->scopes()->getVariable((yyvsp[0].strval));
      
      if (variable != nullptr) {
        // variable exists, now use it
        node = ast->createNodeReference(variable);
      }
      else {
        // variable does not exist
        // now try variable aliases OLD (= $OLD) and NEW (= $NEW)
        if (strcmp((yyvsp[0].strval), "OLD") == 0) {
          variable = ast->scopes()->getVariable(Variable::NAME_OLD);
        }
        else if (strcmp((yyvsp[0].strval), "NEW") == 0) {
          variable = ast->scopes()->getVariable(Variable::NAME_NEW);
        }
        else if (ast->scopes()->canUseCurrentVariable() && strcmp((yyvsp[0].strval), "CURRENT") == 0) {
          variable = ast->scopes()->getCurrentVariable();
        }
        else if (strcmp((yyvsp[0].strval), Variable::NAME_CURRENT) == 0) {
          variable = ast->scopes()->getCurrentVariable();
        }
        
        if (variable != nullptr) {
          // variable alias exists, now use it
          node = ast->createNodeReference(variable);
        }
      }

      if (node == nullptr) {
        // variable not found. so it must have been a collection
        node = ast->createNodeCollection((yyvsp[0].strval), TRI_TRANSACTION_READ);
      }

      TRI_ASSERT(node != nullptr);

      (yyval.node) = node;
    }
#line 3321 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 151:
#line 1157 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3329 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 152:
#line 1160 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3337 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 153:
#line 1163 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
      
      if ((yyval.node) == nullptr) {
        ABORT_OOM
      }
    }
#line 3349 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 154:
#line 1170 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[-1].node)->type == NODE_TYPE_EXPANSION) {
        // create a dummy passthru node that reduces and evaluates the expansion first
        // and the expansion on top of the stack won't be chained with any other expansions
        (yyval.node) = parser->ast()->createNodePassthru((yyvsp[-1].node));
      }
      else {
        (yyval.node) = (yyvsp[-1].node);
      }
    }
#line 3364 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 155:
#line 1180 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (parser->isModificationQuery()) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected subquery after data-modification operation", yylloc.first_line, yylloc.first_column);
      }
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_SUBQUERY);
      parser->ast()->startSubQuery();
    }
#line 3376 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 156:
#line 1186 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      AstNode* node = parser->ast()->endSubQuery();
      parser->ast()->scopes()->endCurrent();

      std::string const variableName = parser->ast()->variables()->nextName();
      auto subQuery = parser->ast()->createNodeLet(variableName.c_str(), node, false);
      parser->ast()->addOperation(subQuery);

      (yyval.node) = parser->ast()->createNodeReference(variableName.c_str());
    }
#line 3391 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 157:
#line 1196 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // named variable access, e.g. variable.reference
      if ((yyvsp[-2].node)->type == NODE_TYPE_EXPANSION) {
        // if left operand is an expansion already...
        // dive into the expansion's right-hand child nodes for further expansion and
        // patch the bottom-most one
        auto current = const_cast<AstNode*>(parser->ast()->findExpansionSubNode((yyvsp[-2].node)));
        TRI_ASSERT(current->type == NODE_TYPE_EXPANSION);
        current->changeMember(1, parser->ast()->createNodeAttributeAccess(current->getMember(1), (yyvsp[0].strval)));
        (yyval.node) = (yyvsp[-2].node);
      }
      else {
        (yyval.node) = parser->ast()->createNodeAttributeAccess((yyvsp[-2].node), (yyvsp[0].strval));
      }
    }
#line 3411 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 158:
#line 1211 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // named variable access, e.g. variable.@reference
      if ((yyvsp[-2].node)->type == NODE_TYPE_EXPANSION) {
        // if left operand is an expansion already...
        // patch the existing expansion
        auto current = const_cast<AstNode*>(parser->ast()->findExpansionSubNode((yyvsp[-2].node)));
        TRI_ASSERT(current->type == NODE_TYPE_EXPANSION);
        current->changeMember(1, parser->ast()->createNodeBoundAttributeAccess(current->getMember(1), (yyvsp[0].node)));
        (yyval.node) = (yyvsp[-2].node);
      }
      else {
        (yyval.node) = parser->ast()->createNodeBoundAttributeAccess((yyvsp[-2].node), (yyvsp[0].node));
      }
    }
#line 3430 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 159:
#line 1225 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // indexed variable access, e.g. variable[index]
      if ((yyvsp[-3].node)->type == NODE_TYPE_EXPANSION) {
        // if left operand is an expansion already...
        // patch the existing expansion
        auto current = const_cast<AstNode*>(parser->ast()->findExpansionSubNode((yyvsp[-3].node)));
        TRI_ASSERT(current->type == NODE_TYPE_EXPANSION);
        current->changeMember(1, parser->ast()->createNodeIndexedAccess(current->getMember(1), (yyvsp[-1].node)));
        (yyval.node) = (yyvsp[-3].node);
      }
      else {
        (yyval.node) = parser->ast()->createNodeIndexedAccess((yyvsp[-3].node), (yyvsp[-1].node));
      }
    }
#line 3449 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 160:
#line 1239 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // variable expansion, e.g. variable[*], with optional FILTER, LIMIT and RETURN clauses
      if ((yyvsp[0].intval) > 1 && (yyvsp[-2].node)->type == NODE_TYPE_EXPANSION) {
        // create a dummy passthru node that reduces and evaluates the expansion first
        // and the expansion on top of the stack won't be chained with any other expansions
        (yyvsp[-2].node) = parser->ast()->createNodePassthru((yyvsp[-2].node));
      }

      // create a temporary iterator variable
      std::string const nextName = parser->ast()->variables()->nextName() + "_";
      char const* iteratorName = nextName.c_str();

      if ((yyvsp[-2].node)->type == NODE_TYPE_EXPANSION) {
        auto iterator = parser->ast()->createNodeIterator(iteratorName, (yyvsp[-2].node)->getMember(1));
        parser->pushStack(iterator);
      }
      else {
        auto iterator = parser->ast()->createNodeIterator(iteratorName, (yyvsp[-2].node));
        parser->pushStack(iterator);
      }

      auto scopes = parser->ast()->scopes();
      scopes->stackCurrentVariable(scopes->getVariable(iteratorName));
    }
#line 3478 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 161:
#line 1262 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto scopes = parser->ast()->scopes();
      scopes->unstackCurrentVariable();

      auto iterator = static_cast<AstNode const*>(parser->popStack());
      auto variableNode = iterator->getMember(0);
      TRI_ASSERT(variableNode->type == NODE_TYPE_VARIABLE);
      auto variable = static_cast<Variable const*>(variableNode->getData());

      if ((yyvsp[-7].node)->type == NODE_TYPE_EXPANSION) {
        auto expand = parser->ast()->createNodeExpansion((yyvsp[-5].intval), iterator, parser->ast()->createNodeReference(variable->name.c_str()), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node));
        (yyvsp[-7].node)->changeMember(1, expand);
        (yyval.node) = (yyvsp[-7].node);
/*
        auto current = const_cast<AstNode*>(parser->ast()->findExpansionSubNode($1));
        TRI_ASSERT(current->type == NODE_TYPE_EXPANSION);
        auto expand = parser->ast()->createNodeExpansion($3, iterator, parser->ast()->createNodeReference(variable->name.c_str()), $5, $6, $7);
        current->changeMember(1, expand);
        $$ = $1;
*/
      }
      else {
        (yyval.node) = parser->ast()->createNodeExpansion((yyvsp[-5].intval), iterator, parser->ast()->createNodeReference(variable->name.c_str()), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node));
      }
    }
#line 3508 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 162:
#line 1290 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3516 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 163:
#line 1293 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3524 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 164:
#line 1299 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }
      
      (yyval.node) = (yyvsp[0].node);
    }
#line 3536 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 165:
#line 1306 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = (yyvsp[0].node);
    }
#line 3548 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 166:
#line 1316 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval)); 
    }
#line 3556 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 167:
#line 1319 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3564 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 168:
#line 1322 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueNull();
    }
#line 3572 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 169:
#line 1325 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 3580 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 170:
#line 1328 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(false);
    }
#line 3588 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 171:
#line 1334 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = parser->ast()->createNodeCollection((yyvsp[0].strval), TRI_TRANSACTION_WRITE);
    }
#line 3600 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 172:
#line 1341 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = parser->ast()->createNodeCollection((yyvsp[0].strval), TRI_TRANSACTION_WRITE);
    }
#line 3612 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 173:
#line 1348 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }
      
      if (strlen((yyvsp[0].strval)) < 2 || (yyvsp[0].strval)[0] != '@') {
        parser->registerParseError(TRI_ERROR_QUERY_BIND_PARAMETER_TYPE, TRI_errno_string(TRI_ERROR_QUERY_BIND_PARAMETER_TYPE), (yyvsp[0].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.node) = parser->ast()->createNodeParameter((yyvsp[0].strval));
    }
#line 3628 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 174:
#line 1362 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeParameter((yyvsp[0].strval));
    }
#line 3636 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 175:
#line 1368 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3648 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 176:
#line 1375 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3660 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 177:
#line 1384 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3668 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;


#line 3672 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, parser, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, parser, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, parser);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, parser);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, parser, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, parser);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, parser);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
