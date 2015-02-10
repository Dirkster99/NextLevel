//
//Character tables and basic definitions
//
//
//String Matching functions and TRIE implementation Version 1.1
//

#ifndef CHAR_TAB_H____283463128571340
#define CHAR_TAB_H____283463128571340

#include "custr.h"

#ifndef _UINT
#define _UINT
	typedef unsigned int UINT;
#endif

#define LF_CHAR    0xA //Line Feed
#define ENTER_CHAR 0xD //Define enter character

#define SPACE_CHAR 32 //Define code for space ' '
#define RAUTE_CHAR 35 //Define code '#' character
#define AMPERCENT  38 //Define the code for the ampercent character '&'

#define BG_TAG_CHAR 60  //Define code for begin of tag symbol '<'
#define EO_TAG_CHAR 62  //Define code for end of tag symbol '>'
#define QMARK_CHAR  63  //Define code for question mark symbol '?'

#define EO_REF     59  //Define terminator symbol for reference codes ';'
#define A_CHAR     65  //Define code for upper case A
#define Z_CHAR     90  //Define code for upper case Z

#define X_CHAR     120 //Define code 'x' character

#define Q_BRCK_OP   91 //Code for quadratic opening bracket [
#define Q_BRCK_CL   93 //Code for quadratic closing bracket ]

//
//define character sequences as a sorted array of intervals
//this interval is then used to efficiently determine the 
//membership of a character c
//
typedef struct intval
{
	UCHAR cLowLimit, cUpLimit;

} INTVAL;

//Here is the function that gets an array of intervals and a chacter
//and determines whether that character was part of an interval or not
//
//Take a value and an array of intervals and return true
//if the value was part of an interval, otherwise false
bool isOutOfRange(const UCHAR & c, int iIntValsSz, const INTVAL *pivVals);

int FindKeyWord(const UCHAR *pcMask, const UCHAR *pcStr, int iKeyCnt, int iKeyLen,
                int & idx, int iIntValsSz, const INTVAL *pivVals);

#define MST_MISMATCH -1
#define MST_MATCH    0

class MSTATE
{
public:
	int iPos;  //Currently matching at this character position [0...m]
	int iWrd; //and at this word [0...n]

	MSTATE():iPos(0),iWrd(1){}
};

#define REFERNCESZ  7
#define REFERNCECNT 2
const UCHAR REFERNCES[] = "PUBLIC SYSTEM";

//Match/Mismatch a keyword from a list of keywords indicated by mask
//return -1 on mismatch
//return 0  on match for this letter
//return Index [1...n] of keyword on match for whole string
//
int FindKeyWord(const UCHAR *pcMask, const UCHAR pc, const int iKeyCnt, const int iKeyLen,
                MSTATE & State);

//Intervals for white spaces
#define WHTESPSZ 3
const INTVAL WHTESP[WHTESPSZ] = { {0x09,0x0A}, {0x0D,0x0D}, {0x20,0x20} };

//Define macros to determine white and non-white spaces
//Evaluate to true if c is NOT a white character
#define NOWHITE(c) (isOutOfRange(c, WHTESPSZ-1, WHTESP)==true)

//Evaluate to true if c IS a white character
#define ISWHITE(c) (isOutOfRange(c, WHTESPSZ-1, WHTESP)!=true)

//Interval for upper case letter
#define UPLETTRSZ 1
const INTVAL UPLETTR[UPLETTRSZ] = { {'A','Z'}};

//match Name and value strings and return position in string
int MatchName(const UCHAR *pcStr, bool SkipWhSpace, int & idxBeg);
int MatchVal(const UCHAR *pcStr, bool SkipWhSpace, int & idxBeg);

//Name entity has two parts one for first letter and one for all others
//
#define NAME1SZ 6
const INTVAL NAME1[NAME1SZ] =
{ {0x41,0x5A}, {0x5F,0x5F}, {0x61,0x7A}, {0xC0,0xD6}, {0xD8,0xF6}, {0xF8,0xFF}};

