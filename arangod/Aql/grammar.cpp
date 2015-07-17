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
#define YYLAST   860

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  79
/* YYNRULES -- Number of rules.  */
#define YYNRULES  175
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  304

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
       0,   216,   216,   218,   220,   222,   224,   226,   231,   233,
     238,   242,   248,   250,   255,   257,   259,   261,   263,   265,
     270,   276,   281,   286,   294,   302,   307,   309,   314,   321,
     331,   331,   345,   361,   388,   415,   447,   477,   479,   484,
     491,   494,   500,   514,   531,   531,   545,   545,   556,   559,
     565,   571,   574,   577,   580,   586,   591,   598,   606,   609,
     615,   626,   637,   646,   658,   663,   672,   684,   689,   692,
     698,   698,   750,   753,   756,   759,   762,   765,   771,   778,
     795,   795,   807,   810,   813,   819,   822,   825,   828,   831,
     834,   837,   840,   843,   846,   849,   852,   855,   858,   861,
     867,   873,   875,   880,   883,   883,   902,   905,   911,   914,
     920,   920,   929,   931,   936,   939,   945,   948,   962,   962,
     971,   973,   978,   980,   985,   988,   991,  1006,  1009,  1015,
    1018,  1024,  1027,  1030,  1036,  1039,  1045,  1048,  1055,  1059,
    1066,  1072,  1071,  1080,  1084,  1093,  1096,  1099,  1105,  1147,
    1150,  1153,  1160,  1170,  1170,  1186,  1201,  1215,  1229,  1229,
    1280,  1283,  1289,  1296,  1306,  1309,  1312,  1315,  1318,  1324,
    1331,  1338,  1352,  1358,  1365,  1374
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
     315,   316,   317,   318,   319,   320,   321,   322,    46
};
# endif

#define YYPACT_NINF -267

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-267)))

