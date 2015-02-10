
#include "cdtd.h"
#include "chartab.h"
#include "cerr.h"

UCHAR
	_DTD_REF1[] = "amp", _DTD_RPL1[] = "&",    //Initialize trie from std entities
	_DTD_REF2[] = "lt", _DTD_RPL2[] = "<",
	_DTD_REF3[] = "gt", _DTD_RPL3[] = ">",
	_DTD_REF4[] = "quot", _DTD_RPL4[] = "\"",
	_DTD_REF5[] = "apos", _DTD_RPL5[] = "\'";

#define STD_REFSSZ 5                         //Table of string pointers
HCODE STD_REFS[STD_REFSSZ] =
{
	{_DTD_REF1, _DTD_RPL1},{_DTD_REF2, _DTD_RPL2}, {_DTD_REF3, _DTD_RPL3},
	{_DTD_REF4, _DTD_RPL4},{_DTD_REF5, _DTD_RPL5}
};

//Construct DTD object
//
CDTD::CDTD(): iRefMaxSz(0)
{
	UINT iCnt;
	//Initialize references from standard table
	ptrRooRefs = trInitFromTable(STD_REFS, STD_REFSSZ, &iRefMaxSz, iCnt);
}
#undef STD_REFSSZ

//Destruct DTD object
//
CDTD::~CDTD()
{
	trFreeTree(ptrRooRefs); //Destroy trie of references
}

const UCHAR _DTD_SCOM[] = "!--";

//This will process a DTD tag from CSTRM
//The Function assumes that '<' is already consumed
//Comments are ignored (function will pass back NULL)
//
#define E_LOC "CDTD::ProcessDTDTag"
CTAG *CDTD::ProcessDTDTag(CSTRM *pxsInp)
{
	int iCnt;
	UCHAR cRd;

	for(pxsInp->get(cRd), iCnt=0;
	    cRd != '>' && pxsInp->eof() == false && iCnt < pxsInp->igetBufSz(); pxsInp->get(cRd))
	{
		pxsInp->pcBuf[iCnt++] = cRd;

		if(iCnt == 3) //Test and Skip comment if there is one...
		{
			if(s::scmp_n(pxsInp->pcBuf, _DTD_SCOM, 3) == 0)
			{
				pxsInp->SkipComment();
				return NULL;
			}
		}
	}

	//Check Syntax seen this far
	if(iCnt >= pxsInp->igetBufSz()) pxsInp->throwErr(E_LOC, ERR_UNEXP_EOF);
	if(cRd  != '>')                 pxsInp->throwErr(E_LOC, ERR_SYNTAX);

	pxsInp->pcBuf[iCnt] = '\0';

	CTAG *pTag = new CTAG(pxsInp->pcBuf);

	if(pTag != NULL) //DTD Tags must have some sort of content
	{
		if(pTag->iContsSz() <= 0 && pTag->Props.Size() <= 0)
			pxsInp->throwErr(E_LOC,ERR_SYNTAX);
	}

	return pTag;
}
#undef E_LOC