#define NAME2SZ 9
const INTVAL NAME2[NAME2SZ] =
{
	{0x2D,0x2E}, {0x30,0x3A}, {0x41,0x5A}, {0x5F,0x5F},
	{0x61,0x7A}, {0xB7,0xB7}, {0xC0,0xD6}, {0xD8,0xF6}, {0xF8,0xFF}
};

//This is what a leaf of the trie looks like
//There is a pcRepl code for each pcCode
typedef struct tleaf
{
	UCHAR *pcCode; //This is a code such as Auml
	UCHAR *pcRepl; //And this is a pointer to a replacement code

}TLEAF;

//This is a trie node
//Its a collection of hash codes, leaf nodes, and next pointers
//to deeper (child) nodes
typedef struct tnode
{
	int iSz, iActSz;        //Used Size and Actual Size of arrays in this node
	         UCHAR   *pHash; //Array of hash codes for this level
	        TLEAF **ppLefs; //Leafs for this node
	struct  tnode **ppNext; //point to deeper hash nodes

}TNODE;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//this structure is used to initialize a parse trie from a table
//of constant strings, in XML (for example), entities such as
//&gt; and &lt; are used as standard entities, we store such
//entities and their replacement code in the structure below
//and use an array of these constants to initialize the TRIE ...
typedef struct hcode
{
	UCHAR *pcCode;
	UCHAR *pcRepl;

}HCODE;

//Call this to initialize code translation from a table of constants
///////////////////////////////////////////////////////////////////////
TNODE *trInitFromTable(const HCODE *pHCodes, UINT iSz, int *piMaxSz, UINT & iCnt);

// Add more entries ...
bool trAddEntry(const UCHAR *pc, const UCHAR *pcRepl, TNODE **ppTrRoot, int *piMaxSz);

//Find a string and its translation code in TRIE
//pc Points to the location of the string and CodeLen
//shows the length of the string that we need to find in the trie
//The string returned (if any) is the replacement string for pc
const UCHAR *trFindString(const UCHAR *pc, TNODE *pTRoot, int iCodeLen=-1);

//Destroy the TRIE here ...
void trFreeTree(TNODE *pTrie);

void trDebugTree(TNODE *pTrie, int iLevel=0);

int CompareTries(TNODE *trRoot, TNODE *trRoot1, TNODE **pTDeb, int iLev=0);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//Convert a number suach as: &#223; (Decimal encoded)
//                       or: &#x34; (Hexadecimal encoded)
typedef enum PC_TYPE { PCT_HEX=1, PCT_DEC=2};

//Convert a UCHAR character string into an actual UCHAR integer value
//The string can represent a Decimal or Hex number
//
//<<<<< Translate a string such as &#223; into an 8-bit value >>>>>
//<<<<< This Function is limited to eight bit unsigned integer>>>>>
//<<<<< Function will return zero on error, so zero CANNOT be converted!!!
//<<<<< The function assumes no leading zeros
//////////////////////////////////////////////////////////////////////////
UINT pc2i(const UCHAR *pcIn, PC_TYPE ptType, int iLen);

#endif

