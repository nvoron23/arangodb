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
    T_NIN = 313,
    UMINUS = 314,
    UPLUS = 315,
    FUNCCALL = 316,
    REFERENCE = 317,
    INDEXED = 318,
    EXPANSION = 319
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

#line 199 "arangod/Aql/grammar.cpp" /* yacc.c:355  */
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


#line 262 "arangod/Aql/grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   830

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  79
/* YYNRULES -- Number of rules.  */
#define YYNRULES  171
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  294

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

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
       2,     2,     2,     2,     2,     2,    65,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   213,   213,   215,   217,   219,   221,   223,   228,   230,
     235,   239,   245,   247,   252,   254,   256,   258,   260,   262,
     267,   276,   284,   289,   291,   296,   303,   313,   313,   327,
     343,   370,   397,   429,   459,   461,   466,   473,   476,   482,
     496,   513,   513,   527,   527,   538,   541,   547,   553,   556,
     559,   562,   568,   573,   580,   588,   591,   597,   608,   619,
     628,   640,   645,   654,   666,   671,   674,   680,   680,   732,
     735,   738,   741,   744,   747,   753,   760,   777,   777,   789,
     792,   795,   801,   804,   807,   810,   813,   816,   819,   822,
     825,   828,   831,   834,   837,   840,   843,   849,   855,   857,
     862,   865,   865,   884,   887,   893,   896,   902,   902,   911,
     913,   918,   921,   927,   930,   944,   944,   953,   955,   960,
     962,   967,   970,   973,   988,   991,   997,  1000,  1006,  1009,
    1012,  1018,  1021,  1027,  1030,  1036,  1040,  1047,  1047,  1056,
    1060,  1067,  1070,  1076,  1118,  1121,  1124,  1127,  1134,  1144,
    1144,  1160,  1175,  1189,  1203,  1203,  1254,  1257,  1263,  1270,
    1280,  1283,  1286,  1289,  1292,  1298,  1305,  1312,  1326,  1332,
    1339,  1348
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
  "\",\"", "\"(\"", "\")\"", "\"{\"", "\"}\"", "\"[\"", "\"]\"", "T_NIN",
  "UMINUS", "UPLUS", "FUNCCALL", "REFERENCE", "INDEXED", "EXPANSION",
  "'.'", "$accept", "query", "optional_post_modification_lets",
  "optional_post_modification_block",
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
  "reference", "$@10", "$@11", "simple_value", "numeric_value",
  "value_literal", "collection_name", "bind_parameter",
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
     315,   316,   317,   318,   319,    46
};
# endif

#define YYPACT_NINF -274

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-274)))

