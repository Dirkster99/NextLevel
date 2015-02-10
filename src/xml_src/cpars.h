
#ifndef CPARS_H____283463128571340
#define CPARS_H____283463128571340

#include "cdtd.h"
#include "cstrm.h"

#include "ctag.h"

#include "chartab.h"

#include <stdio.h>
#include <string.h>

//Parser object keeps track of general information such as
//input stream, line and character position of parser etc.
class CPARS
{
private:
	CDTD dtProf; //Store and resolve document type declarations
	CSTRM xsInp;   //input stream of XML tags

	//Skip over an XML comment <!-- .... -->
	//assuming that '<!--' was already consumed on input stream
	//
	CTAG *SkipComment();
	CTAG *SkipInclude();

	//Handle !DOCTYPE Tags seperately !!!
	CTAG *ProcessDocType(int & iError);
	CTAG *ResolveDocType(const int iType, int & iError);

	//Last character red after the current item
	//This is set if we saw a '<' tag opener to be processed with next call
	UCHAR cLast;
	bool bClean;

public:
	CPARS(char *pcName=NULL);
	~CPARS();

	//Read a raw XML tag and return it in a 'structure'
	CTAG *getNextRawTag();
	const UCHAR *getError(int iError);  //get Error String for above function

	//Access XML parser through these members
	//
	int eof()                  {return xsInp.eof();    }
	int getLine()              {return xsInp.getLine();}
	int getCharPos()           {return xsInp.getCharPos();}
	int open(const char *fName){return xsInp.open(fName); }

	//Create a tag object from a string and a typ descriptor
	//and resolve DTD reference hidden from user of class
	CTAG *CreateTag(const UCHAR *pcTag=NULL, ETAGTYP InTagTyp = TGP_UNKNOWN);

	 //Throw an error message object (Err) with sMess and StdErr
	//including line number and file src of error
	void throwErr(const CUSTR & sMessIn="", int iStdErrNo=ERR_UNKNOWN)
	{
		xsInp.throwErr(sMessIn,iStdErrNo);
	}
};

#endif
