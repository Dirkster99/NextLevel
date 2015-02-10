
// CUSTR Implementation of an UnSigned String Class V1.0
//
/////////////////////////////////////////////////////////////////////
#ifndef CUSTR_H____283463128571340
#define CUSTR_H____283463128571340


#include <iostream>

using namespace std;

#ifndef _UCHAR
	#define _UCHAR
typedef unsigned char UCHAR;
#endif

#ifndef _UINT
	#define _UINT
typedef  unsigned int UINT;
#endif

#ifndef false
	#define false 0
#endif

#ifndef true
	#define true 1
#endif

//Character definition for Operating System and web directory delimiter
#ifndef WIN32
	#define DIR_DELIMITER '/'
#else
	#define DIR_DELIMITER '\\'
#endif

#define WEB_DIR_DEL '/'

class CUSTR
{
private:
	UCHAR *pcStr;
	UINT iStrSz;

	void Reset(); //Reset object state at construction time (called from constructors)
	void AllocString(const UCHAR *pc);
public:

	CUSTR();          //String Class Constructors
	CUSTR(const UCHAR *pc);
	CUSTR(char *pc);
	CUSTR(const char *pc);
	CUSTR(UINT iCntChars);
	CUSTR(const CUSTR &s);  //Copy constructor

	~CUSTR();

	int operator==(const UCHAR *s) const; //Equality operator
	int operator==(const char  *s) const;
	int operator==(const CUSTR & s) const;

	int operator!=(const UCHAR *s) const; //InEquality operator
	int operator!=(const char  *s) const;
	int operator!=(const CUSTR & s) const;

	int operator>(const UCHAR *s) const; //Greater than inEquality operator
	int operator>(const char  *s) const;
	int operator>(const CUSTR & s) const;

	int operator<(const UCHAR *s) const; //Less than inEquality operator
	int operator<(const char  *s) const;
	int operator<(const CUSTR & s) const;

	//Copy strings from simple null terminated memory representations
	CUSTR & operator=(const UCHAR * right);
	CUSTR & operator=(const char  * right);
	CUSTR & operator=(const CUSTR & right); //Overwrite standard operator
	CUSTR & operator=(const CUSTR *pusPtr);

	//Cut of part of the string and return remainder
	CUSTR Mid(int idxBegin, int idxEnd) const;
	CUSTR Mid(int idxBegin) const;

	int Left(UCHAR c) const;
	int Right(UCHAR c) const;

	CUSTR & replaceAt(int iBeg, int iLen, UCHAR c);   //insert new character as replacement
	CUSTR & replaceAt(int iBeg, int iLen, const UCHAR *pc);

	//Replace the occurrence of c with cNewChar in all spots of the string
	CUSTR & Replace(const UCHAR cNewChar, const UCHAR c);

	//Split string into left and right remainder
	CUSTR & Split(int idxSplit, CUSTR & s);
	CUSTR & Split(int iBeg, int iEnd, CUSTR & s);

	//Convert string contents to lower or upper case
	CUSTR LowerCase();
	CUSTR UpperCase();

	//Convert int to string
	static int i2a(int iVal, UCHAR *pcRet=NULL, int iSz=0);

	void operator+=(const UCHAR *pcRight);
	void operator+=(const char *pcRight);
	void operator+=(const CUSTR & right);
	void operator+=(const char right);
	void operator+=(const UCHAR right);
	void operator+=(const int iVal);

	CUSTR operator+(const CUSTR & str) const;
	CUSTR operator+(const int iVal) const;

	//Cut string by this many character from the right side
	void operator-=(const int iShort);
	int slen() const;

	// Compare two strings case-insensitive
	int scmp_ci( const UCHAR *s1) const;

	//Concatinate two strings and copy iSz characters
	void scat_n(UINT iSz, CUSTR & sRight);
	
	//String index [] operator returns one character at a time
	//
	UCHAR operator[](int index) const;

	void SetChar(int i, UCHAR c);

	//Expose the contents of the string as const uchar
	//beginning at the n. position
	const UCHAR *SubString(int i) const;

	//Find the occurrence of a substring within a given string
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	int FindSubString(const char *pcSubStr) const;
	int FindChar(const UCHAR c, int i) const;
	int FindChar(const UCHAR c) const;
	int FindCharFromRight(const UCHAR c, int i) const;
	int FindCharFromRight(const UCHAR c) const;

	operator const UCHAR *() const { return pcStr;              }
	operator const char *()  const { return (char *)this->pcStr;}

	//operator const UCHAR *[](int idx) const{return &pcStr[idx];}
	void Trim(); //Trim current string on left and right side from whitespaces
	void TrimAllWhiteSpaces();

	void print(FILE *fp) const; //Print contents into stream

	friend istream & operator>>(istream & is, CUSTR & s); //Declare input stream operator
	friend ostream & operator<<(ostream & is, CUSTR & s); //Declare output stream operator
};