#define YYTABLE_NINF -168

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -274,    14,   807,  -274,    22,    22,   440,   440,    28,  -274,
     122,   440,   440,   440,   440,  -274,  -274,  -274,  -274,  -274,
      69,  -274,  -274,  -274,  -274,    38,    38,    38,    38,    38,
    -274,    49,    35,  -274,    55,   440,  -274,  -274,  -274,   -29,
    -274,  -274,  -274,  -274,   440,   440,   440,   440,  -274,  -274,
     759,    40,  -274,  -274,  -274,  -274,  -274,  -274,  -274,   -43,
    -274,  -274,  -274,   759,    67,    68,    22,   440,    45,  -274,
    -274,   578,   578,  -274,   485,  -274,   524,   440,    22,    68,
      71,    58,  -274,  -274,  -274,  -274,  -274,   440,    22,   440,
     759,    85,    74,    74,    74,   312,  -274,   -16,   440,   440,
      96,   440,   440,   440,   440,   440,   440,   440,   440,   440,
     440,   440,   440,   440,   440,   440,    83,    72,   200,    20,
      98,    66,  -274,    70,  -274,   117,   104,  -274,   356,   122,
     460,     1,    68,    68,   440,    68,   440,    68,   614,   124,
    -274,    66,    68,  -274,  -274,  -274,   759,  -274,   759,   440,
    -274,   106,  -274,  -274,   109,   440,   108,   114,  -274,   118,
     759,   110,   119,   685,   440,   225,   650,   501,   501,    34,
      34,    34,    34,   -28,   -28,    74,    74,    74,   632,    60,
    -274,   407,  -274,   156,   125,  -274,  -274,    22,  -274,    22,
     440,   440,  -274,  -274,  -274,  -274,  -274,    39,    21,    46,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,   578,  -274,   578,
    -274,   440,   440,    22,  -274,   669,  -274,   440,   276,  -274,
     -16,   440,  -274,   440,   685,   440,   759,   121,  -274,  -274,
     120,  -274,  -274,   167,  -274,  -274,   759,  -274,    68,    68,
     560,   705,   126,  -274,    26,   759,   127,  -274,   759,   759,
     759,  -274,  -274,   440,   440,   169,  -274,  -274,  -274,  -274,
     440,  -274,    22,  -274,  -274,  -274,   440,   129,   440,  -274,
     759,   440,   170,   578,  -274,   130,   741,   759,   376,   440,
     128,    68,    54,  -274,   440,   759,  -274,  -274,  -274,   131,
    -274,   759,    54,  -274
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      12,     0,     0,     1,     0,     0,     0,     0,    27,    43,
       0,     0,     0,     0,     0,    67,    13,    14,    16,    15,
      37,    17,    18,    19,     2,    10,    10,    10,    10,    10,
     171,     0,    22,    23,     0,   141,   162,   163,   164,   143,
     160,   158,   159,   168,     0,     0,     0,   149,   115,   107,
      21,    77,   147,    69,    70,    71,   144,   105,   106,    73,
     161,    72,   145,    54,     0,   113,     0,     0,    52,   156,
     157,     0,     0,    61,     0,    64,     0,     0,     0,   113,
     113,     0,     3,     4,     5,     6,     7,     0,     0,     0,
     142,     0,    81,    79,    80,     0,    12,   117,   109,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    29,    28,    34,     0,    44,    45,    48,     0,
       0,     0,   113,   113,     0,   113,     0,   113,     0,    38,
      30,    41,   113,    31,     9,    11,    20,    24,    25,     0,
     148,     0,   169,   170,     0,     0,     0,   118,   119,     0,
     111,     0,   110,    95,     0,    83,    82,    89,    90,    91,
      92,    93,    94,    84,    85,    86,    87,    88,     0,    74,
      76,   101,   124,     0,   154,   151,   152,     0,   114,     0,
       0,     0,    49,    50,    47,    51,    53,   143,   160,   168,
      55,   165,   166,   167,    56,    57,    58,     0,    59,     0,
      62,     0,     0,     0,    32,     0,   150,     0,     0,   116,
       0,     0,   108,     0,    96,     0,   100,     0,   103,    12,
      99,   153,   125,   126,    26,    35,    36,    46,   113,   113,
       0,   113,    42,    39,     0,   123,     0,   120,   121,   112,
      97,    78,   102,   101,     0,   128,    60,    63,    65,    66,
       0,    33,     0,   133,   139,   137,     0,   140,     0,   104,
     127,     0,   131,     0,    40,     0,     0,   122,   129,     0,
       0,   113,     0,   146,     0,   132,   155,    68,   135,   138,
     134,   130,     0,   136
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -274,   -89,  -274,    88,  -274,  -274,  -274,  -274,   102,  -274,
      99,   188,  -274,  -274,  -274,  -274,    23,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,    18,  -274,  -274,   134,   -66,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,    24,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,   -42,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,   -77,  -117,  -274,  -274,  -274,    -4,
    -274,  -274,  -274,  -274,  -273,  -274,  -274,  -274,  -274,  -274,
    -274,  -274,   -95,  -274,    16,    87,   -10,  -274,    -3
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    81,    82,     2,    16,    17,    18,    19,    32,
      33,    65,    20,    66,    21,   123,   124,    80,   242,   142,
     213,    22,    67,   126,   127,   194,    23,    24,   132,    25,
      26,    73,    27,    75,    28,   260,    29,    77,   128,    51,
      52,   117,    53,    54,    55,   227,   228,   229,   230,    56,
      57,    98,   161,   162,   122,    58,    97,   156,   157,   158,
     184,   255,   272,   280,   265,   289,   266,   275,    91,    59,
      96,   233,    68,    60,    61,   200,    62,   159,    34
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      70,    31,   140,   143,   188,   100,   133,   151,   135,   288,
     137,   152,   153,   118,     3,   154,   111,   112,   113,   293,
     -75,  -166,   119,   -75,   188,  -166,    69,  -166,   201,   202,
      50,    63,   203,   195,   196,    71,    72,    74,    76,  -165,
     155,    64,    -8,  -165,    -8,  -165,  -167,   185,  -166,    30,
    -167,    43,  -167,   263,   264,   205,   206,    43,   208,    90,
     210,    87,     5,   125,     7,   214,  -165,   100,    92,    93,
      94,    95,  -166,  -167,  -166,   139,   109,   110,   111,   112,
     113,   263,    64,    78,   115,    43,    88,    89,   -75,   116,
    -165,   -75,  -165,   100,   120,   121,   129,  -167,   141,  -167,
     149,   138,   109,   110,   111,   112,   113,   100,   164,   186,
     180,   146,   187,   148,    83,    84,    85,    86,    70,    70,
      48,   189,   160,   163,   181,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     252,   238,   183,   239,    69,    69,    36,    37,    38,   190,
      40,    41,    42,    43,   163,   191,   212,   217,   207,   216,
     209,   256,   257,   219,   261,   220,   221,   222,    99,   232,
     223,   253,   254,   215,   251,   268,   279,   262,   271,   218,
    -134,   282,   292,   144,   234,   286,   125,   147,   224,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   287,   226,   115,   281,    79,   237,
     243,   269,   235,   231,   236,   145,   247,    35,   204,     0,
       0,     0,     0,     0,    36,    37,    38,    39,    40,    41,
      42,    43,     0,    44,   267,   240,   241,    99,     0,     0,
       0,   245,    45,    46,   182,   248,     0,   249,     0,   250,
       0,     0,    47,     0,    48,     0,    49,     0,   100,   274,
       0,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   290,     0,     0,   115,     0,   226,   270,     0,
       0,     0,   290,     0,   273,     0,     0,     0,    99,     0,
     276,     0,   277,     0,     0,   278,     0,     0,     0,     0,
       0,     0,     0,   285,     0,     0,     0,     0,   291,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    99,     0,   115,     0,     0,     0,
       0,     0,     0,   246,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
       0,     0,   115,     0,     0,   150,   192,   193,    99,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,    37,    38,     0,    40,    41,    42,    43,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,     0,     0,   115,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    35,     0,   115,   284,     0,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,     0,
      44,     0,     0,     0,     0,     0,     0,     0,     0,    45,
      46,     0,     0,     0,     0,     0,     0,    35,     0,    47,
     -98,    48,     0,    49,    36,    37,    38,    39,    40,    41,
      42,    43,     0,    44,     0,     0,     0,    35,     0,     0,
       0,     0,    45,    46,    36,    37,    38,   197,   198,    41,
      42,   199,    47,    44,    48,     0,    49,   130,   134,   131,
       0,     0,    45,    46,     0,     0,     0,     0,     0,     0,
       0,     0,    47,    99,    48,     0,    49,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,     0,   100,   115,   130,   136,   131,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
       0,   115,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    99,     0,   115,     0,     0,     0,     0,     0,
       0,   258,   259,     0,     0,     0,     0,     0,     0,     0,
     130,     0,   131,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,     0,
     115,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    99,     0,   115,     0,
       0,     0,     0,     0,   211,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    99,     0,   115,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     225,    99,   115,   100,   101,   244,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,     0,
     115,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    99,   100,   115,
       0,     0,     0,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   121,     0,     0,   115,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    99,     0,   115,     0,     0,     0,   283,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    99,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
       0,   115,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,     0,   115,
       4,     5,     6,     7,     8,     9,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,    14,
      15
};