//
//Process Entities for !DOCTYPE XML Tag directly from input stream
//Parse internal doctype declaration and process entity declarations
//
//This code assumes that the DOCTYPE decl was consumed
//up to the first '[' character
//
#define E_LOC "CDTD::ParseDocType"
void CDTD::ParseDocType(const RDT & rdtType, CSTRM *pxsInp)
{
	UCHAR cRd;

	while(pxsInp->eof() == false)
	{
		//Skip white spaces
		for(pxsInp->get(cRd); pxsInp->eof() == false && ISWHITE(cRd); pxsInp->get(cRd)) ;

		if(pxsInp->eof() == true)
		{
			if(rdtType == RDT_EXTERNAL) return;

			pxsInp->throwErr(E_LOC,ERR_UNEXP_EOF);
		}

		if(rdtType == RDT_INTERNAL) //Test for END OF Internal DTD Tag
		{
			if(cRd == ']')
			{
				for(pxsInp->get(cRd); pxsInp->eof() == false && ISWHITE(cRd); pxsInp->get(cRd)) ;

				if(cRd == '>') return; //EO_TAG found

				pxsInp->throwErr(E_LOC,ERR_SYNTAX);
			}
		}

		//An XML tag should start right here!?
		if(cRd != '<') pxsInp->throwErr(E_LOC,ERR_SYNTAX);

		CTAG *pRwTag = ProcessDTDTag(pxsInp);

		if(pRwTag == NULL) continue; //Nothing found, might have been a comment???

		//An entity needs to have exactly one property (name and value)
		if(pRwTag->Props.Size() != 1) pxsInp->throwErr(E_LOC,ERR_SYNTAX);
		
		switch(pRwTag->getTagTyp())
		{
			case TGP_ENTITY_GE:             //Insert entity into reference TRIE
			{
				CUSTR sName, sVal;
				pRwTag->Props.GetProp(0, sName, sVal);
				
				//An entity ref can have refs itself, so we resolve this first
				//and insert processed ref below
				if(ResolveRefs(sVal) < 0) pxsInp->throwErr(E_LOC,ERR_SYNTAX);

				trAddEntry(sName, sVal, &ptrRooRefs, &iRefMaxSz);
			}
			break;

			case TGP_PI:      //Ignore these tags
			case TGP_ELEMENT:
			case TGP_ATTLIST:
			case TGP_NOTATION:
	
			case TGP_ENTITY:
			case TGP_ENTITY_PE:
			break;

			default:         //Unknown tagType! Throw an error
				delete pRwTag;
				pxsInp->throwErr(E_LOC,ERR_SYNTAX);
		}
		delete pRwTag;
	}
}
#undef E_LOC

//Document Declaration was found
//*consume this decl and return state to caller
//Process !DOCTYPE XML Tags directly from input stream
//Up to here the !DOCTYPE string is consumed on input
//
#define E_LOC "CDTD::ParseDOCTYPEDecl"
void CDTD::ParseDOCTYPEDecl(CSTRM *pxsInp)
{
	int iCnt=0, iMatch;
	UCHAR cRd;
	MSTATE mtKey;

	//Skip white spaces
	for(pxsInp->get(cRd), iCnt=0; pxsInp->eof() == false && (ISWHITE(cRd));
	    pxsInp->get(cRd), iCnt++)
	;

	if(iCnt==0)                         //White spaces are required here!
		pxsInp->throwErr(E_LOC, ERR_SYNTAX);

	if((iCnt = pxsInp->ReadName(cRd)) < 0)
		pxsInp->throwErr(E_LOC, ERR_SYNTAX);

	CUSTR sDocName = pxsInp->pcBuf;

	//Skip white spaces
	for(iCnt=0; pxsInp->eof() == false && ISWHITE(cRd); pxsInp->get(cRd), iCnt++) ;

	if(cRd == '[') //Parse internal doctype and return
	{
		ParseDocType(RDT_INTERNAL, pxsInp);
		return;
	}

	if(cRd == '>') return; //Empty DOCTYPES are allowed!

	//Find next keyword
	for(iMatch = MST_MATCH,iCnt=0;
	    pxsInp->eof() == false && iMatch == MST_MATCH; pxsInp->get(cRd), iCnt++)
		iMatch = FindKeyWord(REFERNCES, cRd, REFERNCECNT, REFERNCESZ, mtKey);	

	if(iMatch <= 0) pxsInp->throwErr(E_LOC, ERR_SYNTAX); //Did we find a valid match

	if(NOWHITE(cRd) || pxsInp->eof() == true) //Did we find a limiting Whitespace ???
		pxsInp->throwErr(E_LOC, ERR_SYNTAX);

	UCHAR cLim;     //Character for string literal limiter

	switch(iMatch)
	{
		case 1: //We found a PUBLIC entry -> Skip this unseen
			for(pxsInp->get(cRd), iCnt=0; pxsInp->eof() == false && cRd != '>';
			    pxsInp->get(cRd), iCnt++) ;

			pxsInp->throwErr(E_LOC, ERR_SYNTAX); //TODO: Expand here
		break;

		case 2: //SYSTEM entry -> Get literal and extract entities
			//Skip white spaces
			for( ; pxsInp->eof() == false && ISWHITE(cRd); pxsInp->get(cRd)) ;
			
			if(cRd != '\"' && cRd != '\'') pxsInp->throwErr(E_LOC, ERR_SYNTAX);

			cLim = cRd;
			pxsInp->ReadLiteral(cLim);
			
			if(cLim != cRd)
				pxsInp->throwErr(E_LOC, ERR_SYNTAX); //End limiter found???

			{
				CSTRM dtExtern;
				try //Open second CSTRM object and share buffer with pxsInp
				{
					dtExtern.open((char *)pxsInp->pcBuf, pxsInp->getPath(),
												pxsInp->pcBuf,pxsInp->igetBufSz());
				}
				catch(CERR) //Catch File Open Error and throw better description
				{
					pxsInp->throwErr(E_LOC,ERR_FILEOPEN);
				}

				ParseDocType(RDT_EXTERNAL, &dtExtern);  //Parse external DocType
			}
			
			//Get next none white space char
			for(cRd=' ' ; pxsInp->eof() == false && ISWHITE(cRd); pxsInp->get(cRd)) ;

			if(pxsInp->eof() == true) pxsInp->throwErr(E_LOC,ERR_UNEXP_EOF);

			if(cRd == '>') return; //Found end of tag (EOT)

			if(cRd == '[') //Parse internal doctype and return
			{
				ParseDocType(RDT_INTERNAL, pxsInp);
				return;
			}

			pxsInp->throwErr(E_LOC, ERR_SYNTAX);   //Something's wrong here!?
		break;

		default:              //Neither PUBLIC nor SYSTEM found :(
			pxsInp->throwErr(E_LOC, ERR_SYNTAX);  //Return Syntax error
	}
}
#undef E_LOC
#undef REFERNCESZ
#undef REFERNCECNT

