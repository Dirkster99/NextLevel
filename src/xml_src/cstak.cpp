
#include "cstak.h"
#include "cerr.h"

//Standard ctor
CSTAK::CSTAK() : pstRoot(NULL), pstLast(NULL), bUsed(false),
								 ppMinorSTRdefs(NULL), iMinorSTRdefsSz(0), iMinorSTRdefsActSz(0)
{}

void CSTAK::ResetStack()
{
	ST_STAK *pTmp, *pThis = pstRoot; //Destroy linked list (stack of tags)

	while(pThis != NULL)
	{
		pTmp = pThis->pNext;
		delete pThis;
		pThis = pTmp;
	}

	//Invalidad stack root and minor state engine
	//(Major state is implicitely destroyed with stack list)
	pstRoot = pstLast = NULL;
}

void CSTAK::ResetMajorTRdefs()
{
	delete [] MajorSTRans.pTrans;     //Free state transitions
	MajorSTRans.pTrans = NULL;

	for(int i=0;i<MajorTRdefs.iDefSz;i++) //Free transfer definitions
	{
		delete [] MajorTRdefs.pTransdef[i].pcName;
	}
	delete [] MajorTRdefs.pTransdef;
	MajorTRdefs.pTransdef = NULL;
}

//Standard dtor
CSTAK::~CSTAK()
{
	ResetStack();
	ResetMajorTRdefs();    //Init Major Engine
	ResetMinorTRdefs();   //Init Minor Engine
}

//Push a new tag element on top of the stack
bool CSTAK::Push(CTAG & rt)
{
	bUsed = true;
	int iNxtState = STN_UNKNOWN, i, iMinorStateIdx=-1, iNxtMinorSt = STN_UNKNOWN;

	if(pstLast == NULL)
	{
		iNxtState = GetNextState(rt, &MajorSTRans); //Get next state of XML parser

		if((pstLast = pstRoot = new ST_STAK)==NULL)
			return false;

		pstLast->pPrev = NULL;  //Link to none previous node
	}
	else
	{
		if(pstLast->iStateIdx == -1) //Browsing Major or Minor transitions ???
		{
			//Get next state of XML parser - We try major first and minor later
			iNxtState = GetNextState(rt, &MajorSTRans);

			if(iMinorSTRdefsActSz > 0)
			{
				for(i=0; i<iMinorSTRdefsActSz && iNxtMinorSt == STN_UNKNOWN;i++)
				{
					if(ppMinorSTRdefs[i]->iRootStateID == iNxtState)
					{
						iNxtMinorSt = GetNextState(rt, &MajorSTRans);
						break;
					}
				}

				if(iNxtMinorSt != STN_UNKNOWN)
				{
					iNxtState = iNxtMinorSt;
					iMinorStateIdx = i;
				}
			}
		}
		else //Browsing Minor transitions...
		{
			iNxtState      = GetNextState(rt, ppMinorSTRdefs[pstLast->iStateIdx]);
			iMinorStateIdx = pstLast->iStateIdx;

			if(iNxtState !=  STN_UNKNOWN)
			{
				pstLast->ptgBaseTag->ChildTag_Add(&rt); //Add child to last root tag
			}
		}

		if((pstLast->pNext = new ST_STAK)==NULL)
			return false;

		pstLast->pNext->pPrev      = pstLast;              //Link to previous node
		pstLast->pNext->ptgBaseTag = pstLast->ptgBaseTag;  //Copy base for CTAG root
		pstLast = pstLast->pNext;
	}

	pstLast->prwTag   = new CTAG(rt);  //Previous link is set above
	pstLast->iStateID = iNxtState;
	pstLast->pNext    = NULL;

	pstLast->iStateIdx = iMinorStateIdx; //Set Minor state indicators

	if(iMinorStateIdx != -1)
	{
		if(pstLast->ptgBaseTag == NULL)
			pstLast->ptgBaseTag = pstLast->prwTag;
	}
	else
		pstLast->ptgBaseTag = NULL;

	return true;
}

//Pop a tag element from top of the stack
#define E_LOC "CSTAK::Pop"
CTAG *CSTAK::Pop()
{
	CTAG *pRet=NULL;

	//Throw things if stack is empty and there's nothing to Pop
	if(this->pstLast == NULL)
		throw CERR(E_LOC, ERR_IDX_OOF_RANGE);

	pstLast = pstLast->pPrev; //Find previous node and return current
	
	if(pstLast != NULL) //Any nodes left in stack???
	{
		pRet                   = pstLast->pNext->prwTag;
		if(pstLast->pNext->iStateID == STN_UNKNOWN)
		{ delete pRet;
		  pRet=NULL;
		}

		pstLast->pNext->prwTag = NULL;
		delete pstLast->pNext;

		pstLast->pNext = NULL;
	}
	else
	{
		if(pstRoot != NULL) //Return Root node and empty stack clean
		{
			pRet            = pstRoot->prwTag;
			if(pstRoot->iStateID == STN_UNKNOWN)
			{ delete pRet;
			  pRet=NULL;
			}

			pstRoot->prwTag = NULL;
			delete pstRoot;

			pstRoot = NULL; //Stack is empty
		}
	}
	
	return pRet;
}
#undef E_LOC