static const yytype_int16 yycheck[] =
{
      10,     4,    79,    80,   121,    33,    72,    96,    74,   282,
      76,    27,    28,    56,     0,    31,    44,    45,    46,   292,
      49,     0,    65,    52,   141,     4,    10,     6,    27,    28,
       6,     7,    31,   128,   129,    11,    12,    13,    14,     0,
      56,    13,     4,     4,     6,     6,     0,    27,    27,    27,
       4,    31,     6,    27,    28,   132,   133,    31,   135,    35,
     137,    12,     4,    66,     6,   142,    27,    33,    44,    45,
      46,    47,    51,    27,    53,    78,    42,    43,    44,    45,
      46,    27,    13,    14,    50,    31,    51,    32,    49,    49,
      51,    52,    53,    33,    27,    27,    51,    51,    27,    53,
      15,    77,    42,    43,    44,    45,    46,    33,    12,   119,
      27,    87,    14,    89,    26,    27,    28,    29,   128,   129,
      54,    51,    98,    99,    52,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     229,   207,   118,   209,   128,   129,    24,    25,    26,    32,
      28,    29,    30,    31,   130,    51,    32,    48,   134,    53,
     136,   238,   239,    55,   241,    51,    48,    57,    12,    44,
      51,    51,     5,   149,    53,    48,     6,    51,     9,   155,
      51,    51,    51,    81,   187,    57,   189,    88,   164,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,   281,   181,    50,   273,    20,   191,
     213,   253,   189,    57,   190,    81,   220,    17,   131,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    33,   244,   211,   212,    12,    -1,    -1,
      -1,   217,    42,    43,    44,   221,    -1,   223,    -1,   225,
      -1,    -1,    52,    -1,    54,    -1,    56,    -1,    33,   262,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,   282,    -1,    -1,    50,    -1,   253,   254,    -1,
      -1,    -1,   292,    -1,   260,    -1,    -1,    -1,    12,    -1,
     266,    -1,   268,    -1,    -1,   271,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   279,    -1,    -1,    -1,    -1,   284,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    12,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    -1,    50,    -1,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    25,    26,    -1,    28,    29,    30,    31,    12,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    -1,    50,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    17,    -1,    50,    51,    -1,    -1,
      -1,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    17,    -1,    52,
      53,    54,    -1,    56,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    33,    -1,    -1,    -1,    17,    -1,    -1,
      -1,    -1,    42,    43,    24,    25,    26,    27,    28,    29,
      30,    31,    52,    33,    54,    -1,    56,    12,    13,    14,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    12,    54,    -1,    56,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    33,    50,    12,    13,    14,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    12,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      12,    -1,    14,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    -1,
      50,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    12,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    12,    -1,    50,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    12,    50,    33,    34,    16,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    -1,    -1,    -1,
      50,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    12,    33,    50,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    27,    -1,    -1,    50,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    12,    -1,    50,    -1,    -1,    -1,    18,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      -1,    50,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    -1,    50,
       3,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
      23
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    67,    70,     0,     3,     4,     5,     6,     7,     8,
       9,    19,    20,    21,    22,    23,    71,    72,    73,    74,
      78,    80,    87,    92,    93,    95,    96,    98,   100,   102,
      27,   144,    75,    76,   144,    17,    24,    25,    26,    27,
      28,    29,    30,    31,    33,    42,    43,    52,    54,    56,
     104,   105,   106,   108,   109,   110,   115,   116,   121,   135,
     139,   140,   142,   104,    13,    77,    79,    88,   138,   140,
     142,   104,   104,    97,   104,    99,   104,   103,    14,    77,
      83,    68,    69,    69,    69,    69,    69,    12,    51,    32,
     104,   134,   104,   104,   104,   104,   136,   122,   117,    12,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    50,    49,   107,    56,    65,
      27,    27,   120,    81,    82,   144,    89,    90,   104,    51,
      12,    14,    94,    94,    13,    94,    13,    94,   104,   144,
     120,    27,    85,   120,    74,    93,   104,    76,   104,    15,
      53,    67,    27,    28,    31,    56,   123,   124,   125,   143,
     104,   118,   119,   104,    12,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
      27,    52,    44,   104,   126,    27,   142,    14,   121,    51,
      32,    51,    10,    11,    91,   138,   138,    27,    28,    31,
     141,    27,    28,    31,   141,   120,   120,   104,   120,   104,
     120,    20,    32,    86,   120,   104,    53,    48,   104,    55,
      51,    48,    57,    51,   104,    48,   104,   111,   112,   113,
     114,    57,    44,   137,   144,    82,   104,    90,    94,    94,
     104,   104,    84,   144,    16,   104,    57,   125,   104,   104,
     104,    53,    67,    51,     5,   127,   120,   120,    21,    22,
     101,   120,    51,    27,    28,   130,   132,   142,    48,   112,
     104,     9,   128,   104,   144,   133,   104,   104,   104,     6,
     129,    94,    51,    18,    51,   104,    57,   120,   130,   131,
     142,   104,    51,   130
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    66,    67,    67,    67,    67,    67,    67,    68,    68,
      69,    69,    70,    70,    71,    71,    71,    71,    71,    71,
      72,    73,    74,    75,    75,    76,    77,    79,    78,    80,
      80,    80,    80,    80,    81,    81,    82,    83,    83,    84,
      84,    86,    85,    88,    87,    89,    89,    90,    91,    91,
      91,    91,    92,    92,    93,    94,    94,    95,    96,    97,
      97,    98,    99,    99,   100,   101,   101,   103,   102,   104,
     104,   104,   104,   104,   104,   105,   105,   107,   106,   108,
     108,   108,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   110,   111,   111,
     112,   113,   112,   114,   114,   115,   115,   117,   116,   118,
     118,   119,   119,   120,   120,   122,   121,   123,   123,   124,
     124,   125,   125,   125,   126,   126,   127,   127,   128,   128,
     128,   129,   129,   130,   130,   131,   131,   133,   132,   132,
     132,   134,   134,   135,   135,   135,   135,   135,   135,   136,
     135,   135,   135,   135,   137,   135,   138,   138,   139,   139,
     140,   140,   140,   140,   140,   141,   141,   141,   142,   143,
     143,   144
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     3,     3,     3,     3,     0,     2,
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       4,     2,     2,     1,     3,     3,     4,     0,     3,     3,
       3,     3,     4,     6,     1,     3,     3,     0,     2,     1,
       3,     0,     3,     0,     3,     1,     3,     2,     0,     1,
       1,     1,     2,     4,     2,     2,     2,     4,     4,     3,
       5,     2,     3,     5,     2,     1,     1,     0,     9,     1,
       1,     1,     1,     1,     3,     1,     3,     0,     5,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     4,     5,     0,     1,
       1,     0,     2,     1,     3,     1,     1,     0,     4,     0,
       1,     1,     3,     0,     2,     0,     4,     0,     1,     1,
       3,     3,     5,     3,     1,     2,     0,     2,     0,     2,
       4,     0,     2,     1,     1,     1,     3,     0,     4,     1,
       1,     0,     1,     1,     1,     1,     8,     1,     3,     0,
       4,     3,     3,     4,     0,     8,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
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
#line 213 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1799 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 3:
#line 215 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1806 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 4:
#line 217 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1813 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 5:
#line 219 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1820 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 6:
#line 221 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1827 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 7:
#line 223 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1834 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 8:
#line 228 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1841 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 9:
#line 230 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1848 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 10:
#line 235 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // still need to close the scope opened by the data-modification statement
      parser->ast()->scopes()->endNested();
    }
