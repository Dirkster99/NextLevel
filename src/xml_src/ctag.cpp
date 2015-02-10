//
//Character tables and basic definitions
//

#include "ctag.h"
#include "cdtd.h"

#include "chartab.h"

//Class constructor
//
//The tag type is set by default to UNKNOWN
//
//The raw tag string will not be processed further
//if the caller sets this parameter to something else than UNKNOWN
//
//Caller should check m_TagConts len
//
CTAG::CTAG(const UCHAR *pcTAG, const ETAGTYP InTagTyp)
       : Props(), m_TagTyp(InTagTyp), m_TagName(""), m_TagConts(""),
			   ppChildren(NULL), iChildrenSz(0), iChildrenActSz(0), ptgParent(NULL)
{
	m_TagConts = pcTAG;
	m_TagTyp   = InTagTyp;

	if(m_TagTyp == TGP_UNKNOWN) //Post-process tag to extract name and type
	{
		int iLen = m_TagConts.slen();
		m_TagTyp = TGP_ERROR;         //Return parser error by default

		if(iLen <= 0) return;

		switch(m_TagConts[0]) //Process in dependence of first character
		{
			case '/': //Closing Tag
				m_TagTyp = ProcessClosTag();
				break;

			case '?': //Parse processing extractions and extract a name
				m_TagTyp = ProcessPI();
				break;

			case '!': //Could be DOCTYPE, NOTATION ...,(other) or comment tag
				m_TagTyp = ProcessInstruction();
				break;

			default:  //Process tags such as <SAMPLE> or <SAMPLE/>
				m_TagTyp = ProcessOpenClose();
				break;
		}
	}

	//Process remaining list of properties for valid types of tags
	if(m_TagConts.slen() > 0)
	{
		if(m_TagTyp == TGP_ENTITY)
		{
			if(ProcessEntity(m_TagConts) < 0)  //Process !ENTITY TAG
					m_TagTyp = TGP_ERROR;          //propagate error info
		}
		else
		{
			//Property parsing is supported for the following types of XML Tags
			if(m_TagTyp == TGP_OPEN     || m_TagTyp == TGP_CLOSE   ||
				 m_TagTyp == TGP_CLO_OPEN || m_TagTyp == TGP_PI      ||
				 m_TagTyp == TGP_ELEMENT  || m_TagTyp == TGP_ATTLIST)
			{
				if(ProcessPropList(m_TagConts) < 0) //Caller should check this and
					m_TagTyp = TGP_ERROR;             //propagate error info
			}
		}
	}
}

// Assignment operator
CTAG & CTAG::operator=(const CTAG & tagObj)
{
	m_TagTyp   = tagObj.m_TagTyp;
	m_TagName  = tagObj.m_TagName;
	m_TagConts = tagObj.m_TagConts;

	Props = tagObj.Props;

	CopyArray(tagObj); //Free children and get copy of children tags as well
	ptgParent = NULL;  //Equal operator invalidates relation to parent

	return *this;
}

// Assignment operator
CTAG & CTAG::operator=(const CTAG *pTag)
{
	if(pTag != NULL) *this = *pTag;

	return *this;
}

//Copy constructor
CTAG::CTAG(const CTAG & rt)
       : Props(), m_TagTyp(TGP_ERROR), m_TagName(""), m_TagConts(""),
			   ppChildren(NULL), iChildrenSz(0), iChildrenActSz(0), ptgParent(NULL)
{
	m_TagTyp   = rt.m_TagTyp;
	m_TagName  = rt.m_TagName;
	m_TagConts = rt.m_TagConts;

	Props = rt.Props;

	CopyArray(rt); //Get copy of children tags as well
	ptgParent = NULL; //Copy constructor invalidates relation to parent
}

//+-/*&$#@!+-/*&$#@!+-/*&$#@!>
//Standard destructor
//+-/*&$#@!+-/*&$#@!+-/*&$#@!>
CTAG::~CTAG()
{
	FreeChildren(); //Get rid of child tags
}