#define YYTABLE_NINF -172

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -267,    21,   229,  -267,    28,    28,   784,   784,    36,  -267,
     128,   784,   784,   784,   784,  -267,  -267,  -267,  -267,  -267,
      22,  -267,  -267,  -267,  -267,    81,    81,    81,    81,    81,
    -267,    -3,    12,  -267,    40,  -267,  -267,  -267,    30,  -267,
    -267,  -267,  -267,   784,   784,   784,   784,  -267,  -267,   653,
      43,  -267,  -267,  -267,  -267,  -267,  -267,  -267,   -36,  -267,
    -267,  -267,   653,    62,    93,    28,   784,    57,  -267,  -267,
     544,   544,  -267,   472,  -267,   490,   784,    28,    93,    98,
     107,  -267,  -267,  -267,  -267,  -267,   698,    28,    28,   784,
      94,    94,    94,   333,  -267,    -2,   784,   784,   119,   784,
     784,   784,   784,   784,   784,   784,   784,   784,   784,   784,
     784,   784,   784,   784,   105,    97,   718,    -4,   141,   108,
    -267,   110,  -267,   131,   113,  -267,   377,   128,   804,    25,
      93,    93,   784,    93,   784,    93,   580,   133,  -267,   108,
      93,  -267,  -267,  -267,  -267,  -267,  -267,   653,   784,     6,
    -267,   653,  -267,   114,  -267,  -267,   123,   784,   118,   124,
    -267,   126,   653,   121,   125,   449,   784,   671,   160,   413,
     413,     0,     0,     0,     0,    14,    14,    94,    94,    94,
     598,    91,  -267,   751,  -267,   261,   135,  -267,  -267,    28,
    -267,    28,   784,   784,  -267,  -267,  -267,  -267,  -267,   117,
      24,    61,  -267,  -267,  -267,  -267,  -267,  -267,  -267,   544,
    -267,   544,  -267,   784,   784,    28,  -267,   433,    87,    28,
    -267,   784,   297,  -267,    -2,   784,  -267,   784,   449,   784,
     653,   129,  -267,  -267,   134,  -267,  -267,   179,  -267,  -267,
     653,  -267,    93,    93,   526,   635,   136,  -267,    53,  -267,
     138,  -267,  -267,   784,   180,   653,   143,  -267,   653,   653,
     653,  -267,  -267,   784,   784,   186,  -267,  -267,  -267,  -267,
     784,  -267,    28,  -267,  -267,  -267,   433,    87,   784,  -267,
     653,   784,   205,   544,  -267,    35,  -267,   784,   653,   397,
     784,   155,    93,  -267,   162,   433,   784,   653,  -267,  -267,
      35,  -267,   653,  -267
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      12,     0,     0,     1,     0,     0,     0,     0,    30,    46,
       0,     0,     0,     0,     0,    70,    13,    14,    16,    15,
      40,    17,    18,    19,     2,    10,    10,    10,    10,    10,
     175,     0,    25,    26,     0,   166,   167,   168,   148,   164,
     162,   163,   172,     0,     0,     0,   153,   118,   110,    24,
      80,   151,    72,    73,    74,   149,   108,   109,    76,   165,
      75,   150,    57,     0,   116,     0,     0,    55,   160,   161,
       0,     0,    64,     0,    67,     0,     0,     0,   116,   116,
       0,     3,     4,     5,     6,     7,     0,     0,     0,     0,
      84,    82,    83,     0,    12,   120,   112,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      32,    31,    37,     0,    47,    48,    51,     0,     0,     0,
     116,   116,     0,   116,     0,   116,     0,    41,    33,    44,
     116,    34,     9,    11,   145,   146,   147,    20,     0,     0,
      27,    28,   152,     0,   173,   174,     0,     0,     0,   121,
     122,     0,   114,     0,   113,    98,     0,    86,    85,    92,
      93,    94,    95,    96,    97,    87,    88,    89,    90,    91,
       0,    77,    79,   104,   127,     0,   158,   155,   156,     0,
     117,     0,     0,     0,    52,    53,    50,    54,    56,   148,
     164,   172,    58,   169,   170,   171,    59,    60,    61,     0,
      62,     0,    65,     0,     0,     0,    35,     0,     0,     0,
     154,     0,     0,   119,     0,     0,   111,     0,    99,     0,
     103,     0,   106,    12,   102,   157,   128,   129,    29,    38,
      39,    49,   116,   116,     0,   116,    45,    42,     0,   136,
     140,    21,   137,     0,     0,   126,     0,   123,   124,   115,
     100,    81,   105,   104,     0,   131,    63,    66,    68,    69,
       0,    36,     0,   144,   143,   141,     0,     0,     0,   107,
     130,     0,   134,     0,    43,     0,    22,     0,   125,   132,
       0,     0,   116,   138,   142,     0,     0,   135,   159,    71,
       0,    23,   133,   139
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -267,   -90,  -267,   112,  -267,  -267,  -267,  -267,   137,  -267,
     132,   196,  -267,  -267,  -267,  -267,    33,  -267,  -267,  -267,
    -267,  -267,  -267,  -267,    32,  -267,  -267,   146,   -61,  -267,
    -267,  -267,  -267,  -267,  -267,  -267,  -267,  -267,    -6,  -267,
    -267,  -267,  -267,  -267,  -267,  -267,   -35,  -267,  -267,  -267,
    -267,  -267,  -267,  -267,   -62,  -108,  -267,  -267,  -267,     5,
    -267,  -267,  -267,  -267,  -266,  -267,  -254,  -267,  -203,  -267,
    -267,  -267,   -11,  -267,     3,   102,    -8,  -267,    -1
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    80,    81,     2,    16,    17,    18,    19,    32,
      33,    64,    20,    65,    21,   121,   122,    79,   246,   140,
     215,    22,    66,   124,   125,   196,    23,    24,   130,    25,
      26,    72,    27,    74,    28,   270,    29,    76,   126,    50,
      51,   115,    52,    53,    54,   231,   232,   233,   234,    55,
      56,    96,   163,   164,   120,    57,    95,   158,   159,   160,
     186,   265,   282,   291,   250,   294,   251,   285,   148,    58,
      94,   237,    67,    59,    60,   202,    61,   161,    34
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      49,    62,    69,    31,   153,    70,    71,    73,    75,    86,
     131,   190,   133,    68,   135,   253,   138,   141,   218,   293,
     116,     3,   286,   187,  -170,   154,   155,    42,  -170,   156,
    -170,   190,   117,    98,   303,    63,    77,    90,    91,    92,
      93,   301,   107,   108,   109,   110,   111,    98,    87,    63,
     113,  -170,   203,   204,   157,    30,   205,   219,   109,   110,
     111,  -171,   249,    88,   123,  -171,    42,  -171,   207,   208,
     136,   210,    89,   212,   287,  -170,   137,  -170,   216,   -78,
     147,   273,   -78,   151,    42,    -8,   149,    -8,  -171,   118,
     162,   165,   114,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   127,   188,
     185,     5,  -171,     7,  -171,   197,   198,  -169,    69,    69,
     119,  -169,   165,  -169,    98,   139,   209,    98,   211,    68,
      68,   166,   182,   107,   108,   109,   110,   111,    82,    83,
      84,    85,   217,   262,  -169,   144,   145,   146,   242,   183,
     243,   222,    35,    36,    37,   189,    39,    40,    41,    42,
     228,   191,    47,   192,   193,   214,   -78,   220,  -169,   -78,
    -169,   221,    97,   223,   225,   224,   227,   230,   226,   236,
     266,   267,   261,   271,   264,   263,   240,   272,   238,   275,
     123,   278,   277,    98,    99,   281,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   244,   245,   252,
     113,   290,   298,   300,   247,   255,    78,   142,   254,   258,
     150,   259,   292,   260,   239,   241,   143,     0,   279,   257,
     299,   206,     4,     5,     6,     7,     8,     9,    10,     0,
     274,     0,     0,     0,     0,     0,     0,   276,    11,    12,
      13,    14,    15,     0,     0,     0,     0,   230,   280,     0,
       0,     0,     0,     0,   283,     0,     0,     0,   252,     0,
       0,   284,   288,    97,     0,   289,     0,   252,     0,     0,
       0,   295,     0,     0,   297,     0,     0,   252,     0,     0,
     302,     0,   252,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    97,
       0,   113,     0,     0,     0,     0,     0,     0,   235,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    97,     0,   113,     0,     0,
       0,     0,     0,     0,   256,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,   152,   194,   195,    97,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    35,    36,    37,     0,    39,    40,    41,    42,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    97,     0,   113,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    97,    98,   113,   296,   248,
       0,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     249,     0,     0,   113,    42,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,    98,   113,   128,   132,   129,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,   113,
       0,     0,   128,   134,   129,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    97,     0,
     113,     0,     0,     0,     0,     0,     0,   268,   269,     0,
       0,     0,     0,     0,     0,     0,   128,     0,   129,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    97,     0,   113,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   229,    97,   113,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   119,     0,     0,    97,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    97,     0,   113,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,    98,     0,     0,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,   113,    35,    36,    37,    38,    39,    40,    41,    42,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
      44,    45,    35,    36,    37,    38,    39,    40,    41,    42,
      46,    43,    47,     0,    48,     0,   144,   145,   146,     0,
      44,    45,   184,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,    48,    35,    36,    37,    38,    39,
      40,    41,    42,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,    44,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,  -101,    47,     0,    48,    35,    36,
      37,    38,    39,    40,    41,    42,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,    44,    45,    35,    36,
      37,   199,   200,    40,    41,   201,    46,    43,    47,     0,
      48,     0,     0,     0,     0,     0,    44,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
      48
};