#line 1857 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 11:
#line 239 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // the RETURN statement will close the scope opened by the data-modification statement
    }
#line 1865 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 12:
#line 245 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1872 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 13:
#line 247 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1879 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 14:
#line 252 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1886 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 15:
#line 254 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1893 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 16:
#line 256 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1900 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 17:
#line 258 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1907 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 18:
#line 260 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1914 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 19:
#line 262 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1921 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 20:
#line 267 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
     
      auto node = parser->ast()->createNodeFor((yyvsp[-2].strval), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1932 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 21:
#line 276 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // operand is a reference. can use it directly
      auto node = parser->ast()->createNodeFilter((yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1942 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 22:
#line 284 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1949 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 23:
#line 289 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1956 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 24:
#line 291 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1963 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 25:
#line 296 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeLet((yyvsp[-2].strval), (yyvsp[0].node), true);
      parser->ast()->addOperation(node);
    }
#line 1972 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 26:
#line 303 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! TRI_CaseEqualString((yyvsp[-2].strval), "COUNT")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'COUNT'", (yyvsp[-2].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 1984 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 27:
#line 313 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 1993 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 28:
#line 316 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      auto list = static_cast<AstNode*>(parser->popStack());

      if (list == nullptr) {
        ABORT_OOM
      }
      (yyval.node) = list;
    }
#line 2006 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 29:
#line 327 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2027 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 30:
#line 343 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2059 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 31:
#line 370 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2091 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 32:
#line 397 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2128 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 33:
#line 429 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2160 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 34:
#line 459 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2167 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 35:
#line 461 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2174 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 36:
#line 466 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeAssign((yyvsp[-2].strval), (yyvsp[0].node));
      parser->pushArrayElement(node);
    }