//Process string properties for Entity tags and add to internal property bag
//Return number of recognized properties
//
int CTAG::ProcessEntity(CUSTR & sProps)
{
	if(sProps.slen() <=0) return -1; //Requires more than nothing!

	if(sProps[0] == '%')
	{
		m_TagTyp = TGP_ENTITY_PE;        //Set appropriate tag type
		sProps   = sProps.Mid(1);
	}
	else
		m_TagTyp = TGP_ENTITY_GE;

	int iLen, iEnd, iBeg;

	if((iLen = sProps.slen()) <=0) return -1; //Requires more than nothing!

	iEnd=0;
	if(m_TagTyp == TGP_ENTITY_PE)
	{
		//skip whitespaces
		for( ;iEnd<iLen && ISWHITE(sProps[iEnd]); iEnd++) ;

		if(iEnd==0|| iEnd >=iLen) return -1; //White spaces are required here!
	}

	if((iBeg = MatchName(sProps, false, iEnd))<0) return -1;

	CUSTR sName, sVal;
	sProps.Split(iEnd, sName); //Extract Property Name
	iLen = sProps.slen();

	//skip whitespaces
	for(iEnd=0;iEnd<iLen && ISWHITE(sProps[iEnd]); iEnd++) ;

	if(iEnd==0 || iEnd >=iLen) return -1; //White spaces are required here!

	if(sProps[iEnd] == '\"' || sProps[iEnd] == '\'') //Get Entity Value
	{
		//There is a name value pair
		if((iBeg = MatchVal(sProps, false, iEnd))<0) return -1;

		
		sProps.Split(iBeg+1, iEnd-1, sVal); //Extract Property Value
		sProps = sProps.Mid(iEnd+1);       //Get rid of value
		iLen = sProps.slen();

		Props.AddProp(sName, sVal); //FINALY: Add Property Name - Value pair

		//skip whitespaces
		for(iEnd=0;iEnd<iLen && ISWHITE(sProps[iEnd]); iEnd++) ;

		if(iEnd<iLen) return -1; //There should be nothing (but whitespaces)
		                         //At the end of a PE Entity

		//Optional NDATA Section is also ignored if:
		//m_TagTyp == TGP_ENTITY_GE
		return 1;
	}		

	MSTATE mtKey;

	//This could be an ExternalID (SYSTEM or PUBLIC reference)
	//We check this but DO NOT process further
	for(iBeg = MST_MATCH; iEnd<iLen && iBeg == MST_MATCH; iEnd++)
		iBeg = FindKeyWord(REFERNCES, sProps[iEnd], REFERNCECNT, REFERNCESZ, mtKey);	

	if(iBeg != 1 && iBeg != 2) return -1;

	return 1; //Do not process SYSTEM or PUBLIC reference any further
}

//Process string properties and add to internal property bag
int CTAG::ProcessPropList(CUSTR & sProps)
{
	int iBeg, iEnd, iLen,iCnt=0;
	CUSTR sName, sVal;

	while((iLen = sProps.slen()) > 0)
	{
		//skip whitespaces
		for( iEnd=0;iEnd<iLen && ISWHITE(sProps[iEnd]); iEnd++) ;

		if(iEnd == iLen) //No properties available (only whitespaces)
		{
			sProps="";
			return iCnt;
		}
		sProps = sProps.Mid(iEnd); //Cut off trailing white spaces
		iEnd = 0;

		//There is at least one name value pair
		if((iBeg = MatchName(sProps, true, iEnd))<0) return -1;

		sProps.Split(iEnd, sName); //Extract Property Name
		iLen = sProps.slen();

		//skip whitespaces
		for(iEnd=0;iEnd<iLen && ISWHITE(sProps[iEnd]); iEnd++) ;

		//SYNTAX ERROR (Equal sign is required!)
		if(iEnd >= iLen || sProps[iEnd] != '=') return -1;

		iEnd++; // Skip Equal sign

		//skip whitespaces
		for(;iEnd<iLen && ISWHITE(sProps[iEnd]); iEnd++) ;
		
		//Nothing left, so there is no value for this property???
		if(iEnd >= iLen) return -1;

		sProps = sProps.Mid(iEnd); //Cut of equal character
		iEnd=0;

		//There is at least one name value pair
		if((iBeg = MatchVal(sProps, false, iEnd))<0) return -1;

		sProps.Split(iBeg+1, iEnd, sVal); //Extract Property Value
		sProps = sProps.Mid(iEnd+1);     //Get rid of value

		Props.AddProp(sName, sVal); //FINALY: Add Property Name - Value pair
	}

	return iCnt;
}