static const yytype_int16 yycheck[] =
{
       6,     7,    10,     4,    94,    11,    12,    13,    14,    12,
      71,   119,    73,    10,    75,   218,    78,    79,    12,   285,
      56,     0,   276,    27,     0,    27,    28,    31,     4,    31,
       6,   139,    68,    33,   300,    13,    14,    43,    44,    45,
      46,   295,    42,    43,    44,    45,    46,    33,    51,    13,
      50,    27,    27,    28,    56,    27,    31,    51,    44,    45,
      46,     0,    27,    51,    65,     4,    31,     6,   130,   131,
      76,   133,    32,   135,   277,    51,    77,    53,   140,    49,
      86,    28,    52,    89,    31,     4,    87,     6,    27,    27,
      96,    97,    49,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,    51,   117,
     116,     4,    51,     6,    53,   126,   127,     0,   126,   127,
      27,     4,   128,     6,    33,    27,   132,    33,   134,   126,
     127,    12,    27,    42,    43,    44,    45,    46,    26,    27,
      28,    29,   148,   233,    27,    58,    59,    60,   209,    52,
     211,   157,    24,    25,    26,    14,    28,    29,    30,    31,
     166,    51,    54,    32,    51,    32,    49,    53,    51,    52,
      53,    48,    12,    55,    48,    51,    51,   183,    57,    44,
     242,   243,    53,   245,     5,    51,   192,    51,   189,    51,
     191,    48,    12,    33,    34,     9,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,   213,   214,   217,
      50,     6,    57,    51,   215,   221,    20,    80,   219,   225,
      88,   227,   283,   229,   191,   193,    80,    -1,   263,   224,
     292,   129,     3,     4,     5,     6,     7,     8,     9,    -1,
     248,    -1,    -1,    -1,    -1,    -1,    -1,   253,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    -1,   263,   264,    -1,
      -1,    -1,    -1,    -1,   270,    -1,    -1,    -1,   276,    -1,
      -1,   272,   278,    12,    -1,   281,    -1,   285,    -1,    -1,
      -1,   287,    -1,    -1,   290,    -1,    -1,   295,    -1,    -1,
     296,    -1,   300,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    12,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    12,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    -1,    50,    -1,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    25,    26,    -1,    28,    29,    30,    31,    12,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    12,    -1,    50,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    12,    33,    50,    51,    16,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      27,    -1,    -1,    50,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    33,    50,    12,    13,    14,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    -1,    -1,    -1,    50,
      -1,    -1,    12,    13,    14,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    -1,    50,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    12,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    -1,    14,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    -1,    50,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    12,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    -1,
      50,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    12,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    12,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    12,    -1,    50,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    -1,    50,    33,    -1,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    -1,
      -1,    50,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    24,    25,    26,    27,    28,    29,    30,    31,
      52,    33,    54,    -1,    56,    -1,    58,    59,    60,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    54,    -1,    56,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    24,    25,
      26,    27,    28,    29,    30,    31,    52,    33,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    54,    -1,
      56
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    70,    73,     0,     3,     4,     5,     6,     7,     8,
       9,    19,    20,    21,    22,    23,    74,    75,    76,    77,
      81,    83,    90,    95,    96,    98,    99,   101,   103,   105,
      27,   147,    78,    79,   147,    24,    25,    26,    27,    28,
      29,    30,    31,    33,    42,    43,    52,    54,    56,   107,
     108,   109,   111,   112,   113,   118,   119,   124,   138,   142,
     143,   145,   107,    13,    80,    82,    91,   141,   143,   145,
     107,   107,   100,   107,   102,   107,   106,    14,    80,    86,
      71,    72,    72,    72,    72,    72,    12,    51,    51,    32,
     107,   107,   107,   107,   139,   125,   120,    12,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    50,    49,   110,    56,    68,    27,    27,
     123,    84,    85,   147,    92,    93,   107,    51,    12,    14,
      97,    97,    13,    97,    13,    97,   107,   147,   123,    27,
      88,   123,    77,    96,    58,    59,    60,   107,   137,   147,
      79,   107,    53,    70,    27,    28,    31,    56,   126,   127,
     128,   146,   107,   121,   122,   107,    12,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,    27,    52,    44,   107,   129,    27,   145,    14,
     124,    51,    32,    51,    10,    11,    94,   141,   141,    27,
      28,    31,   144,    27,    28,    31,   144,   123,   123,   107,
     123,   107,   123,    20,    32,    89,   123,   107,    12,    51,
      53,    48,   107,    55,    51,    48,    57,    51,   107,    48,
     107,   114,   115,   116,   117,    57,    44,   140,   147,    85,
     107,    93,    97,    97,   107,   107,    87,   147,    16,    27,
     133,   135,   145,   137,   147,   107,    57,   128,   107,   107,
     107,    53,    70,    51,     5,   130,   123,   123,    21,    22,
     104,   123,    51,    28,   145,    51,   107,    12,    48,   115,
     107,     9,   131,   107,   147,   136,   135,   137,   107,   107,
       6,   132,    97,   133,   134,   107,    51,   107,    57,   123,
      51,   135,   107,   133
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
     138,   138,   138,   139,   138,   138,   138,   138,   140,   138,
     141,   141,   142,   142,   143,   143,   143,   143,   143,   144,
     144,   144,   145,   146,   146,   147
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
       1,     0,     4,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     3,     0,     4,     3,     3,     4,     0,     8,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
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
#line 216 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1813 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 3:
#line 218 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1820 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 4:
#line 220 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1827 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 5:
#line 222 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1834 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 6:
#line 224 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1841 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 7:
#line 226 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1848 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 8:
#line 231 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1855 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 9:
#line 233 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1862 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 10:
#line 238 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // still need to close the scope opened by the data-modification statement
      parser->ast()->scopes()->endNested();
    }