#line 2183 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 37:
#line 473 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = nullptr;
    }
#line 2191 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 38:
#line 476 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);
    }
#line 2199 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 39:
#line 482 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2218 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 40:
#line 496 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2237 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 41:
#line 513 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! TRI_CaseEqualString((yyvsp[0].strval), "KEEP")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'KEEP'", (yyvsp[0].strval), yylloc.first_line, yylloc.first_column);
      }

      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2250 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 42:
#line 520 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode*>(parser->popStack());
      (yyval.node) = list;
    }
#line 2259 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 43:
#line 527 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2268 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 44:
#line 530 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode const*>(parser->popStack());
      auto node = parser->ast()->createNodeSort(list);
      parser->ast()->addOperation(node);
    }
#line 2278 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 45:
#line 538 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2286 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 46:
#line 541 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2294 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 47:
#line 547 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeSortElement((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 2302 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 48:
#line 553 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 2310 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 49:
#line 556 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 2318 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 50:
#line 559 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(false);
    }
#line 2326 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 51:
#line 562 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2334 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 52:
#line 568 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto offset = parser->ast()->createNodeValueInt(0);
      auto node = parser->ast()->createNodeLimit(offset, (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2344 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 53:
#line 573 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeLimit((yyvsp[-2].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2353 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 54:
#line 580 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeReturn((yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->ast()->scopes()->endNested();
    }
#line 2363 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 55:
#line 588 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2371 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 56:
#line 591 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       (yyval.node) = (yyvsp[0].node);
     }
