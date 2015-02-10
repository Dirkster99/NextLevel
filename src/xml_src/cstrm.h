
// CSTRM Implementation of a Stream Class V1.0
//
/////////////////////////////////////////////////////////////////////
#ifndef CSTRM_H____283463128571340
#define CSTRM_H____283463128571340

#include "custr.h"
#include "cerr.h"

#include <malloc.h>
#include <string.h>

#include <stdio.h>

#include <fstream>
#include <iostream>

using namespace std;

//A CSTRM class wrapes around the standard file input class
//and does additional work in terms of counting lines and cursor
//positions and generating an error message for the current input
//position. The class implements also an input buffer that is
//accessible from outside
//
class CSTRM
{
private:
	CUSTR sPath, sFName; //Keep file name and path for this file

	//Store character and line position of parser for syntax error message
	int iLine, iChar;

	ifstream fIn; //input stream for parser

	int iBufSz; //Size of raw input buffer

	void GetFPath(const char *pcfinName);
	bool bFreeBuf;
	void Init(const char *pcfinName, UCHAR *pcBufIn, int iBufSzIn);

public:
	CSTRM(char *pfinName = NULL, UCHAR *pcBufIn = NULL, int iBufSzIn=0);
	~CSTRM();

	//Open a file and return line, char parser
	//Return !=0 if everything's fine
	//
	//Open and try with path if FName is relative
	int open(const char *pcfinName, const CUSTR & sPath="",UCHAR *pcBuf=NULL, int iSz=0);

	//File input stream functions
	UCHAR get();
	void get(UCHAR & c);

	UCHAR *pcBuf; //Read buffer for raw input
	int igetBufSz() const {return iBufSz; }

	//Reached the end of file yet???
	int eof() const {return fIn.eof();}

	//Get line and character position of parser for debugging
	//
	int getLine() const { return iLine; }
	int getCharPos() const { return iChar; }
	void getLineCharPos(int & iLine, int & iChar) const;

	//Read a name entity from stream and return buffer
	//First letter of name is in cRd
	int ReadName(UCHAR & cRd);

	//Read a SYSTEM literal delimited by " ... " or ' ... '
	int ReadLiteral(UCHAR & cRd);

	const CUSTR getPath() const  {return sPath;}
	const CUSTR getFName() const {return sFName;}

	//Throw an error message object (Err) with sMess and StdErr
	//including line number and file src of error
	//
	void throwErr(const CUSTR & sMessIn="", int iStdErrNo=ERR_UNKNOWN);

	//Skip over an XML comment <!-- .... -->
	//assuming that '<!--' was already consumed on input stream
	//This will throw Err class object in case of problems
	//
	void SkipComment();

	int SkipUntil(CUSTR & s);

};

#endif

