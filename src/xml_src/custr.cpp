
// CUSTR Implementation of an UnSigned String Class V1.0
//
/////////////////////////////////////////////////////////////////////

//
//This is a string class to handle strings with the full
//character set of 256 different values
//
//We use 8 Bit unsigned int as character data type

#include "custr.h"

#include <memory.h>
#include <stdlib.h>

#define USTR_BUF 64 //Block Size for re-allocation scheme

// Construct string objects through different methodes
//
void CUSTR::Reset()
{
	iStrSz = 0;
	pcStr  = NULL;
}

CUSTR::CUSTR() : pcStr(NULL), iStrSz(0)
{
}

void CUSTR::AllocString(const UCHAR *pc)
{
	if(pc != NULL)
	{
		iStrSz = s::slen(pc)+1;
		pcStr  = new UCHAR[iStrSz]; //Get memory and copy
		
		s::scpy(pcStr,pc);
	}
	else
		Reset();
}

//Call standard constructor function above...
CUSTR::CUSTR(const UCHAR *pc) { AllocString(pc);          }
CUSTR::CUSTR(char *pc)        { AllocString((UCHAR *)pc); }
CUSTR::CUSTR(const char *pc)  { AllocString((UCHAR *)pc); }

// Destructor
//
CUSTR::~CUSTR()
{
	delete [] pcStr;
}