#line 2379 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 57:
#line 597 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REMOVE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }
      auto node = parser->ast()->createNodeRemove((yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2392 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 58:
#line 608 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_INSERT, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }
      auto node = parser->ast()->createNodeInsert((yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2405 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 59:
#line 619 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_UPDATE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeUpdate(nullptr, (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2419 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 60:
#line 628 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_UPDATE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeUpdate((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2433 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 61:
#line 640 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2440 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 62:
#line 645 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REPLACE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeReplace(nullptr, (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2454 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 63:
#line 654 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REPLACE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeReplace((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2468 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 64:
#line 666 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2475 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 65:
#line 671 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = static_cast<int64_t>(NODE_TYPE_UPDATE);
    }
#line 2483 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 66:
#line 674 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = static_cast<int64_t>(NODE_TYPE_REPLACE);
    }
#line 2491 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 67:
#line 680 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      // reserve a variable named "$OLD", we might need it in the update expression
      // and in a later return thing
      parser->pushStack(parser->ast()->createNodeVariable(Variable::NAME_OLD, true));
    }
#line 2501 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 68:
#line 684 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2551 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 69:
#line 732 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2559 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 70:
#line 735 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2567 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 71:
#line 738 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2575 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 72:
#line 741 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2583 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 73:
#line 744 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2591 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 74:
#line 747 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeRange((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2599 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 75:
#line 753 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);

      if ((yyval.strval) == nullptr) {
        ABORT_OOM
      }
    }