#define E_LOC "CSTAK::Top"
CTAG & CSTAK::Top()
{
	//Throw things if stack is empty and there's nothing to show
	if(this->pstLast == NULL)
		throw CERR(E_LOC, ERR_IDX_OOF_RANGE);

	return *pstLast->prwTag;
}
#undef E_LOC

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<

//Initialize state engine from an array of known state strings
//and state-names (as integer) and an array of state transitions
//
//Define "interesting" tags and their "interesting" paths 
//=============================================================>
void CSTAK::InitMajorEngine(const STRANS *pTransIn, int iSzIn,
                            const TRANSDEF *pTdefIn, int iDefSzIn)
{
	ResetStack();
	ResetMajorTRdefs();
	ResetMinorTRdefs();   //Init Minor Engine as well

	if(pTransIn != NULL && iSzIn > 0) //Copy transitions and states
	{
		MajorSTRans.pTrans = new STRANS[iSzIn];

		for(int i=0;i<iSzIn;i++)
		{
			MajorSTRans.pTrans[i] = pTransIn[i];
		}
		MajorSTRans.iSz = iSzIn;
	}
	else
	{
		MajorSTRans.pTrans = NULL;
		MajorSTRans.iSz    = 0;
	}

	if(pTdefIn != NULL && iDefSzIn > 0) //Copy transfer definitions
	{
		if((MajorTRdefs.pTransdef = new TRANSDEF[iDefSzIn]) == NULL) return;

		for(int i=0;i<iDefSzIn;i++)
		{
			MajorTRdefs.pTransdef[i].iVal   = pTdefIn[i].iVal;
			MajorTRdefs.pTransdef[i].pcName = s::sdup(pTdefIn[i].pcName);
		}
		MajorTRdefs.iDefSz = iDefSzIn;
	}
	else
	{
		MajorTRdefs.pTransdef = NULL;
		MajorTRdefs.iDefSz    = 0;
	}
}

//============================================================>
//Determine the next state of the engine based on the current
//Input: Current State (Top node)      pstLast
//       next XML CTAG element         rt
//       Plus stats transition diagram ptrsTrans
//============================================================>
int CSTAK::GetNextState(CTAG & rt, STRANS_ROOT *ptrsTrans)
{
	//No state transitions or states, ergo no knowing :)
	if(ptrsTrans == NULL) return STN_UNKNOWN;
	if(ptrsTrans->pTrans == NULL || MajorTRdefs.pTransdef == NULL) return STN_UNKNOWN;

	//Always proceed from Unknown to unknown
	if(pstLast != NULL)
	{
		if(pstLast->iStateID == STN_UNKNOWN) return STN_UNKNOWN;
	}
	else //Stack is empty
	{
		if(rt.getTagTyp() != TGP_OPEN) return STN_UNKNOWN;
	}

	//Keep last state if this is not an open tag
	if(rt.getTagTyp() != TGP_OPEN) return pstLast->iStateID;

	int iStName; 	//Map open/close tag name into state id

	for(iStName=0; iStName<MajorTRdefs.iDefSz;iStName++)
	{
		if(s::scmp((UCHAR *)MajorTRdefs.pTransdef[iStName].pcName, rt.getTagName())==0)
		{
			break;
		}
	}

	//Translate unknown open/close tags into unknown state
	if(iStName < MajorTRdefs.iDefSz)           //Lock new state ID
		iStName = MajorTRdefs.pTransdef[iStName].iVal;
	else
		return STN_UNKNOWN;

	//Did we find the root state? Then return it, otherwise unknown
	//This worx, because XML is allowed to have NO MORE than one root tag
	if(pstLast == NULL)
	{
		if(iStName == ST_ROOT)
			return STN_ROOT;
		else
			return STN_UNKNOWN;
	}

	int k;

	//Now find a valid transition for the given State and Stack-State ID
	for(k=0;k<ptrsTrans->iSz;k++)
	{
		if(ptrsTrans->pTrans[k].Src.iStateID == pstLast->iStateID &&
			 ptrsTrans->pTrans[k].Dst.iName    == iStName)
		{

			 break;
		}
	}

	//Translate unknown transition into unknown state
	if(k >= ptrsTrans->iSz) return STN_UNKNOWN;

	return ptrsTrans->pTrans[k].Dst.iStateID; //Return new state ID
}