CUSTR::CUSTR(UINT iCntChars)
{
	Reset();

	if(iCntChars > 0)
	{
		iStrSz   = iCntChars;
		pcStr    = new UCHAR[iStrSz]; //Get memory and copy
		pcStr[0] = '\0';
	}
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//A copy constructor is required for classes that manage
//dynamic memory via new, malloc or delete, free - This is
//also true for the equal operator= - Make sure both are stated and
//implement, see deep copies versus compiler automated shallow copy
//See "C++ Unleashed" by Jesse Liberty on Page 104
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
CUSTR::CUSTR(const CUSTR & s)
{
	pcStr = NULL;

	if(s.pcStr != NULL)
	{
		iStrSz = s::slen(s.pcStr)+1;
		pcStr = new UCHAR[iStrSz];     //Get memory and copy
		s::scpy(pcStr,s.pcStr);
	}
}

int CUSTR::slen() const
{
	return s::slen(pcStr);
}

// Compare two strings case-insensitive
int CUSTR::scmp_ci( const UCHAR *s1) const
{
	return s::scmp_ci(this->pcStr, s1);
}

// Assignment operator for simple UCHAR null terminated strings
//
CUSTR & CUSTR::operator=(const UCHAR *pc)
{
	delete [] pcStr;
	pcStr  = NULL;
	iStrSz = 0;

	if(pc != NULL)
	if(pc != this->pcStr)
	{
		iStrSz = s::slen(pc)+1;
		pcStr = new UCHAR[iStrSz];     //Get memory and copy
		s::scpy(pcStr,pc);
	}

	return *this;
}

// Assignment operator for simple UCHAR null terminated strings
//
CUSTR & CUSTR::operator=(const char *pc)
{
	delete [] pcStr;
	pcStr  = NULL;
	iStrSz = 0;

	if(pc != NULL)
	if((UCHAR *)pc != this->pcStr)
	{
		iStrSz = s::slen((const UCHAR *)pc)+1;
		pcStr = new UCHAR[iStrSz];     //Get memory and copy
		s::scpy(pcStr,(const UCHAR *)pc);
	}

	return *this;
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//A copy constructor is required for classes that manage
//dynamic memory via new, malloc or delete, free - This is
//also true for the equal operator= - Make sure both are stated and
//implement. Deep copies versus compiler automated shallow copy
//See "C++ Unleashed" by Jesse Liberty on Page 104
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
CUSTR & CUSTR::operator=(const CUSTR & right)
{
	delete [] pcStr;
	iStrSz = 0;

	if(right.pcStr != this->pcStr)
	{
		iStrSz = s::slen(right.pcStr)+1;
		pcStr  = new UCHAR[iStrSz];     //Get memory and copy
		s::scpy(pcStr,right.pcStr);
	}

	return *this;
}

CUSTR & CUSTR::operator=(const CUSTR *pusPtr)
{
	if(pusPtr != NULL) *this = *pusPtr;

	return *this;
}

//String index operator with simulated null termination
//
UCHAR CUSTR::operator[](int index)  const
{
	if(index < 0) return 0;

	if(iStrSz > (UINT)index && s::slen(pcStr) > index)
		return pcStr[index];
	else
		return 0;
}

//
//Add a character to string content
//
void CUSTR::operator+=(const char right)
{
	UINT j = (UINT)s::slen(pcStr);

	if(j + 1 >= iStrSz) //Alloc more mem and copy existing string
	{
		UCHAR *pcTmp;
		iStrSz = USTR_BUF + j + 1;

		pcTmp = new UCHAR[iStrSz];
		s::scpy(pcTmp, pcStr);
		
		delete [] pcStr;

		pcStr = pcTmp;
	}

	pcStr[j++] = (UCHAR)right; //Copy new character
	pcStr[j]   = '\0';
}

void CUSTR::operator+=(const UCHAR right)
{
	UINT j = (UINT)s::slen(pcStr);

	if(j + 1 >= iStrSz) //Alloc more mem and copy existing string
	{
		UCHAR *pcTmp;
		iStrSz = USTR_BUF + j + 1;

		pcTmp = new UCHAR[iStrSz];
		s::scpy(pcTmp, pcStr);
		
		delete [] pcStr;

		pcStr = pcTmp;
	}

	pcStr[j++] = (UCHAR)right; //Copy new character
	pcStr[j]   = '\0';
}
//
//Add contents of two strings
//
void CUSTR::operator+=(const CUSTR & right)
{
	UINT i = (UINT)s::slen(right.pcStr);
	UINT j = (UINT)s::slen(pcStr);

	//Copy existing string
	if(i + j + 1 < iStrSz)
	{
		for(UINT k=0 ;k<i;k++,j++)
		{
			pcStr[j] = right.pcStr[k];
		}
		pcStr[j] = '\0';
	}
	else //Get more memory and concatinate both strings
	{
		UCHAR *pcTmp;
		iStrSz = i + j + 1;

		pcTmp = new UCHAR[iStrSz];
		s::scpy(pcTmp, pcStr);
		
		delete [] pcStr;

		s::scpy(&pcTmp[j], right.pcStr);
		pcStr = pcTmp;
	}
}

//
//Add contents of two strings
//
void CUSTR::operator+=(const char *pcRight)
{
	UINT i = (UINT)s::slen((UCHAR *)pcRight);
	UINT j = (UINT)s::slen(pcStr);

	//Copy existing string
	if(i + j + 1 < iStrSz)
	{
		for(UINT k=0 ;k<i;k++,j++)
		{
			pcStr[j] = pcRight[k];
		}
		pcStr[j] = '\0';
	}
	else //Get more memory and concatinate both strings
	{
		UCHAR *pcTmp;
		iStrSz = i + j + 1;

		pcTmp = new UCHAR[iStrSz];
		s::scpy(pcTmp, pcStr);
		
		delete [] pcStr;

		s::scpy(&pcTmp[j], (UCHAR *)pcRight);
		pcStr = pcTmp;
	}
}

//
//Add contents of two strings
//
void CUSTR::operator+=(const UCHAR *pcRight)
{
	UINT i = (UINT)s::slen((UCHAR *)pcRight);
	UINT j = (UINT)s::slen(pcStr);

	//Copy existing string
	if(i + j + 1 < iStrSz)
	{
		for(UINT k=0 ;k<i;k++,j++)
		{
			pcStr[j] = pcRight[k];
		}
		pcStr[j] = '\0';
	}
	else //Get more memory and concatinate both strings
	{
		UCHAR *pcTmp;
		iStrSz = i + j + 1;

		pcTmp = new UCHAR[iStrSz];
		s::scpy(pcTmp, pcStr);
		
		delete [] pcStr;

		s::scpy(&pcTmp[j], (UCHAR *)pcRight);
		pcStr = pcTmp;
	}
}

//Decimal power table for integer code translation
//Int -> String
//This table must reflect the size of int on a given
//computer!
//
#define INT_DEC_POWSZ 10
const int INT_DEC_POW[INT_DEC_POWSZ] =
{
	1000000000,
	100000000,
	10000000,
	1000000,
	100000,
	10000,
	1000,
	100,
	10,
	1
};

int WRITE_CH(UCHAR *pcRet, int iSz, int idxWrit, UCHAR c)
{
	if(pcRet!=NULL && iSz > idxWrit+1)
	{
		pcRet[idxWrit++] = c;
		pcRet[idxWrit]   = '\0';
	}

	return idxWrit;
}

//Convert an integer value into a string representation
//(Decimal number base 10)
//
//Return number of characters written
//This includes the zero terminator, so the returned value
//is always greater equal one
//
int CUSTR::i2a(int iVal, UCHAR *pcRet, int iSz)
{
	int iRetSz=1, idx, idxWrit=0; //iRetSz includes terminator byte
	UCHAR cDig;

	if(iVal < 0) //Negative value needs a '-' character in front
	{
		idxWrit = WRITE_CH(pcRet, iSz, idxWrit, '-');
		iRetSz++;
		iVal *= -1;
	}
	else
	{
		if(iVal == 0) //Write zero character and return
		{
			idxWrit = WRITE_CH(pcRet, iSz, idxWrit, '0');
			iRetSz++;
			return iRetSz;
		}
	}

	for(idx=0;idx<INT_DEC_POWSZ && iVal < INT_DEC_POW[idx];idx++) ;

	for( ; iVal>=0 && idx < INT_DEC_POWSZ; idx++)
	{
		for(cDig='0'; iVal>0 && cDig<= '9' && iVal >= INT_DEC_POW[idx]; )
		{
			iVal -= INT_DEC_POW[idx];
			cDig++;
		}

		if(cDig > '9') return iRetSz; //Check this for safety from exceptions

		idxWrit = WRITE_CH(pcRet, iSz, idxWrit, cDig);
		iRetSz++; //Write new digit
	}

	return iRetSz;
}

// Persist integer value in string
void CUSTR::operator+=(const int iVal)
{
	int iLen = i2a(iVal);      //how long will the number be???

	UCHAR *pcNew; //Get enough memory for number

	if((pcNew = new UCHAR[iLen])==NULL) return;

	i2a(iVal, pcNew, iLen);   //Convert number

	operator+=(pcNew);        //Make addition to this string

	delete [] pcNew;
}

CUSTR CUSTR::operator+(const CUSTR & str) const
{
	CUSTR s(pcStr);

	s += str;

	return s;
}

CUSTR CUSTR::operator+(const int iVal) const
{
	CUSTR s(pcStr);

	s += iVal;

	return s;
}

//Shorten string by this many characters
//
void CUSTR::operator-=(const int iShort)
{
	int i = s::slen(pcStr);

	if(pcStr != NULL )
	{
		if(i >= iShort)
			pcStr[i-iShort] = '\0';
		else
			pcStr[0] = '\0';
	}
}

void CUSTR::scat_n(UINT iSz, CUSTR & sRight)
{
	UINT j = (UINT)s::slen(sRight.pcStr);

	if(iSz > j) iSz = j; //Copy at max available characters

	j = (UINT)s::slen(pcStr);

	if(iSz + j + 1 < iStrSz) //Copy existing string
	{
		for(UINT k=0 ;k<iSz;k++,j++)
		{
			pcStr[j] = sRight.pcStr[k];
		}
		pcStr[j] = '\0';
	}
	else //Get more memory and concatinate both strings
	{
		UCHAR *pcTmp;
		iStrSz = iSz + j + 1;

		pcTmp = new UCHAR[iStrSz];
		s::scpy(pcTmp, pcStr);
		
		delete [] pcStr;

		s::scpy(&pcTmp[j], sRight.pcStr);
		pcStr = pcTmp;
	}
}

void CUSTR::print(FILE *fp) const
{
	fprintf(fp,"%s",this->pcStr);
}

//Make a partial copy of a string...
//
CUSTR CUSTR::Mid(int idxBegin) const
{
	int i = s::slen(this->pcStr);
	CUSTR sRet;

	//This parameters are not useful so we get out of here
	//(Could throw an exception too!?)
	//
	if(i < idxBegin) return sRet;

	i = s::slen(&this->pcStr[idxBegin]);

	sRet.pcStr = s::sdupn(&pcStr[idxBegin], i);
	sRet.iStrSz = s::slen(sRet.pcStr);

	return sRet;           //and return ref
}

//Make a partial copy of a string...
//
CUSTR CUSTR::Mid(int idxBegin, int iLen) const
{
	int i = s::slen(this->pcStr);
	CUSTR sRet;

	//This parameters are not useful so we get out of here
	//(Could throw an exception too!?)
	//
	if(iLen > 0 && i < (idxBegin+iLen)) return sRet;

	sRet.pcStr = s::sdupn(&pcStr[idxBegin], iLen);
	sRet.iStrSz = s::slen(sRet.pcStr);

	return sRet;           //and return string
}

CUSTR & CUSTR::replaceAt(int iBeg, int iReplLen, UCHAR c)
{
	int iLen = slen();

	if(iBeg < 0 || iReplLen <= 0 || (iBeg + iReplLen) >= iLen) return *this;

	//Insert new character and shorten string (don't get new memory)
	pcStr[iBeg++] = c;
	for(iReplLen=iReplLen+iBeg; iReplLen < iLen; iReplLen++)
		pcStr[iBeg++] = pcStr[iReplLen];

	pcStr[iBeg] = '\0';
	iStrSz = this->slen();

	return *this;           //and return ref
}

CUSTR & CUSTR::replaceAt(int iBeg, int iReplLen, const UCHAR *pc)
{
	int iLen = slen(), iAddLen = s::slen(pc), iNewLen, i,j;

	if(iBeg < 0 || iReplLen <= 0 || (iBeg + iReplLen) >= iLen) return *this;

	iNewLen = (iLen - iReplLen) + iAddLen; //Get new string length

	UCHAR *pcNew = new UCHAR[iNewLen+1];

	//Copy first part of string into new string
	for(i=0; i<iBeg; i++) pcNew[i] = pcStr[i];

	//Get new middle part for replacement
	for(j=0; j<iAddLen; i++, j++) pcNew[i] = pc[j];

	//... and attach remainder of old string
	for(j=iBeg + iReplLen+1; j<iLen; i++, j++) pcNew[i] = pcStr[j];

	pcNew[i] = '\0';

	delete [] pcStr;

	pcStr  = pcNew;
	iStrSz = this->slen();

	return *this;           //and return ref
}

//Replace the occurrence of c with cNewChar in all spots of the string
CUSTR & CUSTR::Replace(const UCHAR cNewChar, const UCHAR c)
{
	int iLen = slen(), j;

	if(cNewChar != '\0')
	for(j=0; j<iLen; j++)
	{
	 if(pcStr[j] == c) pcStr[j] = cNewChar;
	}

	return *this;           //and return ref
}

//Split one string (this) into a left and right remainder
//Assign left remainder to s and right remainder to current
//(this) string
//idxSplit shows where the string should be splitted
//
CUSTR & CUSTR::Split(int idxSplit, CUSTR & s)
{
	int iLen = this->slen(),i;

	//Nothing to split in this case
	if(idxSplit <= 0 || idxSplit > iLen)
	{
		s = "";
		return s;
	}
	
	delete [] s.pcStr; //Copy First part into s
	
	s.pcStr = new UCHAR[idxSplit+1];
	
	for(i=0;i<idxSplit;i++)
	{
		s.pcStr[i] = pcStr[i];
	}

	s.pcStr[i] = '\0';   //Terminate string and set new length
	s.iStrSz = idxSplit;

	*this = Mid(idxSplit); //Retain right remainder of string

	return s;
}

//Split current string into s but don't remove remainder
//iEnd points at the letter that is not copied,
//in other words, it points 1 letter past the split on the right side
//------------------------------------------------------------------>
CUSTR & CUSTR::Split(int iBeg, int iEnd, CUSTR & s)
{
	int iLen = this->slen(),i;

	//Nothing to split in this case
	if(iEnd <= 0 || iEnd > iLen || iEnd <= iBeg )
	{
		s = "";
		return s;
	}

	int iNewLen = iEnd - iBeg; //Bugfix: Copy without last letter

	delete [] s.pcStr; //Copy First part into s
	
	s.pcStr = new UCHAR[iNewLen+1];
	
	for(i=0;i<iNewLen;i++)
	{
		s.pcStr[i] = pcStr[iBeg++];
	}

	s.pcStr[i] = '\0';   //Terminate string and set new length
	s.iStrSz = iNewLen;

	return s;
}

//Convert string contents to lower or upper case
CUSTR CUSTR::LowerCase()
{
	CUSTR s(this->pcStr);

	s::sToLower(s.pcStr);

	return s;
}

CUSTR CUSTR::UpperCase()
{
	CUSTR s(this->pcStr);

	s::sToUpper(s.pcStr);

	return s;
}

//Find a leftmost character in a string and return index
//
int CUSTR::Left(UCHAR c) const
{
	int iLen = this->slen(),i;

	for(i=0;i<iLen;i++)
	{
		if(this->pcStr[i] == c) return i;
	}

	return -1;
}

int CUSTR::Right(UCHAR c) const
{
	for(int i = this->slen(); i>=0; i--)
	{
		if(this->pcStr[i] == c) return i;
	}

	return -1;
}

/*

//Trim off leading and ending control sequences
//(This version is for static strings
//-> pcIn is returned, so don't set to it and free then after!)
//
UCHAR *stTrim(UCHAR *pcIn)
{
	UCHAR *pcPos, *pcPos1 = pcIn;
	
	if(*pcIn == '\0') return pcIn;

	while(*pcPos1 != '\0' && *pcPos1 < '!')
		pcPos1++;

	if(*pcPos1 != '\0')
	{
		pcPos = pcPos1 + slen(pcPos1) - 1;

		while(pcPos1 < pcPos && *pcPos < '!')
			pcPos--;

		pcPos[1] = '\0';
	}
	else
		pcPos = pcPos1;

	// String contained no printable words
	if((pcPos1 == pcPos && *pcPos < '!') || pcPos1 > pcPos)
	{}
	else
	{
		pcIn = pcPos1;
	}

	return pcIn;
}

// This version of scat return a cocatination of the given strings
UCHAR *schcat(UCHAR *pcStr, const UCHAR cLetter)
{
	int i;
	UCHAR *pcRet;

	i = slen(pcStr);
	if((pcRet = malloc(sizeof(char) *  (i + 2)))!= NULL)
	{
		memcpy(pcRet, pcStr, sizeof(char) * i);
		pcRet[i] = cLetter;
		pcRet[i+1]   = '\0';
	}
	
	return pcRet;
}

UCHAR *scat(UCHAR *pcStr, UCHAR *pcStr1)
{
	int i , j;
	UCHAR *pcRet;

	j = slen(pcStr);
	i = slen(pcStr1);
	if((pcRet = malloc(sizeof(char) * (i+j+1)))!= NULL)
	{
		memcpy(pcRet    , pcStr, sizeof(char) * j);
		memcpy(&pcRet[j], pcStr, sizeof(char) * i);
		pcRet[i+j]   = '\0';
	}
	
	return pcRet;
}

// Look for a character starting from left side
UCHAR *schl(UCHAR *pcStr, const UCHAR cLetter)
{
	while(cLetter != *pcStr && *pcStr != '\0') pcStr++;

	if(*pcStr != '\0')
		return pcStr;
	else
		return NULL;
}

// Read from FILE *fp until '\n' and returns a resized buffer
#define E_LOC "ERROR: scmp:sGetLineFromFile"
UCHAR *sGetLineFromFile( UCHAR **pcBuf, int *iBufSize, int iBlockLen, FILE *fp)
{
	int iLen;
	UCHAR *pcTmp;

	if((pcTmp=fgets(*pcBuf, *iBufSize, fp)) != NULL)
	{
		iLen = slen(*pcBuf);
		if(iLen > 0)
		{
			while(*(*pcBuf + iLen-1) != '\n' && pcTmp != NULL)
			{
				pcTmp = malloc(sizeof(char) * (*iBufSize + iBlockLen));
				if(pcTmp != NULL)
				{
					memcpy(pcTmp, *pcBuf, sizeof(char) * *iBufSize);
					free(*pcBuf);
					*pcBuf = pcTmp;
					pcTmp += *iBufSize - 1;
					pcTmp=fgets(pcTmp, iBlockLen+1, fp);
					*iBufSize += iBlockLen;
					iLen = slen(*pcBuf);
				}
				else
				{
					printf("%s Failed to allocate %i bytes!!!", E_LOC, *iBufSize + iBlockLen);
					return NULL;
				}
			}
		}

		return *pcBuf;
	}

	return NULL;
}
#undef E_LOC
*/

int CUSTR::operator==(const UCHAR *s) const
{ return(s::scmp(pcStr, s) == 0); }

int CUSTR::operator==(const char *s) const
{ return(s::scmp(pcStr, (UCHAR *)s) == 0); }

int CUSTR::operator==(const CUSTR & s) const
{ return(s::scmp(pcStr, s.pcStr) == 0); }

int CUSTR::operator!=(const UCHAR *s) const //InEquality operator
{
	return(s::scmp(pcStr, s) != 0);
}

int CUSTR::operator!=(const char  *s) const
{
	return(s::scmp(pcStr, (UCHAR *)s) != 0);
}

int CUSTR::operator!=(const CUSTR & s) const
{
	return(s::scmp(pcStr, s.pcStr) != 0);
}

int CUSTR::operator>(const UCHAR *s) const //Greater than inEquality operator
{ return(s::scmp(pcStr, s) > 0); }

int CUSTR::operator>(const CUSTR & s) const  //Less than inEquality operator
{ return(s::scmp(pcStr, s.pcStr) > 0); }

int CUSTR::operator>(const char *s) const  //Greater than inEquality operator
{ return(s::scmp(pcStr, (UCHAR *)s) > 0); }

int CUSTR::operator<(const UCHAR *s) const //Less than inEquality operator
{ return(s::scmp(pcStr, s) < 0); }

int CUSTR::operator<(const char *s) const  //Less than inEquality operator
{ return(s::scmp(pcStr, (UCHAR *)s) < 0); }

int CUSTR::operator<(const CUSTR & s) const  //Less than inEquality operator
{ return(s::scmp(pcStr, s.pcStr) < 0); }

void CUSTR::SetChar(int i, UCHAR c)
{
	if(i < this->slen())
	{
		pcStr[i] = c;
	}
}

const UCHAR *CUSTR::SubString(int i) const
{
	return &pcStr[i];
}

#define _USTR_BUFFER 256

ostream & operator<<(ostream & is, CUSTR & s)
{
	is << s.pcStr;

	return is;
}

istream & operator>>(istream & is, CUSTR & s)
{
	delete [] s.pcStr;

	s.pcStr    = new UCHAR[_USTR_BUFFER];
	s.iStrSz   = _USTR_BUFFER;
	s.pcStr[0] = '\0';

	is.getline((char *)s.pcStr,s.iStrSz);
/*
	UCHAR c=' ';
	int q=0, i=0;
	UINT idx=0;

		if(idx+1 >= s.iStrSz) //Grow buffer???
		{
			UCHAR *pc = new UCHAR[s.iStrSz+_USTR_BUFFER];

			for(UINT i=0;i<s.iStrSz;i++) //Copy string contents
			{
				pc[i] = s.pcStr[i];
			}
			s.iStrSz += _USTR_BUFFER; //Increase buffer size
			delete [] s.pcStr;       //Delete old string and set new ref
			s.pcStr = pc;
		}
		else
		{
			s.pcStr[idx++] = c;
			s.pcStr[idx]   = '\0';
		}
	}
*/
	return is;
}

#undef _USTR_BUFFER

//Find the occurrence of a substring within a given string
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CUSTR::FindSubString(const char *pcSubStr) const
{
	if(pcSubStr == NULL ) return -1;
	if(*pcSubStr == '\0') return -1;
	
	if(pcStr == NULL ) return -1;
	if(*pcStr == '\0') return -1;
	
	int i=0, k=0;
	while(pcStr[i] != '\0')
	{
		//increase pointer until we find the first char of substring or end of string
		for(;pcSubStr[0] != pcStr[i] && pcStr[i] != '\0'; i++)
			;
		
		if(pcStr[i] == '\0')
		{
			if(pcSubStr[1] == '\0' && i > 1)
			if(pcSubStr[0] == pcStr[i-1])
				return i-1;                    //Match substring of length 1

			return -1; //get out if this is the end of string
		}

		i++;
		for(k=0;pcSubStr[k+1] == pcStr[i+k] && pcSubStr[k+1] != '\0' && pcStr[i+k] != '\0'; k++)
		;

		if(pcSubStr[k+1] == '\0')
		{
			if(pcSubStr[k] == pcStr[i+k-1])
				return(i);
		}
		
		i++;
	}
	return -1;
}

//Find the occurrence of a substring within a given string
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CUSTR::FindChar(const UCHAR c, int i) const
{
	if(pcStr == NULL ) return -1;
	if(i > s::slen(pcStr) || i <0) return -1;

	for( ;pcStr[i] != '\0'; i++) //increase pointer until we find the char
	{
		if(c == pcStr[i]) return i;
	}

	return -1;
}

//Find the occurrence of a substring within a given string
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CUSTR::FindChar(const UCHAR c) const
{
	if(pcStr == NULL ) return -1;
	if(*pcStr == '\0') return -1;

	for(int i=0;pcStr[i] != '\0'; i++) //increase pointer until we find the char
	{
		if(c == pcStr[i]) return i;
	}

	return -1;
}

//Find the occurrence of a substring within a given string
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CUSTR::FindCharFromRight(const UCHAR c, int i) const
{
	if(pcStr == NULL ) return -1;
	if(i > s::slen(pcStr) || i <0) return -1;

	for( ;i >0; i--) //increase pointer until we find the char
	{
		if(c == pcStr[i]) return i;
	}

	return -1;
}

//Find the occurrence of a substring within a given string
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CUSTR::FindCharFromRight(const UCHAR c) const
{
	if(pcStr == NULL ) return -1;
	if(*pcStr == '\0') return -1;

	for(int i=s::slen(pcStr); i >0; i--) //decrease ptr 'til we find char
	{
		if(c == pcStr[i]) return i;
	}

	return -1;
}

void CUSTR::Trim()
{
	this->pcStr = s::sTrim(this->pcStr);
	this->iStrSz = s::slen(this->pcStr);
}

void CUSTR::TrimAllWhiteSpaces()
{
	this->pcStr = s::sTrimAllWhiteSpaces(this->pcStr);
	this->iStrSz = s::slen(this->pcStr);
}

/*
// Maximal size of HTML code sequences like &amp;
// that will be recognized and converted to ASCII
#define HTMLCODE_SZ 7
#define HTMLCODES 117 // Number of codes to keep track of

typedef struct hcode
{
	UCHAR Code[HTMLCODE_SZ];
	UCHAR cLetter;

}HCODE;

static HCODE HCodes[HTMLCODES] =
{
{"#033", '!'},{"quot",'\"'},{"#035", '#'},{ "amp", '&'},
{"#039",'\''},{"#040", '('},{"#041", ')'},{"#044", ','},
{"#059", ';'},
{  "lt", '<'},{  "gt", '>'},{"#063", '?'},
{"#091", '['},{"#092",'\\'},{"#093", ']'},{"#094", '^'},
{"#096", '`'},{"#123", '{'},{"#124", '|'},{"#125", '}'},

{"#130",'\''}, // Low left single Quote
{"#132",'\"'}, // Low left double Quote

{"iexcl",''},   // Inverted exclamation mark
{"cent", ''},   // cent                     
{"pound",''},   // pound                    
{"curren",''},  // currency                 
{ "yen", ''},   // Yen symbol               

{"brvbar",''}, // Broken vertikal bar      
{"sect", ''},   // Section sign             
{"uml",  ''},   // Umlaut                   
{"copy", ''},   // Copyright sign           
{"ordf", ''},   // FEMININE ORDINAL INDICATOR

{"laquo",''},// Left Angle pointing quote
{"not", ''}, // Not sign                 
{"shy", ''}, // Soft hyphen              
{"reg", ''}, // Registered Trademark     
{"macr",''}, // Macron accent            

{"deg", ''},    // Degree sign           
{"plusmn",''}, // Plus minus character  
{"sup2", ''},   // Superscript 2         
{"sup3", ''},   // Superscript 3         
{"acute",''},   // Accute accent         

{"micro",''},   // Greek mu (Micro sign)    
{"para", ''},   // Paragraph or pilcrow sign
{"middot",''}, // Middle dot               
{"cedil", ''},  // Cedilla                  
{"sup1", ''},   // Superscript 1            

{"ordm", ''},   // Masculine ordinal        
{"raquo", ''},  // Right pointed angle quote
{"frac14",''},  // 1/4 fraction             
{"frac12",''},  // 1/2 fraction             
{"frac34",''},  // 3/4 fraction             

{"iquest", ''}, // Inverted Question Mark   
{"Agrave", 'ï¿½}, // Capital A grave accent
{"Aacute", 'ï¿½}, // Capital A acute accent
{"Acirc",  'ï¿½}, // Capital A circumflex
{"Atilde", 'ï¿½}, // Capital A tilde

{"Auml", 'ï¿½},   // Capital A umlaut      
{"Aring", 'ï¿½},  // Capital A ring        
{"AElig", 'ï¿½},  // Capital AE ligature   
{"Ccedil", 'ï¿½}, // Capital C,cedilla     
{"Egrave", 'ï¿½}, // Capital E grave accent

{"Eacute", 'ï¿½}, // Capital E acute accent
{ "Ecirc", 'ï¿½}, // Capital E circumflex  
{  "Euml", 'ï¿½}, // Capital E umlaut      
{"Igrave", 'ï¿½}, // Capital I grave accent
{"Iacute", 'ï¿½}, // Capital I acute accent

{"Icirc", 'ï¿½},  // Capital I circumflex  
{ "Iuml", 'ï¿½},  // Capital I umlaut      
{  "ETH", 'ï¿½},  // Capital eth in islandic looks like OE
{"Ntilde",'ï¿½},  // Capital N tilde       
{"Ograve",'ï¿½},  // Capital O grave accent

{"Oacute", 'ï¿½}, // Capital O acute accent
{ "Ocirc", 'ï¿½}, // Capital O circumflex 
{"Otilde", 'ï¿½}, // Capital O tilde     
{  "Ouml", 'ï¿½}, // Capital O umlaut   
{ "times", 'ï¿½}, // times symbol      

{"Oslash", 'ï¿½}, // Capital O with slash  
{"Ugrave", 'ï¿½}, // Capital U grave accent
{"Uacute", 'ï¿½}, // Capital U acute accent
{"Ucirc", 'ï¿½},  // Capital U circumflex  
{"Uuml", 'ï¿½},   // Capital U umlaut      

{"Yactue", 'ï¿½}, // Capital Y  acute accent  
{"THORN", 'ï¿½},  // Capital thorn, islandic  
{"szlig", 'ï¿½},  // small sz ligature, German
{"agrave", 'ï¿½}, // small a grave accent     
{"aacute", 'ï¿½}, // small a acute accent     

{"acirc", 'ï¿½},  // small a circumflex   
{"atilde",'ï¿½},  // small a tilde       
{"auml", 'ï¿½},   // small a umlaut     
{"aring", 'ï¿½},  // small a ring      
{"aelig", 'ï¿½},  // small ae ligature

{"ccedil", 'ï¿½}, // small c cedilla       
{"egrave", 'ï¿½}, // small e grave accent 
{"eacute", 'ï¿½}, // small e acute accent
{"ecirc", 'ï¿½},  // small e circumflex 
{"euml", 'ï¿½},   // small e umlaut    

{"igrave", 'ï¿½}, // small i with grave accent
{"iacute", 'ï¿½}, // small i with acute accent
{"icirc", 'ï¿½},  // small i with circumflex  
{"iuml", 'ï¿½},   // small i with umlaut      
{"eth", 'ï¿½},    // Small eth in islandic looks like OE

{"ntilde", 'ï¿½}, // small n tilde       
{"ograve", 'ï¿½}, // small o grave accent
{"oacute", 'ï¿½}, // small o acute accent
{"ocirc", 'ï¿½},  // small o circumflex  
{"otilde", 'ï¿½}, // small o tilde       

{"ouml", 'ï¿½},   // small o umlaut      
{"divide", 'ï¿½}, // division character  
{"oslash", ''}, // small o with slash  
{"ugrave", ''}, // small u grave accent
{"uacute", ''}, // small u acute accent

{"ucirc", ''}, // small u circumflex    
{"uuml", ''},  // small u umlaut        
{"yacute", ''},// Capital Y acute accent
{"thorn", ''}, // small thorn, islandic 
{"yuml", ''},  // Capital Y umlaut      
};

// Sort HTML Codes by ASCII values
void SortASCII2HTML()
{
	int i;
	UCHAR c, cStr[HTMLCODE_SZ];

	for(i=0;i<HTMLCODES-1;i++)
	{
		if(HCodes[i].cLetter > HCodes[i+1].cLetter)
		{
			scpy(            cStr, HCodes[i+1].Code);
			scpy(HCodes[i+1].Code, HCodes[i  ].Code);
			scpy(HCodes[i  ].Code, cStr            );

			c                   = HCodes[i].cLetter;
			HCodes[i].cLetter   = HCodes[i+1].cLetter;
			HCodes[i+1].cLetter = c;
			
			i=0;
		}
	}

// for(i=0;i<HTMLCODES;i++)
// {
//	printf("{'%c', \"%s\"},\n", HCodes[i].cLetter, HCodes[i].Code);
// }
}

// Expand ASCII into HTML Strings
// 1>  Call SortASCII2HTML first
// 2>  Caller must free returned string
UCHAR *ExpandString2HTML(UCHAR *pcIn)
{
	UCHAR *pcRet=NULL, i,j,k,l=0;

	if(pcIn != NULL)
	{
		j = slen(pcIn);
		pcRet = malloc((j+1)*HTMLCODE_SZ*sizeof(UCHAR));

		for(i=0;i<j;i++)
		{
			for(k=0;k<HTMLCODES && HCodes[k].cLetter < pcIn[i];k++) ;

// 			if(HCodes[k].cLetter == pcIn[i]) //Replace with HTML Code
//			{
//				pcRet[l++] = '&';
//
//				for(m=0;HCodes[k].Code[m] != '\0'; m++ )
//				{
//					pcRet[l++] = HCodes[k].Code[m];
//				}

//				pcRet[l++] = ';';
//			}
//			else
//			{
//				pcRet[l++] = pcIn[i];
//			}
		}

		pcRet[l++] = '\0';
	}

	return pcRet;
}



*/

namespace s
{
// Look for a character starting from right side
UCHAR *schr(UCHAR *pcStrIn, const UCHAR cLetter)
{
	UCHAR *pcStr;

	pcStr = pcStrIn + slen(pcStrIn)-1;

	while(cLetter != *pcStr && pcStr != pcStrIn) pcStr--;

	if(*pcStr == cLetter)
		return pcStr;
	else
		return NULL;
}

// Find the directory delimiter character and return remaining string
CUSTR sGetFileName(const CUSTR & sVocIndex)
{
	CUSTR sRet(sVocIndex);

	int iLen = sVocIndex.Right(DIR_DELIMITER);

	if(iLen>0)
		sRet = sVocIndex.Mid(iLen+1);

	return sRet;
}

CUSTR sGetPath(const CUSTR & sVocIndex)
{
	CUSTR sRet(sVocIndex);
	int iLen;

	iLen = sVocIndex.Right(DIR_DELIMITER);

	if(iLen>0)
		sRet = sVocIndex.Mid(0,iLen+1);

	return sRet;
}

char *sdup(const char *s)
{
	int  i= s::slen(s)+1;
	char *pcRet;

	if((pcRet = new char[i])!= NULL) memcpy(pcRet, s, i);

	return pcRet;
}

//
//Make a copy of of n character and return newly allocated memory
//===============================================================
char *sdupn(const char *s, const int iLen)
{
	char *pcRet;

	if((pcRet = new char[iLen])!= NULL)
		memcpy(pcRet, s, iLen);

	return pcRet;
}

int slen(const char *s1) // standard slen function
{
	if(s1 == NULL) return 0;

	int len =0;
 
	if(s1 != NULL)
	{
		while( *s1 != '\0')
		{
			++len;
			++s1;
		}
	}

	return len;
}
	
// These are the sets that make up words in most definitions
// see chartab.h for definition of these
//--------------------------------------------------------->
#define CAP_A_WGRAVE 192 //LATIN CAPITAL LETTER A WITH GRAVE
#define SML_A_WGRAVE 225 //LATIN SMALL LETTER A WITH ACUTE
#define SML_O_WDIARS 246 //LATIN SMALL LETTER O WITH DIAERESIS
#define CAP_O_WDIARS 214 //LATIN SMALL LETTER O WITH DIAERESIS
#define SML_U_WGRAVE 249 //LATIN SMALL LETTER U WITH GRAVE
#define CAP_U_WGRAVE 217 //LATIN CAPITAL LETTER U WITH GRAVE
#define SML_THORN    254 //LATIN SMALL LETTER THORN

UCHAR *sToLower(UCHAR *pc)
{
	UCHAR *sRet;

	sRet = pc;

	while( *pc != '\0')
	{
		if(*pc >= (UCHAR)'A' && *pc <= (UCHAR)'Z')
		{
			*pc += (UCHAR)'a' - (UCHAR)'A'; // This may not work on certain platforms
		}
		else
		{
			if(*pc >= ((UCHAR)CAP_A_WGRAVE) && *pc <= ((UCHAR)CAP_O_WDIARS))
			{
				*pc += (((UCHAR)SML_A_WGRAVE) - ((UCHAR)CAP_A_WGRAVE));
			}
			else
			{
				if(*pc >= ((UCHAR)CAP_U_WGRAVE) && *pc <= ((UCHAR)SML_THORN))
					*pc += ((UCHAR)SML_U_WGRAVE) - ((UCHAR)CAP_U_WGRAVE);
			}
		}

		pc++;
	}

	return sRet;
}

UCHAR cToLower(UCHAR c)
{
	if(c >= (UCHAR)'A' && c <= (UCHAR)'Z')
	{
		c += ((UCHAR)'a' - (UCHAR)'A');
	}
	else
	{
		if(c >= ((UCHAR)SML_A_WGRAVE) && c <= ((UCHAR)SML_O_WDIARS))
		{
			c += (((UCHAR)SML_A_WGRAVE) - ((UCHAR)CAP_A_WGRAVE));
		}
		else
		{
			if(c >= ((UCHAR)SML_U_WGRAVE) && c <= ((UCHAR)SML_THORN))
			{
				c += ((UCHAR)SML_U_WGRAVE) - ((UCHAR)CAP_U_WGRAVE);
			}
		}
	}
	return c;
}

//
//Casefolding is based on ISO-8859-1, you will need a different case-
//folder on sstems with different encodings...
//---------------------------------------------------------------->
UCHAR *sToUpper(UCHAR *pc)
{
	UCHAR *sRet;

	sRet = pc;

	while( *pc != '\0')
	{
		if(*pc >= (UCHAR)'a' && *pc <= (UCHAR)'z')
		{
			*pc -= ((UCHAR)'a' - (UCHAR)'A');
		}
		else
		{
			if(*pc >= ((UCHAR)SML_A_WGRAVE) && *pc <= ((UCHAR)SML_O_WDIARS))
			{
				*pc -= (((UCHAR)SML_A_WGRAVE) - ((UCHAR)CAP_A_WGRAVE));
			}
			else
			{
				if(*pc >= ((UCHAR)SML_U_WGRAVE) && *pc <= ((UCHAR)SML_THORN))
					*pc -= ((UCHAR)SML_U_WGRAVE) - ((UCHAR)CAP_U_WGRAVE);
			}
		}

		pc++;
	}

	return sRet;
}

UCHAR cToUpper(UCHAR c)
{
	if(c >= (UCHAR)'a' && c <= (UCHAR)'z')
	{
		c -= ((UCHAR)'a' - (UCHAR)'A'); // This may not work on certain platforms
	}
	else
	{
		if(c >= ((UCHAR)SML_A_WGRAVE) && c <= ((UCHAR)SML_O_WDIARS))
		{
			c -= (((UCHAR)SML_A_WGRAVE) - ((UCHAR)CAP_A_WGRAVE)); // This may not work on certain platforms
		}
		else
		{
			if(c >= ((UCHAR)SML_U_WGRAVE) && c <= ((UCHAR)SML_THORN))
				c -= ((UCHAR)SML_U_WGRAVE) - ((UCHAR)CAP_U_WGRAVE);
		}
	}
	return c;
}
#undef CAP_A_WGRAVE //LATIN CAPITAL LETTER A WITH GRAVE
#undef SML_A_WGRAVE //LATIN SMALL LETTER A WITH ACUTE
#undef SML_O_WDIARS //LATIN SMALL LETTER O WITH DIAERESIS
#undef SML_U_WGRAVE //LATIN SMALL LETTER U WITH GRAVE
#undef CAP_U_WGRAVE //LATIN CAPITAL LETTER U WITH GRAVE
#undef SML_THORN    //LATIN SMALL LETTER THORN
// ----------------------------- End of CASE-FOLDING -----------------------------

// Compare at most n characters from two strings
int scmp_n( const UCHAR *s1, const UCHAR *s2, int n)
{
	int iLen=0;

	if(n <= 0) return 0; //AT most zero characters matched :)
	n--;

	if(s1 == NULL || s2 == NULL)
	{ 
		if(s1 == NULL && s2 == NULL) return  0;  //NULL is equal NULL

		if(s2 == NULL) return 1; //NULL is unequal something '>'

		return -1;	             //Second string is not null but first is '<'
	}

	while( *s1 != '\0' && *s1 == *s2 && iLen < n)
	{
		s1++;
		s2++;
		iLen++;
	}

	return(*s1 - *s2);
}

//Standard slen function
int slen(const UCHAR *s1)
{
	if(s1 == NULL) return 0;

	int len =0;
 
	if(s1 != NULL)
	{
		while( *s1 != '\0')
		{
			++len;
			++s1;
		}
	}

	return len;
}

// compare two strings
int scmp( const UCHAR *s1, const UCHAR *s2 )
{
	if(s1 == NULL || s2 == NULL)
	{ 
		if(s1 == NULL && s2 == NULL) return  0;  //NULL is equal NULL

		if(s2 == NULL) return 1; //NULL is unequal something '>'

		return -1;	             //Second string is not null but first is '<'
	}

	while( *s1 != '\0' && *s1 == *s2 )
	{
		s1++;
		s2++;
	}

	return(*s1 - *s2);
}

// Compare two strings case insensitive
int scmp_ci( const UCHAR *s1, const UCHAR *s2 )
{
	while( *s1 != '\0' && cToLower(*s1) == cToLower(*s2))
	{
		s1++;
		s2++;
	}

	return(cToLower(*s1) - cToLower(*s2));
}

// This function assumes that s1,s2 and s3,s4 each are a pair of strings
// The goal is to compare both pairs and return a valid result that is useful
// comparison in sorting functions
// !!! We priorize comparing s1,s3 over s2,s4 !!!
// ================================================================================<<<
int scmp_ci2(const UCHAR *s1, const UCHAR *s2, const UCHAR *s3, const UCHAR *s4)
{
  int iCmp = scmp_ci( s1,s3 );

  if(iCmp != 0) return iCmp;

  return scmp_ci( s2,s4 );
}

// standard string copy function
UCHAR *scpy( UCHAR *s1, const UCHAR *s2 )
{
	UCHAR *s;

	if(s1 == NULL) return s1; //return NULL if dest pointer is NULL
	else
	{
		if(s2 == NULL) //Return empty string if source is NULL
		{              //and destination is NOT NULL
			*s1 = '\0';
			return s1;
		}
	}

	s = s1;

	while( *s2 != '\0')
	{
		*s1 = *s2;
		s1++;
		s2++;
	}
	*s1='\0';

	return(s);
}

UCHAR *sdup(const UCHAR *s)
{
	int  i= s::slen(s)+1;
	UCHAR *pcRet;

	if((pcRet = new UCHAR[i])!= NULL) memcpy(pcRet, s, i);

	return pcRet;
}

//
//Make a copy of of n character and return newly allocated memory
//===============================================================
UCHAR *sdupn(const UCHAR *s, const int iLen)
{
	UCHAR *pcRet;

	if((pcRet = new UCHAR[iLen+1])!= NULL)
	{
		memcpy(pcRet, s, iLen);
		pcRet[iLen] = '\0';
	}

	return pcRet;
}

CUSTR ParentDir(const CUSTR sCurrAbsDir)
{
	int iLen;
	CUSTR sRet;

	iLen = sCurrAbsDir.slen();

	if(iLen <= 2) return sRet; //Got nowhere to go back 2 :(

	iLen -=  2; //Search for next parent dir backslash

	while(iLen > 0 && sCurrAbsDir[iLen] != DIR_DELIMITER) iLen--;

	if(iLen <= 0) return sRet;

	sRet = sCurrAbsDir.Mid(0,iLen+1); //Get Path to parent and return

	return sRet;
}

// <=================================================================>
// Convert sRelP to absolut path (in relation to current working dir)
// <=================================================================>
CUSTR GetAbsPath(const CUSTR & sCurrAbsDir, const CUSTR & sRelP)
{
	CUSTR sRet(sCurrAbsDir);
	
	//Go one up in dir-tree
	const UCHAR pcOneUP[4] = {'.', '.', DIR_DELIMITER, '\0'};
  int iLen, i;

	if((iLen = sRelP.slen()) > 2)
	{
		if(s::scmp_n((const UCHAR *)sRelP, pcOneUP, 3)==0)
		{
			i = 0;

			do			
			{
				i += 3;
				sRet = ParentDir(sRet);
			}
			while(sRet.slen() > 3 &&
            s::scmp_n(((const UCHAR *)sRelP)+i, pcOneUP, 3)==0);

			//ERROR: Relative path not fully consumed so return abs path ;(
			if(s::scmp_n(((const UCHAR *)sRelP)+i, pcOneUP, 3)==0) return sCurrAbsDir;

			return(sRet + sRelP.Mid(i));
		}

		//Is this path realy relative or absolut???
		if(sRelP[2] != ':' && sRelP[0] != DIR_DELIMITER &&
			                    sRelP[1] != DIR_DELIMITER &&
		                      sRelP[2] != DIR_DELIMITER)
		{
			int i=0;

			while(sRelP.slen() - i > 2) //Get Parent Dir in sRet for each "..\" in sRelP
			{
				if(sRelP[0] == '.' && sRelP[1] == '.' && sRelP[2] == DIR_DELIMITER)
				{
					sRet = ParentDir(sRet);

					i += 3;
				}
				else break;
			}

			if(i>0)
				sRet += sRelP.Mid(i);
			else
			{
				if(sRelP[0] == DIR_DELIMITER)
					sRet += sRelP.Mid(1);                // RelPath = '\'
				else
					if(sRelP[0] == '.' && sRelP[1] == DIR_DELIMITER)
						sRet += sRelP.Mid(2);             // RelPath = '.\'
					else
						sRet += sRelP;                // RelPath = 'xyz\'
			}

			return sRet;
		}
		else
			return CUSTR(sRelP); //RelPath is not relative so we return it (as absolut path)
	}
	else
	{
		if(sRelP.slen() <= 1) return CUSTR(sCurrAbsDir);

		if(sRelP[0] == '.')
		{
			if(sRelP[1] == DIR_DELIMITER) return CUSTR(sCurrAbsDir); // RelPath = '.\'
			if(sRelP[1] == '.') return ParentDir(sCurrAbsDir);       // RelPath = '..'
		}			
	}

	return CUSTR(sRelP); //Best answer under given circums...	
}

CUSTR BSlash2ParentDir(CUSTR & sTmp)
{
	int i,j;
	CUSTR sRet;
	const UCHAR pcOneUP[4] = {'.', '.', DIR_DELIMITER, '\0'}; //Go one up in dir-tree

	int iLen = sTmp.slen();

	if(iLen == 0) return sRet;

	for(i=0,j=0;i < iLen;i++)
	{
		if(sTmp[i] == DIR_DELIMITER) j++; //Count levels of 'go back' by number of Deli's
	}

	//At least one level to go back in any case...
	//(Last backslash might have been missing)
	if(j == 0) j++;

	for(i=0;i<j;i++)
	{
		sRet += pcOneUP;   //On UNIX this is sRet += "../"
	}                   //and on Windows: sRet += "..\"

	return sRet;
}

// ==============================================  Relative Path function  +++++
#ifdef WIN32                             //On Windows we require another include
	#include<direct.h>                     //and _getcwd instead of getcwd
	#define getcwd(_x, _y) _getcwd( _x, _y )
#else
	#define _MAX_PATH 1024
#endif

//Get current working directory wrapped in a string
//-------------------------------------------------
CUSTR sGetCWD()
{
	char buffer[_MAX_PATH+2];
	CUSTR sBuffer;

	//Get the current working directory: -> Return absolute path on error
	if( getcwd( buffer, _MAX_PATH ) == NULL ) return sBuffer;

	sBuffer = buffer;

	if(sBuffer.slen()>0)
	if(sBuffer[sBuffer.slen()-1] != DIR_DELIMITER) sBuffer +=(UCHAR)DIR_DELIMITER;

	return sBuffer;
}

//Compute a relative path pointing from sOutPutDir to sPath
//----------------------------------------------------------->
CUSTR GetRelPath(const CUSTR & sOutPutDir, const CUSTR & sPath)
{
	char buffer[_MAX_PATH+2];
	CUSTR sAbsPath, sAbsOutDir, sTmp, sBuffer;
	int i;

	//Get the current working directory: -> Return absolute path on error
	if( getcwd( buffer, _MAX_PATH ) == NULL ) return CUSTR(sPath);

	sBuffer = buffer;

	if(sBuffer.slen()>0)                            //Make sure path is terminated
	if(sBuffer[sBuffer.slen()-1] != DIR_DELIMITER) sBuffer +=(UCHAR)DIR_DELIMITER;

	sAbsPath   = GetAbsPath(sBuffer, sPath);
	sAbsOutDir = GetAbsPath(sBuffer, sOutPutDir);

	if(sAbsPath.slen() < 2 || sAbsOutDir.slen() < 2)
		return CUSTR(sPath); //Handle exceptions without surfacing errors

	//if Diff == 0 return pcAbsPath
	//Otherwise Find Spot with difference and fill remainders in pcAbsOutDir with '..\'
	//and return new relative path
	if(sAbsPath[0] != sAbsOutDir[0] || sAbsPath[1] != sAbsOutDir[1])
		return sAbsPath; //No relative path available :(

	//Both paths point to the same directory - Relative Path is EMPTY
	if(sAbsPath.scmp_ci(sAbsOutDir) == 0) return CUSTR("");

	int jLen = sAbsPath.slen();

	//Find position of in-equality between both path descriptors
  for(i=0;i<jLen && s::cToLower(sAbsPath[i]) == s::cToLower(sAbsOutDir[i]); i++) ;

	if(i == sAbsOutDir.slen()) //sAbsPath is a sub-directory of pcAbsOutDir
	{
		sAbsOutDir = sAbsPath.Mid(i);
		return sAbsOutDir.Replace('/', '\\'); //Convert to web slash's syntax
	}

  //inequality on dir-delimiter, then move back one level
  if(sAbsPath[i] == DIR_DELIMITER && i > 3) i--;

	//Now move back to last dir_limiter or beginning of path and work from there
	for( ;i>0 && sAbsPath[i] != DIR_DELIMITER; i--) ;

	if(sAbsPath[i] == DIR_DELIMITER) i++;

	//Get Backward "..\" for remainder in sAbsOutDir[i]
	sTmp = sAbsOutDir.Mid(i);

  //sAbsPath is a clean sub-directory in sAbsOutdir, so we return that part
  if(sTmp.slen() <= 0 || sTmp.Left(DIR_DELIMITER) <= 0)
  {
    sTmp = sAbsPath.Mid(i);
    return sTmp.Replace('/', '\\');
  }

  //Get backward relative path and link w/ remainder
	sAbsOutDir = BSlash2ParentDir(sTmp);

	sAbsOutDir += sAbsPath.Mid(i);

	//Convert to web slash's syntax and return relative path for hyperlinks(aechz)
	return sAbsOutDir.Replace('/', '\\');
}
// ============================================== Relative Path function +++++

// Create a file name from input parameters
// <=========================================>
CUSTR CreateFileName(const char *pcFPath, const char *pcFName, const char *pcExt)
{
	CUSTR sRet;

	if(pcFPath!=NULL) sRet += pcFPath;

	if(sRet[sRet.slen()-1] != DIR_DELIMITER && sRet[sRet.slen()-1] != WEB_DIR_DEL)
		sRet += (UCHAR)DIR_DELIMITER;

	if(pcFName!=NULL) sRet += pcFName;

	if(pcExt!=NULL) sRet += pcExt;

	return sRet;
}

//Trim off leading and ending control sequences
//Use this only with dynamically allocated strings, because we will
//free the current string and return a new copy, if required
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>°°°°°°°°°
UCHAR *sTrim(UCHAR *pcIn)
{
	UCHAR *pcPos, *pcPos1 = pcIn, *pcTmp;
	long i, x;
	
	if(pcIn != NULL)
	{
		if(*pcIn == '\0') return pcIn;

		while(*pcPos1 != '\0' && *pcPos1 < '!')
			pcPos1++;

	if(*pcPos1 != '\0')
	{
		pcPos = pcPos1 + slen(pcPos1) - 1;

    if(*pcPos < '!') //Trimming string from right side?
    {
		  while(pcPos1 < pcPos && *pcPos < '!')
			  pcPos--;

		  pcPos[1] = '\0';
      if(pcIn == pcPos1) return pcIn; //String was trimmed only at its end
    }
    else
    {
      if(pcIn == pcPos1) return pcIn; //Nothing to trim found
    }
	}
	else
		pcPos = pcPos1;

		// String contained no printable words
		if((pcPos1 == pcPos && *pcPos < '!') || pcPos1 > pcPos)
		{
			free(pcIn);
			return NULL;
		}
		else
		{
      x = s::slen(pcPos1) + 1;

			if((pcTmp = (UCHAR *)malloc(sizeof(UCHAR) * (x + 1)))!= NULL)
			{
				for(i=0; i< x; i++)
					pcTmp[i] = pcPos1[i]; //Copy string INCLUDING '\0' Terminator

				free(pcIn);
				pcIn = pcTmp;
			}
		}
	}

	return pcIn;
}

//Trim off leading, ending, and inside string control sequences
//Use this only with dynamically allocated strings, because we will
//free the current string and return a new copy, if required
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>°°°°°°°°°
UCHAR *sTrimAllWhiteSpaces(UCHAR *pcIn)
{
	if(pcIn    == NULL) return pcIn;
	if(pcIn[0] == '\0') return pcIn;

	//First: Count number of letters to copy from here to new string
	int iLen, iNoneWhite;
	for(iLen=0, iNoneWhite=0; pcIn[iLen] != '\0';iLen++)
	{
		if(pcIn[iLen] > ' ') iNoneWhite++;
	}
	//Second: Copy from here without witespaces to new string
	UCHAR *pcRet = new UCHAR[iNoneWhite+1];

	for(iLen=0, iNoneWhite=0; pcIn[iLen] != '\0';iLen++)
	{
		if(pcIn[iLen] > ' ') pcRet[iNoneWhite++] = pcIn[iLen];
	}

	pcRet[iNoneWhite] = '\0';
	delete [] pcIn;

	return pcRet;
}

}// <<<<<<<<<<<<<<<<<<< END OF NAMESPACE s >>>>>>>>>>>>>>>>