#line 2611 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 76:
#line 760 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2630 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 77:
#line 777 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushStack((yyvsp[0].strval));

      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2641 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 78:
#line 782 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode const*>(parser->popStack());
      (yyval.node) = parser->ast()->createNodeFunctionCall(static_cast<char const*>(parser->popStack()), list);
    }
#line 2650 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 79:
#line 789 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_PLUS, (yyvsp[0].node));
    }
#line 2658 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 80:
#line 792 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_MINUS, (yyvsp[0].node));
    }
#line 2666 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 81:
#line 795 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_NOT, (yyvsp[0].node));
    }
#line 2674 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 82:
#line 801 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_OR, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2682 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 83:
#line 804 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_AND, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2690 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 84:
#line 807 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_PLUS, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2698 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 85:
#line 810 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_MINUS, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2706 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 86:
#line 813 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_TIMES, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2714 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 87:
#line 816 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_DIV, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2722 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 88:
#line 819 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_MOD, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2730 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 89:
#line 822 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_EQ, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2738 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 90:
#line 825 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_NE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2746 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 91:
#line 828 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_LT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2754 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 92:
#line 831 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_GT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2762 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 93:
#line 834 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_LE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2770 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 94:
#line 837 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_GE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2778 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 95:
#line 840 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_IN, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2786 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 96:
#line 843 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_NIN, (yyvsp[-3].node), (yyvsp[0].node));
    }
#line 2794 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 97:
#line 849 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeTernaryOperator((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2802 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 98:
#line 855 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2809 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 99:
#line 857 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2816 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 100:
#line 862 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2824 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 101:
#line 865 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (parser->isModificationQuery()) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected subquery after data-modification operation", yylloc.first_line, yylloc.first_column);
      }
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_SUBQUERY);
      parser->ast()->startSubQuery();
    }
#line 2836 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 102:
#line 871 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      AstNode* node = parser->ast()->endSubQuery();
      parser->ast()->scopes()->endCurrent();

      std::string const variableName = parser->ast()->variables()->nextName();
      auto subQuery = parser->ast()->createNodeLet(variableName.c_str(), node, false);
      parser->ast()->addOperation(subQuery);

      (yyval.node) = parser->ast()->createNodeReference(variableName.c_str());
    }
#line 2851 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 103:
#line 884 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2859 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 104:
#line 887 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2867 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 105:
#line 893 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2875 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 106:
#line 896 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2883 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 107:
#line 902 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2892 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 108:
#line 905 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = static_cast<AstNode*>(parser->popStack());
    }
#line 2900 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 109:
#line 911 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2907 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 110:
#line 913 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2914 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 111:
#line 918 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2922 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 112:
#line 921 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2930 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 113:
#line 927 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 2938 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 114:
#line 930 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[-1].strval) == nullptr || (yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }

      if (! TRI_CaseEqualString((yyvsp[-1].strval), "OPTIONS")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'OPTIONS'", (yyvsp[-1].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.node) = (yyvsp[0].node);
    }
#line 2954 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 115:
#line 944 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeObject();
      parser->pushStack(node);
    }
#line 2963 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 116:
#line 947 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = static_cast<AstNode*>(parser->popStack());
    }
#line 2971 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 117:
#line 953 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2978 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 118:
#line 955 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2985 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 119:
#line 960 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2992 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 120:
#line 962 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2999 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 121:
#line 967 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushObjectElement((yyvsp[-2].strval), (yyvsp[0].node));
    }
#line 3007 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 122:
#line 970 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushObjectElement((yyvsp[-3].node), (yyvsp[0].node));
    }
#line 3015 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 123:
#line 973 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3032 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 124:
#line 988 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = 1;
    }
#line 3040 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 125:
#line 991 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = (yyvsp[-1].intval) + 1;
    }
#line 3048 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 126:
#line 997 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3056 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 127:
#line 1000 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3064 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 128:
#line 1006 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3072 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 129:
#line 1009 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeArrayLimit(nullptr, (yyvsp[0].node));
    }
