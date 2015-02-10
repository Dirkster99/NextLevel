
// Err.cpp: implementation of the Err class. V1.0
//
//////////////////////////////////////////////////////////////////////

// CERR.h: interface for Error class, which is used to throw and catch Errors
//
// -------------------------------------------------------------------------- >

#ifndef OPTS_H____283463128571340
#define OPTS_H____283463128571340

#include "custr.h"
#include "cscol.h"
#include "cerr.h"

//Type of output (ASCII, HTML ...)
typedef enum {eOUT_HTML=1, eOUT_ASCII=2} enOUT_TYPE;

class COpts
{
public:
	COpts();
	~COpts();

	COpts(COpts & optIn)
	{
		throw CERR("Copy constructor for COpts NOT implemented, yet!!!\n");
	}

	COpts & operator=(const COpts *pOpt)
	{
		throw CERR("Equal operator for COpts NOT implemented, yet!!!\n");
	}

	COpts & operator=(const COpts & optObj)
	{
		throw CERR("Equal operator for COpts NOT implemented, yet!!!\n");
	}

	CSHCOL hKeyWords; //String collection of SQL keywords

	CUSTR sVers;
	  int iBuild;
	bool  bGenWinHelp;  //Generate WinHelp Header files (if set to true)
};

//return human readable string for error code
const UCHAR *getError(const int iError);

#endif