//Process tags such as <SAMPLE> or <SAMPLE/>
ETAGTYP CTAG::ProcessOpenClose()
{
	int idx, idxBeg=0, iLen = m_TagConts.slen();

	if((idx = MatchName(m_TagConts, false, idxBeg)) < 0) return TGP_ERROR;

	if(idxBeg - idx <= 0) return TGP_ERROR; //need at least one character

	m_TagName = m_TagConts;
	m_TagName = m_TagName.Mid(idx, idxBeg-idx);
	
	m_TagConts = m_TagConts.Mid(idxBeg);  //Cut off processed stuff
	if((iLen = m_TagConts.slen())<=0) return TGP_OPEN;

	//skip whitespaces
	for(idxBeg=0;idxBeg<iLen && ISWHITE(m_TagConts[idxBeg]); idxBeg++) ;

	if(idxBeg < 1) return TGP_ERROR; //Unrecognized character?!

	m_TagConts = m_TagConts.Mid(idxBeg);  //Cut off white spaces
	if((iLen = m_TagConts.slen())<=0) return TGP_OPEN;

	if(m_TagConts[iLen-1] == '/')
	{
		m_TagConts = m_TagConts.Mid(0,iLen-1); //Cut off slash and return
                              //property list plus TAGGIFIED ID
		return TGP_CLO_OPEN;
	}

	return TGP_OPEN;
}

//Process a close tag like </Start>
//
ETAGTYP CTAG::ProcessClosTag()
{
	int idxBeg=1, iLen = m_TagConts.slen();

	if(iLen < 2 || m_TagConts[0] != '/') return TGP_ERROR;

	int idx = MatchName(m_TagConts, false, idxBeg);

	if(idx < 0) return TGP_ERROR;

	m_TagName = m_TagConts;
	m_TagName = m_TagName.Mid(idx, idxBeg-idx);

	//idxStr shows where the matched keyword ends -> skip whitespaces too
	for( ;idxBeg<iLen && ISWHITE(m_TagConts[idxBeg]); idxBeg++) ;

	//String wasn't consumed completely so somethings wrong here...
	if(idxBeg != iLen) return TGP_ERROR;

	m_TagConts = "";

	return TGP_CLOSE; //Name is taken and type is returned :)
}

//This is sorted list of keywords for processing below
//
#define INSTR_KEYCNT 5
#define INSTR_KEYLEN 9
const UCHAR INSTR_KEYS[ ] = //Keep this in sync with KEY_TYPS below
{
	"ATTLIST  DOCTYPE  ELEMENT  ENTITY   NOTATION"
};

const ETAGTYP KEY_TYPS[INSTR_KEYCNT] = 
{TGP_ATTLIST, TGP_DOCTYPE, TGP_ELEMENT, TGP_ENTITY, TGP_NOTATION};