/*****

A comment from the web design group (wdg) at http://www.htmlhelp.com/reference/

ISO-8859-1 explicitly does not define displayable characters
for positions 0-31 and 127-159, and the HTML standard does not
allow those to be used for displayable characters. The only
characters in this range that are used are 9, 10 and 13, which
are tab, newline and carriage return respectively. If you attempt
to display these invalid characters on your own system, you may
find some characters displayed there, but please do not assume
that other users will see the same thing (or even anything at all)
on their systems.

-------------------------------------------------------------

 ASCII coding table taken from man page Red Hat 7.2 and tested
 succesfully on Windows 95

 Oct   Dec   Hex   Char           Oct   Dec   Hex   Char
 ------------------------------------------------------------
 000   0     00    NUL '\0'       100   64    40    @
 001   1     01    SOH            101   65    41    A
 002   2     02    STX            102   66    42    B
 003   3     03    ETX            103   67    43    C
 004   4     04    EOT            104   68    44    D
 005   5     05    ENQ            105   69    45    E
 006   6     06    ACK            106   70    46    F
 007   7     07    BEL '\a'       107   71    47    G
 010   8     08    BS  '\b'       110   72    48    H
 011   9     09    HT  '\t'       111   73    49    I
 012   10    0A    LF  '\n'       112   74    4A    J
 013   11    0B    VT  '\v'       113   75    4B    K
 014   12    0C    FF  '\f'       114   76    4C    L
 015   13    0D    CR  '\r'       115   77    4D    M
 016   14    0E    SO             116   78    4E    N
 017   15    0F    SI             117   79    4F    O
 020   16    10    DLE            120   80    50    P
 021   17    11    DC1            121   81    51    Q
 022   18    12    DC2            122   82    52    R
 023   19    13    DC3            123   83    53    S
 024   20    14    DC4            124   84    54    T
 025   21    15    NAK            125   85    55    U
 026   22    16    SYN            126   86    56    V
 027   23    17    ETB            127   87    57    W
 030   24    18    CAN            130   88    58    X
 031   25    19    EM             131   89    59    Y
 032   26    1A    SUB            132   90    5A    Z
 033   27    1B    ESC            133   91    5B    [
 034   28    1C    FS             134   92    5C    \   '\\'
 035   29    1D    GS             135   93    5D    ]
 036   30    1E    RS             136   94    5E    ^
 037   31    1F    US             137   95    5F    _
 040   32    20    SPACE          140   96    60    `
 041   33    21    !              141   97    61    a
 042   34    22    "              142   98    62    b
 043   35    23    #              143   99    63    c
 044   36    24    $              144   100   64    d
 045   37    25    %              145   101   65    e
 046   38    26    &              146   102   66    f
 047   39    27    '              147   103   67    g
 050   40    28    (              150   104   68    h
 051   41    29    )              151   105   69    i
 052   42    2A    *              152   106   6A    j
 053   43    2B    +              153   107   6B    k
 054   44    2C    ,              154   108   6C    l
 055   45    2D    -              155   109   6D    m
 056   46    2E    .              156   110   6E    n
 057   47    2F    /              157   111   6F    o
 060   48    30    0              160   112   70    p
 061   49    31    1              161   113   71    q
 062   50    32    2              162   114   72    r
 063   51    33    3              163   115   73    s
 064   52    34    4              164   116   74    t
 065   53    35    5              165   117   75    u
 066   54    36    6              166   118   76    v
 067   55    37    7              167   119   77    w
 070   56    38    8              170   120   78    x
 071   57    39    9              171   121   79    y
 072   58    3A    :              172   122   7A    z
 073   59    3B    ;              173   123   7B    {
 074   60    3C    <              174   124   7C    |
 075   61    3D    =              175   125   7D    }
 076   62    3E    >              176   126   7E    ~
 077   63    3F    ?              177   127   7F    DEL : 


 Oct   Dec   Hex   Char   Description
 --------------------------------------------------------------------
 240   160   A0           NO-BREAK SPACE
 241   161   A1          INVERTED EXCLAMATION MARK
 242   162   A2          CENT SIGN
 243   163   A3          POUND SIGN
 244   164   A4          CURRENCY SIGN
 245   165   A5          YEN SIGN
 246   166   A6          BROKEN BAR
 247   167   A7          SECTION SIGN
 250   168   A8          DIAERESIS
 251   169   A9          COPYRIGHT SIGN
 252   170   AA          FEMININE ORDINAL INDICATOR
 253   171   AB          LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
 254   172   AC          NOT SIGN
 255   173   AD          SOFT HYPHEN
 256   174   AE          REGISTERED SIGN
 257   175   AF          MACRON
 260   176   B0          DEGREE SIGN
 261   177   B1          PLUS-MINUS SIGN
 262   178   B2          SUPERSCRIPT TWO
 263   179   B3          SUPERSCRIPT THREE
 264   180   B4          ACUTE ACCENT
 265   181   B5          MICRO SIGN
 266   182   B6          PILCROW SIGN
 267   183   B7          MIDDLE DOT
 270   184   B8          CEDILLA
 271   185   B9          SUPERSCRIPT ONE
 272   186   BA          MASCULINE ORDINAL INDICATOR
 273   187   BB          RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
 274   188   BC          VULGAR FRACTION ONE QUARTER
 275   189   BD          VULGAR FRACTION ONE HALF
 276   190   BE          VULGAR FRACTION THREE QUARTERS
 277   191   BF          INVERTED QUESTION MARK
 300   192   C0     �    LATIN CAPITAL LETTER A WITH GRAVE
 301   193   C1     �    LATIN CAPITAL LETTER A WITH ACUTE
 302   194   C2     �    LATIN CAPITAL LETTER A WITH CIRCUMFLEX
 303   195   C3     �    LATIN CAPITAL LETTER A WITH TILDE
 303   195   C3     �    LATIN CAPITAL LETTER A WITH TILDE
 304   196   C4     �    LATIN CAPITAL LETTER A WITH DIAERESIS
 305   197   C5     �    LATIN CAPITAL LETTER A WITH RING ABOVE
 306   198   C6     �    LATIN CAPITAL LETTER AE
 307   199   C7     �    LATIN CAPITAL LETTER C WITH CEDILLA
 310   200   C8     �    LATIN CAPITAL LETTER E WITH GRAVE
 311   201   C9     �    LATIN CAPITAL LETTER E WITH ACUTE
 312   202   CA     �    LATIN CAPITAL LETTER E WITH CIRCUMFLEX
 313   203   CB     �    LATIN CAPITAL LETTER E WITH DIAERESIS
 314   204   CC     �    LATIN CAPITAL LETTER I WITH GRAVE
 315   205   CD     �    LATIN CAPITAL LETTER I WITH ACUTE
 316   206   CE     �    LATIN CAPITAL LETTER I WITH CIRCUMFLEX
 317   207   CF     �    LATIN CAPITAL LETTER I WITH DIAERESIS
 320   208   D0     �    LATIN CAPITAL LETTER ETH
 321   209   D1     �    LATIN CAPITAL LETTER N WITH TILDE
 322   210   D2     �    LATIN CAPITAL LETTER O WITH GRAVE
 323   211   D3     �    LATIN CAPITAL LETTER O WITH ACUTE
 324   212   D4     �    LATIN CAPITAL LETTER O WITH CIRCUMFLEX
 325   213   D5     �    LATIN CAPITAL LETTER O WITH TILDE
 326   214   D6     �    LATIN CAPITAL LETTER O WITH DIAERESIS
 327   215   D7     �    MULTIPLICATION SIGN
 330   216   D8     �    LATIN CAPITAL LETTER O WITH STROKE
 331   217   D9     �    LATIN CAPITAL LETTER U WITH GRAVE
 332   218   DA     �    LATIN CAPITAL LETTER U WITH ACUTE
 333   219   DB     �    LATIN CAPITAL LETTER U WITH CIRCUMFLEX
 334   220   DC     �    LATIN CAPITAL LETTER U WITH DIAERESIS
 335   221   DD     �    LATIN CAPITAL LETTER Y WITH ACUTE
 336   222   DE     �    LATIN CAPITAL LETTER THORN
 337   223   DF     �    LATIN SMALL LETTER SHARP S
 340   224   E0     �    LATIN SMALL LETTER A WITH GRAVE
 341   225   E1     �    LATIN SMALL LETTER A WITH ACUTE
 342   226   E2     �    LATIN SMALL LETTER A WITH CIRCUMFLEX
 343   227   E3     �    LATIN SMALL LETTER A WITH TILDE
 344   228   E4     �    LATIN SMALL LETTER A WITH DIAERESIS
 345   229   E5     �    LATIN SMALL LETTER A WITH RING ABOVE
 346   230   E6     �    LATIN SMALL LETTER AE
 347   231   E7     �    LATIN SMALL LETTER C WITH CEDILLA
 350   232   E8     �    LATIN SMALL LETTER E WITH GRAVE
 351   233   E9     �    LATIN SMALL LETTER E WITH ACUTE
 352   234   EA     �    LATIN SMALL LETTER E WITH CIRCUMFLEX
 353   235   EB     �    LATIN SMALL LETTER E WITH DIAERESIS
 354   236   EC     �    LATIN SMALL LETTER I WITH GRAVE
 355   237   ED     �    LATIN SMALL LETTER I WITH ACUTE
 356   238   EE     �    LATIN SMALL LETTER I WITH CIRCUMFLEX
 357   239   EF     �    LATIN SMALL LETTER I WITH DIAERESIS
 360   240   F0     �    LATIN SMALL LETTER ETH
 361   241   F1     �    LATIN SMALL LETTER N WITH TILDE
 362   242   F2     �    LATIN SMALL LETTER O WITH GRAVE
 363   243   F3     �    LATIN SMALL LETTER O WITH ACUTE
 364   244   F4     �    LATIN SMALL LETTER O WITH CIRCUMFLEX
 365   245   F5     �    LATIN SMALL LETTER O WITH TILDE
 366   246   F6     �    LATIN SMALL LETTER O WITH DIAERESIS
 367   247   F7     �    DIVISION SIGN
 370   248   F8          LATIN SMALL LETTER O WITH STROKE
 371   249   F9          LATIN SMALL LETTER U WITH GRAVE
 372   250   FA          LATIN SMALL LETTER U WITH ACUTE
 373   251   FB          LATIN SMALL LETTER U WITH CIRCUMFLEX
 374   252   FC          LATIN SMALL LETTER U WITH DIAERESIS
 375   253   FD          LATIN SMALL LETTER Y WITH ACUTE
 376   254   FE          LATIN SMALL LETTER THORN
 377   255   FF          LATIN SMALL LETTER Y WITH DIAERESIS

*/

