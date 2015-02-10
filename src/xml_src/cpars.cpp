
#include "cpars.h"
#include "cerr.h"

CPARS::CPARS(char *pcInName) : xsInp(pcInName), cLast(0), bClean(true)
{}

CPARS::~CPARS()
{}

//Create a tag object from a string and a typ descriptor
//and resolve DTD reference hidden from user of class
#define E_LOC "CPARS::CreateTag"
CTAG *CPARS::CreateTag(const UCHAR *pcTag, ETAGTYP InTagTyp)
{
	CTAG *prtNext;

	if((prtNext = new CTAG(pcTag, InTagTyp))!=NULL)
	{
		prtNext->ResolveRefs(dtProf);

		if(prtNext->getTagTyp() == TGP_ERROR)  //TGP_ERROR indicates syntax error
			xsInp.throwErr(E_LOC,ERR_SYNTAX);   //Propagate this further
	}

	return prtNext;
}
#undef E_LOC

//Skip over an XML include section <![ .... ]>
//assuming that '<![' was already consumed on input stream
//
CTAG *CPARS::SkipInclude()
{
	UCHAR cRd, cLast=0;

	//Read until we find something like ']>'
	//
	for(xsInp.get(cRd); xsInp.eof() == false; cLast = cRd, xsInp.get(cRd))
	{
		if(cLast == ']' && cRd == '>')
			break;
	}

	if(xsInp.eof() == true) xsInp.throwErr("", ERR_UNEXP_EOF);

	return NULL;
}

//States of simple tag parser
//
#define STARTING 1 //it is not known whether we are in or outside of a tag
#define IN_TAG   2 //We have seen a '<' so we are now inside of a tag
#define OUT_TAG  3 //We have seen a '>' so we are now outside of a tag

#define SPEC_TAGSSZ  9
#define SPEC_TAGSCNT 3
const UCHAR SPEC_TAGS[] = "!--      !DOCTYPE ![       ";

#define MTS_COMMENT 1
#define MTS_DOCTYPE 2
#define MTS_INCLUDE 3

//Read a raw tag from input stream and return string in CTAG
//This function will skip comments and CTAG will pre-process
//parsed information from input stream
//
//TAG Name, Type and remaining content should then be available through
//CTAG properties
//
CTAG *CPARS::getNextRawTag()
{
	int iCnt, iState = STARTING, iMatch=-1;
	UCHAR cRd;
	MSTATE mtKey;

	if(cLast == BG_TAG_CHAR) iState = IN_TAG;
	cLast = 0;

	for(xsInp.get(cRd), iCnt=0; xsInp.eof() == false && iCnt < xsInp.igetBufSz();
	    xsInp.get(cRd))
	{
		//Skip trailing white spaces
		if(ISWHITE(cRd) && iCnt == 0) continue;

		if(cRd == BG_TAG_CHAR) //Process '<' character
		{
			bClean = false;
			if(iState == STARTING)
			{
				iState = IN_TAG;

				if(eof() == false)
				{
					xsInp.get(cRd);
					if(NOWHITE(cRd) && cRd != '<'&& cRd != '>')
					{
						xsInp.pcBuf[iCnt++] = cRd; //Add first character of tag
						xsInp.pcBuf[iCnt] = '\0';

						iMatch = FindKeyWord(SPEC_TAGS, cRd, SPEC_TAGSCNT, SPEC_TAGSSZ, mtKey);
						continue;
					}

					xsInp.throwErr("", ERR_SYNTAX);  //Syntax Error???
				}

				xsInp.throwErr("", ERR_UNEXP_EOF);  //Error EOF???
			}
			else
			{
				if(iState == IN_TAG)               //End of tag content
					xsInp.throwErr("", ERR_SYNTAX);  //Syntax Error???
				else //iState == OUT_TAG -> return text as tag-content-text
				{
					cLast = cRd;
					return CreateTag(xsInp.pcBuf, TGP_CONTENT);
				}
			}
		}
		else //Process '>' character
		{
			//Input stream must start with white space (processed above)
			//or '<' character !!!
			if(bClean == true) xsInp.throwErr("", ERR_SYNTAX);

			if(cRd == EO_TAG_CHAR)
			{
				if(iState == IN_TAG)
				{
						cLast = cRd;
						return CreateTag(xsInp.pcBuf);
				}
				else
				{
					xsInp.throwErr("", ERR_SYNTAX);  //Syntax Error???
					return NULL;
				}
			}
		}

		//Force an advanced state as soon as we start taking notes!
		//We are reading tag content
		if(iCnt == 0 && iState == STARTING) iState = OUT_TAG;

		xsInp.pcBuf[iCnt++] = cRd;
		xsInp.pcBuf[iCnt] = '\0';
		bClean = false;

		if(iState == IN_TAG && iMatch == MST_MATCH)
			iMatch = FindKeyWord(SPEC_TAGS, cRd, SPEC_TAGSCNT, SPEC_TAGSSZ, mtKey);

		if(iMatch > 0) //Process special XML Tags if one was recognized
		{
			switch(iMatch)
			{
				case MTS_COMMENT:             //Skip comments completely
					xsInp.SkipComment();
					return NULL;

				case MTS_INCLUDE:            //Skip includes completely
					return SkipInclude();

				case MTS_DOCTYPE:            //Process !DOCTYPE Tag content
					dtProf.ParseDOCTYPEDecl(&xsInp);
					return NULL;
			}
		}
	}

	if(eof() == true)
	{
		if(iState != STARTING)
			xsInp.throwErr("", ERR_UNEXP_EOF);  //End of file
	}
	else
	{
		if(iCnt > 0)
			xsInp.throwErr("", ERR_BUFFER_OVFL);  //Buffer Overflow :(
		else
			//Unknown Error (should never occur...!? but hey, who says never?)
			xsInp.throwErr("", ERR_UNKNOWN);
	}

	return NULL;
}

#undef SPEC_TAGS_SZ
#undef SPEC_TAGS_CNT

#undef STARTING
#undef IN_TAG
#undef OUT_TAG