#line 1871 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 11:
#line 242 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // the RETURN statement will close the scope opened by the data-modification statement
    }
#line 1879 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 12:
#line 248 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1886 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 13:
#line 250 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1893 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 14:
#line 255 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1900 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 15:
#line 257 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1907 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 16:
#line 259 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1914 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 17:
#line 261 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1921 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 18:
#line 263 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1928 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 19:
#line 265 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1935 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 20:
#line 270 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
     
      auto node = parser->ast()->createNodeFor((yyvsp[-2].strval), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1946 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 21:
#line 276 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
      auto node = parser->ast()->createNodeTraversal((yyvsp[-4].strval), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1956 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 22:
#line 281 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
      auto node = parser->ast()->createNodeTraversal((yyvsp[-6].strval), (yyvsp[-4].strval), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1966 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 23:
#line 286 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_FOR);
      auto node = parser->ast()->createNodeTraversal((yyvsp[-8].strval), (yyvsp[-6].strval), (yyvsp[-4].strval), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1976 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 24:
#line 294 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // operand is a reference. can use it directly
      auto node = parser->ast()->createNodeFilter((yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 1986 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 25:
#line 302 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 1993 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 26:
#line 307 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2000 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 27:
#line 309 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2007 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 28:
#line 314 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeLet((yyvsp[-2].strval), (yyvsp[0].node), true);
      parser->ast()->addOperation(node);
    }
#line 2016 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 29:
#line 321 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! TRI_CaseEqualString((yyvsp[-2].strval), "COUNT")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'COUNT'", (yyvsp[-2].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 2028 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 30:
#line 331 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2037 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 31:
#line 334 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      auto list = static_cast<AstNode*>(parser->popStack());

      if (list == nullptr) {
        ABORT_OOM
      }
      (yyval.node) = list;
    }
#line 2050 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 32:
#line 345 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2071 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 33:
#line 361 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2103 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 34:
#line 388 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2135 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 35:
#line 415 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2172 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 36:
#line 447 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2204 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 37:
#line 477 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2211 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 38:
#line 479 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2218 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 39:
#line 484 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeAssign((yyvsp[-2].strval), (yyvsp[0].node));
      parser->pushArrayElement(node);
    }
#line 2227 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 40:
#line 491 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = nullptr;
    }
#line 2235 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 41:
#line 494 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);
    }
#line 2243 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 42:
#line 500 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2262 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 43:
#line 514 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2281 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 44:
#line 531 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! TRI_CaseEqualString((yyvsp[0].strval), "KEEP")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'KEEP'", (yyvsp[0].strval), yylloc.first_line, yylloc.first_column);
      }

      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2294 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 45:
#line 538 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode*>(parser->popStack());
      (yyval.node) = list;
    }
#line 2303 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 46:
#line 545 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2312 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 47:
#line 548 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode const*>(parser->popStack());
      auto node = parser->ast()->createNodeSort(list);
      parser->ast()->addOperation(node);
    }
#line 2322 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 48:
#line 556 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2330 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 49:
#line 559 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2338 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 50:
#line 565 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeSortElement((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 2346 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 51:
#line 571 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 2354 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 52:
#line 574 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 2362 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 53:
#line 577 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(false);
    }
#line 2370 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 54:
#line 580 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2378 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 55:
#line 586 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto offset = parser->ast()->createNodeValueInt(0);
      auto node = parser->ast()->createNodeLimit(offset, (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2388 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 56:
#line 591 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeLimit((yyvsp[-2].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
    }
#line 2397 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 57:
#line 598 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeReturn((yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->ast()->scopes()->endNested();
    }
#line 2407 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 58:
#line 606 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2415 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 59:
#line 609 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       (yyval.node) = (yyvsp[0].node);
     }
#line 2423 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 60:
#line 615 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REMOVE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }
      auto node = parser->ast()->createNodeRemove((yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2436 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 61:
#line 626 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_INSERT, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }
      auto node = parser->ast()->createNodeInsert((yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2449 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 62:
#line 637 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_UPDATE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeUpdate(nullptr, (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2463 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 63:
#line 646 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_UPDATE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeUpdate((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2477 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 64:
#line 658 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2484 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 65:
#line 663 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REPLACE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeReplace(nullptr, (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2498 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 66:
#line 672 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (! parser->configureWriteQuery(AQL_QUERY_REPLACE, (yyvsp[-1].node), (yyvsp[0].node))) {
        YYABORT;
      }

      AstNode* node = parser->ast()->createNodeReplace((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node));
      parser->ast()->addOperation(node);
      parser->setWriteNode(node);
    }
#line 2512 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 67:
#line 684 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2519 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 68:
#line 689 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = static_cast<int64_t>(NODE_TYPE_UPDATE);
    }
#line 2527 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 69:
#line 692 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = static_cast<int64_t>(NODE_TYPE_REPLACE);
    }