//Process tags that look like the following instructions:
//
//<!--       ... --> This is a comment in XML
//<!ATTLIST ...>
//<!DOCTYPE ... >
//<!ELEMENT greeting (#PCDATA)> ... ]>
//<!ENTITY ...>
//<!NOTATION ...>
//<![ .... ]>   ---> Include section (skip unseen)
//<![CDATA[ ]]> ---> Handle just like include section (skip unseen)
//
//
//The code assumes that outer '<' and '>' delimiters are stripped off
ETAGTYP CTAG::ProcessInstruction()
{
	int iLen = m_TagConts.slen();

	if(iLen < 5 || m_TagConts[0] != '!') return TGP_ERROR;

	if(m_TagConts[1] == '-' && m_TagConts[2] == '-' &&
		 m_TagConts[iLen-2] == '-' && m_TagConts[iLen-1] == '-')
	{
		m_TagConts = m_TagConts.Mid(3,iLen-5);   //This is a comment
		return TGP_COMMENT;         //So, we take off the formating parts
	}	                            //and return what we have

	if(m_TagConts[1] == '[' && m_TagConts[iLen-1] == ']')
	{
		m_TagConts = m_TagConts.Mid(2,iLen-4); //This is an include or CDATA section
		return TGP_INCL;            //So, we take off the formating parts
	}                             //and return what we have...

	//Now we look for one of the keywords such as NOTATION, ENTITY etc...
	int idxKey, idxStr=1;
	idxKey = FindKeyWord(INSTR_KEYS, m_TagConts, INSTR_KEYCNT, INSTR_KEYLEN, idxStr, UPLETTRSZ, UPLETTR);

	if(idxKey < 0 || idxKey >= INSTR_KEYCNT) return TGP_ERROR;

	//idxStr shows where the matched keyword ends -> skip whitespaces too
	for( ;idxStr<iLen && ISWHITE(m_TagConts[idxStr]); idxStr++) ;
		
	m_TagConts = m_TagConts.Mid(idxStr);
	m_TagTyp   = KEY_TYPS[idxKey];

	return m_TagTyp;
}

#undef INSTR_KEYCNT
#undef INSTR_KEYLEN

//Extract target of processing instruction into Name String Member
//and return Processing instruction type if everything's fine
//
ETAGTYP CTAG::ProcessPI()
{
	int i,iLen = m_TagConts.slen();

	if(iLen<=3) return TGP_ERROR; //not a valid processing instruction

	if(m_TagConts[0] != '?' && m_TagConts[iLen-1] != '?') return TGP_ERROR;

	for(i=1;i<iLen-1 && NOWHITE(m_TagConts[i]);i++) ; //Find next whitespace

	m_TagName = m_TagConts;
	m_TagName = m_TagName.Mid(1, i-1);

	for(;i<iLen-1 && ISWHITE(m_TagConts[i]);i++) ; //Find next non-whitespaces

	//Get rid of processed stuff
	m_TagConts = m_TagConts.Mid(i, iLen - (1+i));

	if(m_TagConts.slen() > 0) return TGP_PI; //Return processing instruction or error
	else
		return TGP_ERROR;
}

UCHAR STR_RAW_TAG_TYP[TGP_UNKNOWN+1][14] =
{
	{"TAGTYPE ERROR"},
	{"Open"},
	{"TXT Content"},
	{"Close"},
	{"CloseOpen"},
	{"PI"},
	{"Comment"},
	{"DOCTYPE"},

	//These are parsed rawly but not supported in syntax check
	{"ELEMENT"}, {"ATTLIST"}, {"NOTATION"},
	{"ENTITY"},{"ENTITY_PE"},{"ENTITY_GE"},
	{"INCL"},    //<![ .... ]>   -> Include section <![CDATA[ ]]>

	{"Unknown"} //This type must be last in array!!!
};

//
//Debug tag contents and state
void CTAG::DebugTag()
{
	int idx;

	if(m_TagTyp > 0 && m_TagTyp <= TGP_UNKNOWN)
		idx = m_TagTyp;
	else
		idx = TGP_ERROR;

	if(m_TagConts.slen() > 0)
		printf("TYP: %s - '%s'", STR_RAW_TAG_TYP[idx], (const char *)m_TagConts);
	else
		printf("TYP: %s - ''", STR_RAW_TAG_TYP[idx]);

	if(m_TagName.slen() > 0)
	{
		printf(" NAME: %s", (const char *)m_TagName);
	}

	printf("\n");

	if(Props.Size() > 0) Props.DebugProp(); //Print property contents

	for(idx=0;idx < iChildrenActSz;idx++)
		ppChildren[idx]->DebugTag();
}