/*
// File string related functions
char *sGetFileName(char *pcVocIndex);
char *sGetPath(char *pcVocIndex);

#ifndef WIN32
	#define DIR_DELIMITER '/'
#else
	#define DIR_DELIMITER '\\'
#endif

// strcmp for unsigned char
int scmp( const UCHAR *s1, const UCHAR *s2 );

// String Compare with inclusive case fold
int scmp_c( const UCHAR *s1, const UCHAR *s2 );

//Trim off leading and ending control sequences
UCHAR  *sTrim(UCHAR *pcIn);
UCHAR *stTrim(UCHAR *pcIn);

UCHAR *schr(UCHAR *pcStr, const UCHAR cLetter);

UCHAR *schcat(UCHAR *pcStr, const UCHAR cLetter);
UCHAR *scat(UCHAR *pcStr, UCHAR *pcStr1);

UCHAR *schl(UCHAR *pcStrIn, const UCHAR cLetter);

UCHAR *sGetLineFromFile( UCHAR **pcBuf, int *iBufSize, int iBlockLen, FILE *fp);

UCHAR *sToLower(UCHAR *pcWord); //Case fold including extended ASCII set
UCHAR *sToUpper(UCHAR *pc);

UCHAR cToLower(UCHAR c);
UCHAR cToUpper(UCHAR c);

// Sort HTML Codes by ASCII valua
void SortASCII2HTML();

// Expand ASCII into HTML Strings
// 1>  Call SortASCII2HTML first
// 2>  Caller must free returned string
UCHAR *ExpandString2HTML(UCHAR *pcIn);

// FAST case-folding
#define CTOLOWER(c)                                  \
	if(c >= (UCHAR)'A' && c <= (UCHAR)'Z')             \
{                                                    \
		c = c + ((UCHAR)'a' - (UCHAR)'A');               \
	}                                                  \
	else                                               \
	{                                                  \
		if(c >= (UCHAR)'À' && c <= (UCHAR)'Ö')           \
		{                                                \
			c = c + ((UCHAR)'à' - (UCHAR)'À');             \
		}                                                \
		else                                             \
		{                                                \
			                                               \
			if(c >= (UCHAR)'Ù' && c <= (UCHAR)'Þ')         \
			{                                              \
				c = c + ((UCHAR)'ù' - (UCHAR)'Ù');           \
			}                                              \
		}                                                \
	}

#define CTOUPPER(c)                                  \
	if(c >= (UCHAR)'a' && c <= (UCHAR)'z')             \
	{                                                \
		c = c - ((UCHAR)'a' - (UCHAR)'A');               \
	}                                                  \
	else                                               \
	{                                                  \
		if(c >= (UCHAR)'à' && c <= (UCHAR)'ö')           \
		{                                                \
			c = c - ((UCHAR)'à' - (UCHAR)'À');             \
		}                                                \
		else                                             \
		{                                                \
			if(c >= (UCHAR)'ù' && c <= (UCHAR)'Þ')         \
			{                                              \
				c = c - ((UCHAR)'ù' - (UCHAR)'þ');           \
			}                                              \
		}                                                \
	}
*/

namespace s
{
	// Look for a character starting from right side
	UCHAR *schr(UCHAR *pcStrIn, UCHAR cLetter);

	int slen(const UCHAR *s1); // standard slen function

	// Compare two strings case-sensitive
	int scmp( const UCHAR *s1, const UCHAR *s2 );

	// Compare two strings case-insensitive
	int scmp_ci( const UCHAR *s1, const UCHAR *s2 );

	// This function assumes that s1,s2 and s3,s4 each are a pair of strings
  // The goal is to compare both pairs and return a valid result that is useful
  // comparison in sorting functions
  // !!! We priorize comparing s1,s3 over s2,s4 !!!
  // ================================================================================<<<
  int scmp_ci2(const UCHAR *s1, const UCHAR *s2, const UCHAR *s3, const UCHAR *s4);

	// Compare two strings case-insensitive (explicit cast for char version)
	inline int scmp_ci_uc( const UCHAR *s1, const char *s2 )
	{
		return scmp_ci(s1, (const UCHAR *)s2);
	}

	// Compare two strings case-insensitive (explicit cast for char version)
	inline int scmp_ci_cu( const char *s1, const UCHAR *s2 )
	{
		return scmp_ci((const UCHAR *)s1, s2);
	}

	// Compare two strings case-insensitive (explicit cast for char version)
	inline int scmp_ci_cc( const char *s1, const char *s2 )
	{
		return scmp_ci((const UCHAR *)s1, (const UCHAR *)s2);
	}

	UCHAR *sToLower(UCHAR *pc); //Case folding beyond plain English letters :)
	UCHAR cToLower(UCHAR c);
	UCHAR *sToUpper(UCHAR *pc);
	UCHAR cToUpper(UCHAR c);

	// Compare at most n characters from two strings
	int scmp_n( const UCHAR *s1, const UCHAR *s2, int n);

	UCHAR *scpy( UCHAR *s1, const UCHAR *s2 );
	UCHAR *sdup(const UCHAR *s);
	UCHAR *sdupn(const UCHAR *s, const int iLen);

	char *sdup(const char *s);
	char *sdupn(const char *s, const int iLen);
	int slen(const char *s1); // standard slen function

	//Get the path from a filepath string
	CUSTR sGetPath(const CUSTR & sVocIndex);

	// <=================================================================>
	// Convert sRelP to absolut path (in relation to current working dir)
	// <=================================================================>
	CUSTR GetAbsPath(const CUSTR & sCurrAbsDir, const CUSTR & sRelP);

	// Find the directory delimiter character and return remaining string
	CUSTR sGetFileName(const CUSTR & pcVocIndex);

	//Get relative path between two directories
	CUSTR GetRelPath(const CUSTR & sOutPutDir, const CUSTR & sPath);

	//Create a filename and path from a bunch of strings
	CUSTR CreateFileName(const char *pcFPath, const char *pcFName, const char *pcExt);

	//Get current working directory wrapped in a string
	CUSTR sGetCWD();

	//Trim off leading and ending control sequences
	//Use this only with dynamically allocated strings, because we will
	//free the current string and return a new copy, if required
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>°°°°°°°°°
	UCHAR *sTrim(UCHAR *pcIn);
	
	//Trim off leading, ending, and inside string control sequences
	//Use this if you want a string that does not contain any whitespaces
	UCHAR *sTrimAllWhiteSpaces(UCHAR *pcIn);
}

#endif