#line 2535 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 70:
#line 698 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      // reserve a variable named "$OLD", we might need it in the update expression
      // and in a later return thing
      parser->pushStack(parser->ast()->createNodeVariable(Variable::NAME_OLD, true));
    }
#line 2545 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 71:
#line 702 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2595 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 72:
#line 750 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2603 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 73:
#line 753 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2611 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 74:
#line 756 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2619 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 75:
#line 759 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2627 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 76:
#line 762 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2635 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 77:
#line 765 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeRange((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2643 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 78:
#line 771 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);

      if ((yyval.strval) == nullptr) {
        ABORT_OOM
      }
    }
#line 2655 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 79:
#line 778 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 2674 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 80:
#line 795 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushStack((yyvsp[0].strval));

      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2685 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 81:
#line 800 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto list = static_cast<AstNode const*>(parser->popStack());
      (yyval.node) = parser->ast()->createNodeFunctionCall(static_cast<char const*>(parser->popStack()), list);
    }
#line 2694 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 82:
#line 807 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_PLUS, (yyvsp[0].node));
    }
#line 2702 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 83:
#line 810 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_MINUS, (yyvsp[0].node));
    }
#line 2710 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 84:
#line 813 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    { 
      (yyval.node) = parser->ast()->createNodeUnaryOperator(NODE_TYPE_OPERATOR_UNARY_NOT, (yyvsp[0].node));
    }
#line 2718 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 85:
#line 819 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_OR, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2726 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 86:
#line 822 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_AND, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2734 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 87:
#line 825 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_PLUS, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2742 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 88:
#line 828 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_MINUS, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2750 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 89:
#line 831 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_TIMES, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2758 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 90:
#line 834 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_DIV, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2766 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 91:
#line 837 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_MOD, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2774 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 92:
#line 840 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_EQ, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2782 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 93:
#line 843 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_NE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2790 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 94:
#line 846 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_LT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2798 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 95:
#line 849 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_GT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2806 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 96:
#line 852 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_LE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2814 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 97:
#line 855 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_GE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2822 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 98:
#line 858 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_IN, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2830 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 99:
#line 861 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_NIN, (yyvsp[-3].node), (yyvsp[0].node));
    }