//Resolve string references for tag content and property values
//
int CTAG::ResolveRefs(CDTD & dtd)
{
	switch(m_TagTyp)
	{
		case TGP_CONTENT:
			if(m_TagConts.slen()>0) dtd.ResolveRefs(m_TagConts);
			break;

		case TGP_OPEN:
		case TGP_CLO_OPEN:
		case TGP_PI:
				for(int i=0;i<Props.Size();i++)
					dtd.ResolveRefs(Props.sGetVal(i));
		break;
		default:
		break;
	}

	return 0;
}


#define INIT_TAGCHILDS_SZ 10

//TODO: Add support in copy constructor and equal operator
//
//Add a new child node and return index of insertion [0...n]
//=========================================================>
int CTAG::ChildTag_Add(const CTAG *pTag)
{
	if(ppChildren == NULL) //Init on Child addition
	{
		if((ppChildren = new CTAG * [INIT_TAGCHILDS_SZ])!=NULL)
		{
			iChildrenSz    = INIT_TAGCHILDS_SZ;
			iChildrenActSz = 1;

			if((ppChildren[0]  = new CTAG())!=NULL)
			{
				*ppChildren[0]  = pTag;
				ppChildren[0]->ptgParent = this; //Set parent relation-ship

				return iChildrenActSz;
			}
		}
		else
			iChildrenSz    = iChildrenActSz = 0;
	
		return -1;
	}

	//Need to allocate new array of pointers and grow from there...
	if(iChildrenActSz >= iChildrenSz)    //Grow array of objects
	{
		CTAG **ppTmp;
	
		if((ppTmp = new CTAG * [INIT_TAGCHILDS_SZ + iChildrenSz])==NULL)
			return -2; //Can't grow further :(

		//copy array of pointers and free current list
		for(int i=0;i<iChildrenSz;i++) ppTmp[i] = ppChildren[i];

		delete [] ppChildren;                          //Get rid of current array of nodes
		ppChildren  = ppTmp;                           //Reset to new array
		iChildrenSz = INIT_TAGCHILDS_SZ + iChildrenSz; //Reset size of new array
	}

	if(iChildrenActSz < iChildrenSz)       //Grow array of objects
	{
		if((ppChildren[iChildrenActSz] = new CTAG())!=NULL)
		{
			*ppChildren[iChildrenActSz]  = pTag;
			ppChildren[iChildrenActSz]->ptgParent = this; //Set parent relation-ship
			iChildrenActSz++;
		}
	}
	else
		return -3;

	return iChildrenActSz;
}
#undef INIT_TAGCHILDS_SZ

//Remove one Child node at this index
//(array will shrink by one and current indexes are invalid)
//=========================================================>
int CTAG::ChildTag_Remove(int idx)
{
	if(idx >= iChildrenActSz)       //Throw error if index is invalid
	{
		CUSTR s("ERROR: CTAG::ChildTag_Remove ");
		s+= idx;
		s+= "index not valid!";
		throw CERR(s);
	}

	delete ppChildren[idx];

	int i;

	//Reset all entries before idx
	for(i=0;i<idx;i++) ppChildren[i] = ppChildren[i+1];

	//Reset all entries after idx and decrement actually used size of array
	for(i=idx;i<iChildrenActSz;i++) ppChildren[i] = ppChildren[i+1];

	iChildrenActSz--;

	return 0;
}

// Access Array of tag children through index operator
//===================================================>
CTAG & CTAG::operator[](int idx)
{
	if(idx >= iChildrenActSz)       //Throw error if index is invalid
	{
		CUSTR s("ERROR: CTAG::operator (TAG NAME: ");
		s+= this->m_TagName;
		s+= ") Index:";
		s+= idx;
		s+= " Not Valid (Max:";
		s+= iChildrenActSz;
		s+= " )!";
		throw CERR(s);
	}

	return *ppChildren[idx];
}