//=====================================================================>
//=====================================================================>
//=====================================================================>
//Get Rid of Minor State Engine
//=============================>
void CSTAK::ResetMinorTRdefs()
{
	if(ppMinorSTRdefs == NULL)
	{
		iMinorSTRdefsActSz = iMinorSTRdefsSz = 0;
		return;
	}
	else
	{
		if(iMinorSTRdefsSz == 0)
		{
			delete [] ppMinorSTRdefs;
			ppMinorSTRdefs     = NULL;
			iMinorSTRdefsActSz = 0;

			return;
		}
	}

	for(int i=0;i<iMinorSTRdefsActSz;i++)
	{
		delete [] ppMinorSTRdefs[i]->pTrans;
		delete ppMinorSTRdefs[i];
	}
	delete [] ppMinorSTRdefs;
	ppMinorSTRdefs = NULL;

	iMinorSTRdefsActSz = iMinorSTRdefsSz = 0;
}

//
//Copy transdef minor engine entry and return
//=================================================>
CSTAK::STRANS_ROOT *CSTAK::CreateTRDefSlot(const STRANS *pTransIn, //ptr 2 transitions
                                           const int iDefSzIn,     //size of transitions
                                           const int iRootStateID) //Root state for xml root
{
	STRANS_ROOT *pRet = new STRANS_ROOT();

	if(pRet == NULL) return pRet;

	if((pRet->pTrans = new STRANS[iDefSzIn])==NULL)
	{
		delete pRet;
		return NULL;
	}

	for(int i=0;i<iDefSzIn;i++) //Copy data and return
	{
		pRet->pTrans[i].Src = pTransIn[i].Src;
		pRet->pTrans[i].Dst = pTransIn[i].Dst;
		pRet->iRootStateID  = iRootStateID;
	}
	pRet->iSz = iDefSzIn;

	return pRet;
}

//
// Check whether 2 (major/minor or minor/minor) webs collide with their state trans'
//==================================================================================>
void CSTAK::CheckMinorMinor(const STRANS *pTransSrc, const int iDefSzSrc,
                            const STRANS *pTransDst, const int iDefSzDst, CUSTR & sErrSrc)
{
	int i,j;
	const UCHAR pcSrc[] = " Minor Engine STN for TransDef.Src No.:",
	             pcDst[] = " colides with Minor Engine STN TransDef.Src No.:",
	            pcDst1[] = " colides with Minor Engine STN TransDef.Dst No.:";


	for(j=0;j<iDefSzSrc;j++) //Check whether state nodes comply with minor web
	{
		for(i=0;i<iDefSzDst;i++)
		{
			if(pTransSrc[j].Src.iStateID == pTransDst[i].Dst.iStateID)
			{
				sErrSrc += pcSrc;
				sErrSrc += j;
				sErrSrc += pcDst;
				sErrSrc += i;

				throw CERR(sErrSrc, ERR_SYNTAX);
			}

			if(pTransSrc[j].Dst.iStateID == pTransDst[i].Dst.iStateID)
			{
				sErrSrc += pcSrc;
				sErrSrc += j;
				sErrSrc += pcDst1;
				sErrSrc += i;

				throw CERR(sErrSrc, ERR_SYNTAX);
			}
		}
	}
}

const UCHAR pcSrc[] = " Minor Engine STN for TransDef.Src No.:",
	           pcDst[] = " colides with Major Engine STN TransDef.Src No.:",
	          pcDst1[] = " colides with Major Engine STN TransDef.Dst No.:",
	          pcDst2[] = " has more than one base in TransDef.Dst No.:";
//
// Check whether 2 major/minor webs collide with their state transitions
//======================================================================>
int CSTAK::CheckMinorMajor(const STRANS *pTransMin, const int iDefSzMin,
                           const STRANS *pTransMaj, const int iDefSzMaj, CUSTR & sErrSrc)
{
	int i,j;
	int iFound = -1;

	for(j=0;j<iDefSzMin;j++) //Check whether state nodes comply with major web
	{
		for(i=0;i<iDefSzMaj;i++)
		{
			//Can we find a base in major for this minor web?
			if(pTransMaj[i].Dst.iStateID == pTransMin[0].Src.iStateID)
			{
				if(iFound == -1)
					iFound = pTransMin[0].Src.iStateID;
				else
				{
					sErrSrc += pcSrc;
					sErrSrc += j;
					sErrSrc += pcDst2;
					sErrSrc += i;

					throw CERR(sErrSrc, ERR_SYNTAX); //Minor web must have only ONE base
				}
			}

			if(pTransMin[j].Dst.iStateID == pTransMaj[i].Dst.iStateID)
			{
				sErrSrc += pcSrc;
				sErrSrc += j;
				sErrSrc += pcDst1;
				sErrSrc += i;

				throw CERR(sErrSrc, ERR_SYNTAX);
			}
		}
	}

	return iFound;
}