#line 2838 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 100:
#line 867 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeTernaryOperator((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2846 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 101:
#line 873 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2853 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 102:
#line 875 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2860 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 103:
#line 880 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2868 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 104:
#line 883 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (parser->isModificationQuery()) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected subquery after data-modification operation", yylloc.first_line, yylloc.first_column);
      }
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_SUBQUERY);
      parser->ast()->startSubQuery();
    }
#line 2880 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 105:
#line 889 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      AstNode* node = parser->ast()->endSubQuery();
      parser->ast()->scopes()->endCurrent();

      std::string const variableName = parser->ast()->variables()->nextName();
      auto subQuery = parser->ast()->createNodeLet(variableName.c_str(), node, false);
      parser->ast()->addOperation(subQuery);

      (yyval.node) = parser->ast()->createNodeReference(variableName.c_str());
    }
#line 2895 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 106:
#line 902 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2903 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 107:
#line 905 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2911 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 108:
#line 911 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2919 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 109:
#line 914 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 2927 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 110:
#line 920 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
    }
#line 2936 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 111:
#line 923 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = static_cast<AstNode*>(parser->popStack());
    }
#line 2944 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 112:
#line 929 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2951 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 113:
#line 931 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 2958 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 114:
#line 936 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2966 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 115:
#line 939 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushArrayElement((yyvsp[0].node));
    }
#line 2974 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 116:
#line 945 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 2982 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 117:
#line 948 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[-1].strval) == nullptr || (yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }

      if (! TRI_CaseEqualString((yyvsp[-1].strval), "OPTIONS")) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected qualifier '%s', expecting 'OPTIONS'", (yyvsp[-1].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.node) = (yyvsp[0].node);
    }
#line 2998 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 118:
#line 962 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeObject();
      parser->pushStack(node);
    }
#line 3007 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 119:
#line 965 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = static_cast<AstNode*>(parser->popStack());
    }
#line 3015 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 120:
#line 971 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 3022 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 121:
#line 973 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 3029 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 122:
#line 978 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 3036 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 123:
#line 980 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
    }
#line 3043 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 124:
#line 985 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushObjectElement((yyvsp[-2].strval), (yyvsp[0].node));
    }
#line 3051 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 125:
#line 988 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      parser->pushObjectElement((yyvsp[-3].node), (yyvsp[0].node));
    }
#line 3059 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 126:
#line 991 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3076 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 127:
#line 1006 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = 1;
    }
#line 3084 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 128:
#line 1009 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.intval) = (yyvsp[-1].intval) + 1;
    }
#line 3092 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 129:
#line 1015 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3100 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 130:
#line 1018 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3108 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 131:
#line 1024 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3116 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 132:
#line 1027 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeArrayLimit(nullptr, (yyvsp[0].node));
    }
#line 3124 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 133:
#line 1030 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeArrayLimit((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 3132 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 134:
#line 1036 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = nullptr;
    }
#line 3140 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 135:
#line 1039 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3148 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 136:
#line 1045 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval));
    }
#line 3156 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 137:
#line 1048 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // TODO FIXME check @s
      (yyval.node) = (yyvsp[0].node);
    }
#line 3165 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 138:
#line 1055 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       auto node = static_cast<AstNode*>(parser->peekStack());
       node->addMember((yyvsp[0].node));
     }
#line 3174 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 139:
#line 1059 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
       auto node = static_cast<AstNode*>(parser->peekStack());
       node->addMember((yyvsp[0].node));
     }
#line 3183 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 140:
#line 1066 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      node->addMember((yyvsp[0].node));
      (yyval.node) = parser->ast()->createNodeCollectionList(node);
    }
#line 3193 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 141:
#line 1072 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = parser->ast()->createNodeArray();
      parser->pushStack(node);
      node->addMember((yyvsp[-1].node));
    }
#line 3203 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 142:
#line 1076 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      auto node = static_cast<AstNode*>(parser->popStack());
      (yyval.node) = parser->ast()->createNodeCollectionList(node);
    }