//Resolve a reference to a string item delimited by '&' and ';'
//
#define E_LOC "CDTD::ResolveRefs"
int CDTD::ResolveRefs(CUSTR & s)
{
	int iLen = s.slen(), iBeg=0, idx;
	UCHAR c;

	//Process string and replace references
	for(;iLen >0 && iBeg < iLen; iBeg++)
	{
		if(s[iBeg] == '&')
		{
			int iCodeLen;
			//Find end of coding sequence
			for(iCodeLen=iBeg;iCodeLen < iLen && s[iCodeLen] != ';'; iCodeLen++) ;

			if(iCodeLen < iBeg+2)  return -1; //Sanity checks
			if(s[iCodeLen] != ';') return -1; //No terminator found???

			if(iBeg+2 >= iLen) return -1;

			if(s[iBeg+1] == '#')
			{
				idx = iBeg+2;
				PC_TYPE ptType;

				if((idx+2) > iLen)
					return -1; //Check minimal length

				if(s[idx] == 'x')
				{
					idx++;
					ptType = PCT_HEX;
				}
				else
					ptType = PCT_DEC;
				
				if((c = pc2i(s.SubString(idx), ptType, iCodeLen-idx))==0) return 0;

				//Now insert new code at iBeg + iCodeLen
				s.replaceAt(iBeg, iCodeLen-iBeg, c);
			}
			else //This is an other than numeric reference
			{
				const UCHAR *pc = trFindString(s.SubString(iBeg+1), ptrRooRefs, (iCodeLen-1)-iBeg);

				if(pc == NULL) return -1;

				//Now insert new code at iBeg + iCodeLen
				s.replaceAt(iBeg, iCodeLen-iBeg, pc);
			}

			iLen = s.slen(); //Update string length
		}
	}

	return 0;
}
#undef E_LOC
