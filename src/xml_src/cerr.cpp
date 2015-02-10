// Err.cpp: implementation of the Err class. V1.0
//
//////////////////////////////////////////////////////////////////////

#include "cerr.h"

//Simple Error handling function
UCHAR xpERR_UNKNOWN_CODE[] = "UNKNOWN ERROR CODE";

#define XP_ERR_SZ 9
const UCHAR xpERR_STR[XP_ERR_SZ][23] =
{
	{"no error"},
	{"syntax error"},
	{"buffer overflow"},
	{"unknown error"},
	{"unexpected end of file"},
	{"file open error"},
	{"run out of memory"},
	{"index out of range"},
	{"internal program error"}
};

///////////////////////////
// Construction/Destruction
///////////////////////////
CERR::CERR(const CUSTR & sMessIn, int iErrNoIn, const CUSTR & sSrcIn, const CUSTR & sPosIn)
{
	if(iErrNoIn >= 0 && iErrNoIn < XP_ERR_SZ)
		pcErr = xpERR_STR[iErrNoIn];
	else
		pcErr = xpERR_STR[ERR_UNKNOWN]; //No description for this error number

	sMess   = sMessIn;
	iErrNo  = iErrNoIn;

	sSrc   = sSrcIn;
	sPos   = sPosIn;
}

void CERR::printfMsg(FILE *fp)
{
	if(fp == NULL) return;

	//print Std Error Code, Std-Msg and customized message (should always be set)
	fprintf(fp, "(Code %i) %s\n%s\n", iErrNo, pcErr, (const char *)sMess);

	//print Extended error info about file source and position
	if(sSrc.slen() >0) 	fprintf(fp, "Source: %s\n", (const char *)sSrc);

	if(sPos.slen() >0) 	fprintf(fp, "   Pos: %s\n", (const char *)sPos);
}

CUSTR CERR::getMsg()
{
	CUSTR s;

	s += "(Code ";
	s += iErrNo;
	s += ") ";
	s += pcErr;
	s += '\n';
	s += sMess;
	s += '\n';

	//print Std Error Code, Std-Msg and customized message (should always be set)
	//fprintf(fp, "(Code %i) %s\n%s\n", iErrNo, pcErr, (const char *)sMess);

	//print Extended error info about file source and position
	if(sSrc.slen() >0)
	{
		s += " Src:";
		s += sSrc;
		s += '\n';
	}

	if(sPos.slen() >0)
	{
		s += " Pos: ";
		s += sPos;
		s += '\n';
	}

	return s;
}

const UCHAR *CERR::getStdErrMsg(){ return pcErr; }
const CUSTR  CERR::getsErrMsg()  { return sMess; }
const int    CERR::getStdErrNo() { return iErrNo;}

const CUSTR CERR::getsErrSrc(){ return sSrc; }
const CUSTR CERR::getsErrPos(){ return sPos; }

///////////////////////////////////////////////////////////////////
//return human readable string for error code from above functions
const UCHAR *getError(const int iError)
{
	if(iError >= 0 && iError < XP_ERR_SZ) return xpERR_STR[iError];

	return xpERR_UNKNOWN_CODE; //Return error on error code :(
}