#line 3212 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 143:
#line 1080 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // graph name
      (yyval.node) = (yyvsp[0].node);
    }
#line 3221 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 144:
#line 1084 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      // graph name
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval));
    }
#line 3230 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 145:
#line 1093 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeDirection(2, 1);
    }
#line 3238 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 146:
#line 1096 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeDirection(1, 1);
    }
#line 3246 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 147:
#line 1099 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeDirection(0, 1);
    }
#line 3254 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 148:
#line 1105 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3301 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 149:
#line 1147 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3309 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 150:
#line 1150 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3317 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 151:
#line 1153 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
      
      if ((yyval.node) == nullptr) {
        ABORT_OOM
      }
    }
#line 3329 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 152:
#line 1160 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3344 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 153:
#line 1170 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if (parser->isModificationQuery()) {
        parser->registerParseError(TRI_ERROR_QUERY_PARSE, "unexpected subquery after data-modification operation", yylloc.first_line, yylloc.first_column);
      }
      parser->ast()->scopes()->start(triagens::aql::AQL_SCOPE_SUBQUERY);
      parser->ast()->startSubQuery();
    }
#line 3356 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 154:
#line 1176 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      AstNode* node = parser->ast()->endSubQuery();
      parser->ast()->scopes()->endCurrent();

      std::string const variableName = parser->ast()->variables()->nextName();
      auto subQuery = parser->ast()->createNodeLet(variableName.c_str(), node, false);
      parser->ast()->addOperation(subQuery);

      (yyval.node) = parser->ast()->createNodeReference(variableName.c_str());
    }
#line 3371 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 155:
#line 1186 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3391 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 156:
#line 1201 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3410 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 157:
#line 1215 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3429 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 158:
#line 1229 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3458 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 159:
#line 1252 "arangod/Aql/grammar.y" /* yacc.c:1661  */
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
#line 3488 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 160:
#line 1280 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3496 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 161:
#line 1283 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3504 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 162:
#line 1289 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }
      
      (yyval.node) = (yyvsp[0].node);
    }
#line 3516 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 163:
#line 1296 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].node) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = (yyvsp[0].node);
    }
#line 3528 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 164:
#line 1306 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueString((yyvsp[0].strval)); 
    }
#line 3536 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 165:
#line 1309 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3544 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 166:
#line 1312 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueNull();
    }
#line 3552 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 167:
#line 1315 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(true);
    }
#line 3560 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 168:
#line 1318 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeValueBool(false);
    }
#line 3568 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 169:
#line 1324 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = parser->ast()->createNodeCollection((yyvsp[0].strval), TRI_TRANSACTION_WRITE);
    }
#line 3580 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 170:
#line 1331 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.node) = parser->ast()->createNodeCollection((yyvsp[0].strval), TRI_TRANSACTION_WRITE);
    }
#line 3592 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 171:
#line 1338 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }
      
      if (strlen((yyvsp[0].strval)) < 2 || (yyvsp[0].strval)[0] != '@') {
        parser->registerParseError(TRI_ERROR_QUERY_BIND_PARAMETER_TYPE, TRI_errno_string(TRI_ERROR_QUERY_BIND_PARAMETER_TYPE), (yyvsp[0].strval), yylloc.first_line, yylloc.first_column);
      }

      (yyval.node) = parser->ast()->createNodeParameter((yyvsp[0].strval));
    }
#line 3608 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 172:
#line 1352 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.node) = parser->ast()->createNodeParameter((yyvsp[0].strval));
    }
#line 3616 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 173:
#line 1358 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3628 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 174:
#line 1365 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      if ((yyvsp[0].strval) == nullptr) {
        ABORT_OOM
      }

      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3640 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;

  case 175:
#line 1374 "arangod/Aql/grammar.y" /* yacc.c:1661  */
    {
      (yyval.strval) = (yyvsp[0].strval);
    }
#line 3648 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
    break;


#line 3652 "arangod/Aql/grammar.cpp" /* yacc.c:1661  */
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