//
// Check validity of minor web
//===============================>
bool CSTAK::CheckMinorWeb(const STRANS *pTransIn, const int iDefSzIn, int *piMinorRoot)
{
	CUSTR s("CSTAK::CheckMinorWeb ME Def: "); s += iMinorSTRdefsActSz;
	int i,j;
	bool bFound, bFound1;

	if(pTransIn == NULL || iDefSzIn == 0) return false;

	//Check validity of minor web
	if(MajorSTRans.pTrans == NULL)
		throw CERR(s + "Minor Engine can not initialize without Major Engine Transistions", ERR_SYNTAX);

	if(MajorTRdefs.pTransdef == NULL)
		throw CERR(s + " Minor Engine can not initialize without Major Engine State Definitions", ERR_SYNTAX);

	for(j=0;j<iDefSzIn;j++) //Check whether states are connected to actual tags
	{
		bFound=false, bFound1=false;

		for(i=0; i<MajorTRdefs.iDefSz && (bFound == false || bFound1 == false); i++)
		{
			if(MajorTRdefs.pTransdef[i].iVal == pTransIn[j].Src.iName)
				bFound = true;

			if(MajorTRdefs.pTransdef[i].iVal == pTransIn[j].Dst.iName)
				bFound1 = true;
		}

		if(bFound == false || bFound1 == false)
		{
			if(bFound1 == false)
				s += " Minor Engine without state definition for TransDef.Dst No.:";
			else
				s += " Minor Engine without state definition for TransDef.Src No.:";

			s += j;

			throw CERR(s, ERR_SYNTAX);
		}
	}

	//Minor web may not have a root in major, so there's no point adding
	if((*piMinorRoot = CheckMinorMajor(pTransIn, iDefSzIn, MajorSTRans.pTrans, MajorSTRans.iSz,s)) < 0)
		return false;

	//Check new minor defs against minor defs added so far and throw things, if required
	for(i=0;i<iMinorSTRdefsActSz;i++)
	{
		s = "CSTAK::CheckMinorWeb ME Def: ";
		s += i;
		s += " ME Def: ";
		s += iMinorSTRdefsActSz;
		CheckMinorMinor(ppMinorSTRdefs[i]->pTrans, ppMinorSTRdefs[i]->iSz, pTransIn, iDefSzIn,s);
	}

	return true;
}

#define INIT_TRDEF_SZ 10
//
// Define "interesting" tags and their "interesting" children
// Return number of Minor Webs defined so far or Error/Warning < 0
//================================================================>
int CSTAK::InitMinorEngine(const STRANS *pTransIn, const int iDefSzIn)
{
	//This is the state where major and minor states are equal
	//So, we can come from the former and submerge in the latter
	//Naturally, there can be only one such state, because XML syntax forbides anything else
	int iRootKey = -1;

	//Minor web was not added because something was wrong with it
	//XML parser can still run and produce results, but not with this minor web
	if(CheckMinorWeb(pTransIn, iDefSzIn, &iRootKey) == false) return -5;

	if(ppMinorSTRdefs == NULL) //add this this to the collection of states
	{
		if((ppMinorSTRdefs = new STRANS_ROOT * [INIT_TRDEF_SZ])!=NULL)
		{
			iMinorSTRdefsSz    = INIT_TRDEF_SZ;
			iMinorSTRdefsActSz = 0;

			ppMinorSTRdefs[iMinorSTRdefsActSz++] = CreateTRDefSlot(pTransIn, iDefSzIn, iRootKey);
			return iMinorSTRdefsActSz;
		}
		else
			iMinorSTRdefsSz = iMinorSTRdefsActSz = 0;
	
		return -1;
	}

	//Need to allocate new array of pointers and grow from there...
	if(iMinorSTRdefsActSz >= iMinorSTRdefsSz)    //Grow array of objects
	{
		STRANS_ROOT **ppTmp;
	
		if((ppTmp = new STRANS_ROOT * [INIT_TRDEF_SZ + iMinorSTRdefsSz])==NULL)
			return -2; //Can't grow further :(

		//copy array of pointers and free current list
		for(int i=0;i<iMinorSTRdefsSz;i++) ppTmp[i] = ppMinorSTRdefs[i];

		delete [] ppMinorSTRdefs;                         //Get rid of current array of nodes
		ppMinorSTRdefs  = ppTmp;                          //Reset to new array
		iMinorSTRdefsSz = INIT_TRDEF_SZ + iMinorSTRdefsSz; //Reset size of new array
	}

	if(iMinorSTRdefsActSz < iMinorSTRdefsSz)       //Grow array of objects
		ppMinorSTRdefs[iMinorSTRdefsActSz++] = CreateTRDefSlot(pTransIn, iDefSzIn, iRootKey);
	else
		return -3;

	return iMinorSTRdefsActSz;
}

#undef INIT_TRDEF_SZ