#line 3080 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 130:
#line 1012 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeArrayLimit((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 3088 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 131:
#line 1018 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3096 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 132:
#line 1021 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3104 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 133:
#line 1027 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval));
    }
#line 3112 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 134:
#line 1030 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3120 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 135:
#line 1036 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       auto node = static_cast<AstNode*>(parser->peekStack());
       node->addMember((yyvsp[0].node));
     }
#line 3129 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 136:
#line 1040 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       auto node = static_cast<AstNode*>(parser->peekStack());
       node->addMember((yyvsp[0].node));
     }
#line 3138 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 137:
#line 1047 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 3147 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 138:
#line 1051 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // edgecollection, nodecollection(, nodecollection)*
      auto node = static_cast<AstNode*>(parser->popStack());
      (yyval.node) = parser->ast()->createNodeCollectionPair((yyvsp[-3].node), node);
    }
#line 3157 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 139:
#line 1056 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // graph name
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval));
    }
#line 3166 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 140:
#line 1060 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // graph name
      (yyval.node) = (yyvsp[0].node);
    }
#line 3175 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 141:
#line 1067 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3183 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 142:
#line 1070 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3191 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 143:
#line 1076 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3238 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 144:
#line 1118 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3246 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 145:
#line 1121 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3254 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 146:
#line 1124 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeTraversal((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); 
    }
#line 3262 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 147:
#line 1127 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
      
      if ((yyval.node) == nullptr) {
        ABORT_OOM
      }
    }
#line 3274 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 148:
#line 1134 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3289 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 149:
#line 1144 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (parser->isModificationQuery()) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected subquery after data-modification operation", yylloc.first_line, yylloc.first_column);
      }
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_SUBQUERY);
      parser->ast()->startSubQuery();
    }
#line 3301 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 150:
#line 1150 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      AstNode* node = parser->ast()->endSubQuery();
      parser->ast()->scopes()->endCurrent();

      std::string const variableName = parser->ast()->variables()->nextName();
      auto subQuery = parser->ast()->createNodeLet(variableName.c_str(), node, false);
      parser->ast()->addOperation(subQuery);

      (yyval.node) = parser->ast()->createNodeReference(variableName.c_str());
    }
#line 3316 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 151:
#line 1160 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3336 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 152:
#line 1175 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3355 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 153:
#line 1189 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3374 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 154:
#line 1203 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3403 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 155:
#line 1226 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3433 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 156:
#line 1254 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3441 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 157:
#line 1257 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3449 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 158:
#line 1263 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }
      
      (yyval.node) = (yyvsp[0].node);
    }
#line 3461 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 159:
#line 1270 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = (yyvsp[0].node);
    }
#line 3473 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 160:
#line 1280 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval)); 
    }
#line 3481 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 161:
#line 1283 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3489 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 162:
#line 1286 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueNull();
    }
#line 3497 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 163:
#line 1289 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 3505 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 164:
#line 1292 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(false);
    }
#line 3513 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 165:
#line 1298 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = parser->ast()->createNodeCollection((yyvsp[0].strval), TRI_TRANSACTION_WRITE);
    }
#line 3525 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 166:
#line 1305 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = parser->ast()->createNodeCollection((yyvsp[0].strval), TRI_TRANSACTION_WRITE);
    }
#line 3537 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 167:
#line 1312 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }
      
      if (strlen((yyvsp[0].strval)) < 2 || (yyvsp[0].strval)[0] != '@') {
        parser->registerParseError(TRI_ERROR_QUERY_BIND_PARAMETER_TYPE, TRI_errno_string(TRI_ERROR_QUERY_BIND_PARAMETER_TYPE), (yyvsp[0].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.node) = parser->ast()->createNodeParameter((yyvsp[0].strval));
    }
#line 3553 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 168:
#line 1326 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeParameter((yyvsp[0].strval));
    }
#line 3561 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 169:
#line 1332 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3573 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 170:
#line 1339 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3585 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 171:
#line 1348 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3593 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;


#line 3597 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
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
