
// CSTRM Implementation of a Stream Class V1.0
//
/////////////////////////////////////////////////////////////////////
#include "cstrm.h"

#include "chartab.h"

#define INIT_LINE_CHAR 1

using namespace s; //Use standard stringy functions declared in CUSTR

//Construct CSTRM file name with standard open read parameters
void CSTRM::Init(const char *pcfinName, UCHAR *pcBufIn, int iBufSzIn)
{
	if(pcfinName == NULL) return;

	iChar = iLine = INIT_LINE_CHAR;

	fIn.open(pcfinName, ios::in|ios::binary);

	if(! fIn.is_open()) throwErr(pcfinName, ERR_FILEOPEN); //File is not open
	//if(fIn.fail()) throwErr(pcfinName, ERR_FILEOPEN); //File is not open

	GetFPath(pcfinName);

	if(pcBuf == NULL) //Reserve parser buffer if it is'nt there already
	{
		if(pcBufIn != NULL)
		{
			iBufSz = iBufSzIn;
			pcBuf  = pcBufIn;
			bFreeBuf = false;
		}
		else
		{
			iBufSz = 10*1024;
			pcBuf = new UCHAR[iBufSz];
			iBufSz--;                  //Reserve extra token for end terminator
			bFreeBuf = true;

			if(pcBuf == NULL)
			{
				iBufSz=0;
				throw CERR("", ERR_OUTOFMEM); //Can't Allocate Memory :(
				return;
			}
		}
	}

	pcBuf[0] = '\0';
}

#undef INIT_LINE_CHAR

CSTRM::CSTRM(char *pcfinName, UCHAR *pcBufIn, int iBufSzIn)
           :sPath(), sFName(), iLine(0), iChar(0),fIn(), iBufSz(0),bFreeBuf(true), pcBuf(NULL)
{
	if(pcfinName != NULL) Init(pcfinName, pcBufIn, iBufSzIn);
}

CSTRM::~CSTRM()
{
	if(bFreeBuf == true) delete [] pcBuf;

	fIn.close();
}

#ifndef LF_CHAR
#define LF_CHAR    0xA //Line Feed
#endif

//Get a character from input stream
UCHAR CSTRM::get()
{
	UCHAR c;

	get(c);

	return c;
}

void CSTRM::get(UCHAR & c)
{
	if(fIn.eof() == false)
	{
		fIn.read((char *)&c, sizeof(UCHAR));

		if(c == LF_CHAR)
		{
			iLine++;
			iChar = 1;
		}
		else
			iChar++;
	}
}

//Get line and character position of parser for debugging
//
void CSTRM::getLineCharPos(int & iInLine, int & iInChar) const
{
	iInLine = getLine();
	iInChar = getCharPos();
}

//Return !=0 if everything's fine
//
int CSTRM::open(const char *pcfinName, const CUSTR & sPath, UCHAR *pcBufIn, int iSz)
{
	try
	{
		if(fIn.is_open() == true) fIn.close();
		if(pcfinName == NULL) return fIn.good();

		if(slen((UCHAR *)pcfinName)>2)
		if((pcfinName[0] == '\\' && pcfinName[1] == '\\') || pcfinName[1] == ':')
		{
			Init(pcfinName, pcBufIn, iSz); //Open File as given
			return fIn.good();
		}
		
		CUSTR sNewPath = sPath;       //Open relative to given path
		sNewPath += pcfinName;
		
		Init(sNewPath, pcBufIn, iSz);

		return fIn.good();
	}
	catch(...)
	{
		CUSTR s("ERROR: CSTRM::open Can not open Path: ");
		s += sPath;
		s += " file: ";
		s += pcfinName;
		throw CERR(s);
	}
}

//Fill buffer until either:
//
// 1> Buffer is full
// 2> End of File is found
// 3> A delimiter is found
//
// In any case return last character read in cLimit
//
int CSTRM::ReadLiteral(UCHAR & cLimit)
{
	UCHAR cRd;
	int iCnt;
	pcBuf[0] = 0;

	for(iCnt=0, get(cRd); eof() == false && cRd != cLimit && iCnt<iBufSz; get(cRd))
	{
		pcBuf[iCnt++] = cRd;
	}
	
	pcBuf[iCnt] = '\0';
	cLimit      = cRd;

	return iCnt;
}

//Split Filename+Path into two strings
//
void CSTRM::GetFPath(const char *pcfinName)
{
	int idx;

	sFName = pcfinName;
	if((idx = sFName.Right('\\'))>0)
	{
		idx++;
		sPath  = sFName;
		sFName = sFName.Mid(idx);
		sPath  = sPath.Mid(0,idx);
	}
	else
		sPath = "";
}

//Throw an error message object (Err) with sMess and StdErr
//including line number and file src of error
//
void CSTRM::throwErr(const CUSTR & sMessIn, int iStdErrNo)
{
	CUSTR sPos, sSrc;

	if(iStdErrNo != ERR_FILEOPEN) //Position information required???
	{
		sPos = "at Line: ";
		sPos += iLine;
		sPos += " Pos: ";
		sPos += iChar;
	}

	sSrc  = sPath;
	sSrc += sFName;

	throw CERR(sMessIn, iStdErrNo, sSrc, sPos);
}

//Skip over an XML comment <!-- .... -->
//assuming that '<!--' was already consumed on input stream
//This will throw Err class object in case of problems
//
void CSTRM::SkipComment()
{
	int iState = 0;
	UCHAR cRd;

	for(get(cRd); eof() == false; get(cRd))
	{
		if(cRd == '-')
		{
			if(iState == 0 || iState == 1)
				iState += 1;
			else
				throwErr("",ERR_SYNTAX); //Throw Syntax Error on something like '---'
		}
		else
		{
			if(cRd == '>')
			{
				if(iState == 2) //Found end of comment
					return;
				else
				{
					if(iState >= 1)
						throwErr("",ERR_SYNTAX); //Throw Syntax Error on '->'
				}
			}
			else
				if(iState > 0) throwErr("",ERR_SYNTAX); //Throw Syntax Error on '-- -'
		}
	}

	if(eof() == true) throwErr("",ERR_UNEXP_EOF);
}

int CSTRM::SkipUntil(CUSTR & s)
{
	int iState = 0;
	UCHAR c=0;

	while(iState <= 0 && !eof())
	{
		if(s[0] != c)
		{
			while((c = get()) != s[0] && !eof() )
				;
		}

		if(eof()) return false;

		while(!eof())
		{
			if(s.slen() == (iState+1)) return true;

			c = get();

			if(s[iState+1] == c )
				iState++;
			else
			{
				iState = 0;
				break;      /**go back to outer loop**/
			}
		}
	}
	
	if(s.slen() == (iState+1) && c == s[iState])
		return true; /** Match FOUND: Stop Skipping **/

	return false;
}

//Read a name entity from stream and return buffer
//First letter of name is in cRd
//
int CSTRM::ReadName(UCHAR & cRd)
{
	int iCnt=0;

	pcBuf[0]='\0';

	if(fIn.eof() == true || isOutOfRange(cRd, NAME1SZ-1, NAME1) == true)
		return -1;

	pcBuf[iCnt++] = cRd;
	pcBuf[iCnt]   = '\0';

	for(cRd = (UCHAR)fIn.get(); fIn.eof() == false && iCnt < iBufSz &&
	    isOutOfRange(cRd, NAME2SZ-1, NAME2) == false; cRd = (UCHAR)fIn.get())
	{
		pcBuf[iCnt++] = cRd;
		pcBuf[iCnt]   = '\0';
	}

	if(fIn.eof() == true) return -1;

	return iCnt;
}
