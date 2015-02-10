
// Err.cpp: implementation of the Err class. V1.0
//
//////////////////////////////////////////////////////////////////////

// CERR.h: interface for the Error class, which is used to throw and catch Errors
//
// ------------------------------------------------------------------------------>

#ifndef ERR_H____283463128571340
#define ERR_H____283463128571340

#include "custr.h"

#define ERR_NONE          0 //There was no error at all
#define ERR_SYNTAX        1 //Syntax error detected
#define ERR_BUFFER_OVFL   2 //Item is larger than Read Buffer :(
#define ERR_UNKNOWN       3 //Unknown error occured !???
#define ERR_UNEXP_EOF     4 //Unexpected end of file
#define ERR_FILEOPEN      5 //Could not open file
#define ERR_OUTOFMEM      6 //Ran out of memory
#define ERR_IDX_OOF_RANGE 7 //Caller supplied invalid index
#define ERR_INTERNAL      8 //This would be an algorithmic problem

class CERR  
{
public:
	CERR(const CUSTR & sMessIn="", const int iStdErrNo=ERR_UNKNOWN, const CUSTR & sSrcIn="", const CUSTR & sPosIn="" );

	void printfMsg(FILE *fp = stdout);

	//Get Error description and number through these interfaces
	const UCHAR *getStdErrMsg();
	const CUSTR getsErrMsg();
	const int   getStdErrNo();

	const CUSTR getsErrSrc();
	const CUSTR getsErrPos();

	CUSTR getMsg(); //Get a formated error message for printing...
private:
	const UCHAR *pcErr;
	CUSTR sMess, sSrc, sPos;
	int   iErrNo;
};

//return human readable string for error code
const UCHAR *getError(const int iError);

#endif