/*
 Dec   Hex   Char   Description (German letter subset)
 --------------------------------------------------------------------
 223   DF     �    LATIN SMALL LETTER SHARP S
 196   C4     �    LATIN CAPITAL LETTER A WITH DIAERESIS
 203   CB     �    LATIN CAPITAL LETTER E WITH DIAERESIS
 220   DC     �    LATIN CAPITAL LETTER U WITH DIAERESIS
 214   D6     �    LATIN CAPITAL LETTER O WITH DIAERESIS

 228   E4     �    LATIN SMALL LETTER A WITH DIAERESIS
 235   EB     �    LATIN SMALL LETTER E WITH DIAERESIS
 252   FC          LATIN SMALL LETTER U WITH DIAERESIS
 246   F6     �    LATIN SMALL LETTER O WITH DIAERESIS
*/

/*
 Dec   Hex   Char   Description (Spanish letter subset)
 --------------------------------------------------------------------
 193   C1     �    LATIN CAPITAL LETTER A WITH ACUTE
 201   C9     �    LATIN CAPITAL LETTER E WITH ACUTE
 205   CD     �    LATIN CAPITAL LETTER I WITH ACUTE
 209   D1     �    LATIN CAPITAL LETTER N WITH TILDE
 211   D3     �    LATIN CAPITAL LETTER O WITH ACUTE
 218   DA     �    LATIN CAPITAL LETTER U WITH ACUTE

 225   E1     �    LATIN SMALL LETTER A WITH ACUTE
 233   E9     �    LATIN SMALL LETTER E WITH ACUTE
 237   ED     �    LATIN SMALL LETTER I WITH ACUTE
 241   F1     �    LATIN SMALL LETTER N WITH TILDE
 243   F3     �    LATIN SMALL LETTER O WITH ACUTE
 250   FA          LATIN SMALL LETTER U WITH ACUTE
*/
