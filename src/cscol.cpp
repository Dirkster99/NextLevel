
#include "cscol.h"

CSCOL::CSCOL():psbRoot(NULL), iTreeSz(0),
               ppsbIterate(NULL), iIterateSz(0), iCurrIdx(0)
{
}

CSCOL & CSCOL::operator=(const CSCOL & scolObj)
{
	EmptyColl();

	CopyTree(scolObj.psbRoot);
	iTreeSz = scolObj.iTreeSz;

	return *this;
}

CSCOL & CSCOL::operator=(const CSCOL *pPtr)
{
	EmptyColl();
	if(pPtr != NULL) *this = *pPtr;

	return *this;
}

void CSCOL::CopyTree(const SBIN_TREE *pNode)
{
	if(pNode == NULL) return;

	if(pNode->psbLeft != NULL)CopyTree(pNode->psbLeft);

	this->AddString(pNode->sName, pNode->iFlags);

	if(pNode->psbRight != NULL) CopyTree(pNode->psbRight);
}

CSCOL::CSCOL(const CSCOL & pscIn):psbRoot(NULL), iTreeSz(0),
                            ppsbIterate(NULL), iIterateSz(0), iCurrIdx(0)
{
	CopyTree(pscIn.psbRoot);
	iTreeSz = pscIn.iTreeSz;
}


// --*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*<
// Next two functions free the binary tree of sorted strings
// This function is recursively called until tree is gone
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
void CSCOL::ResetTree(SBIN_TREE *pNode)
{
	if(pNode == NULL) return;

	if(pNode->psbLeft != NULL) ResetTree(pNode->psbLeft);
	if(pNode->psbRight != NULL) ResetTree(pNode->psbRight);

	//delete pNode->sName; sName is called through standard dtor
	delete pNode;
}

void CSCOL::ResetIterator()
{
	if(ppsbIterate != NULL) //Free iterator
	{
		delete [] ppsbIterate;
		ppsbIterate = NULL;
		iIterateSz = iCurrIdx = 0;
	}
}

void CSCOL::EmptyColl()
{
	ResetIterator();

	if(psbRoot != NULL)
	{
		ResetTree(psbRoot); //* Call this to free the binary tree
		psbRoot = NULL;
		iTreeSz = 0;
	}
}

CSCOL::~CSCOL(){ EmptyColl(); }

// --*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*<
// Create a single node of the binary tree
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
CSCOL::SBIN_TREE *CSCOL::CreateNode(const CUSTR & s, const int iFlags)
{
	SBIN_TREE *pNode = new SBIN_TREE;

	if(pNode != NULL)
	{
		pNode->psbLeft = pNode->psbRight = NULL;
		pNode->sName   = s;
		pNode->iFlags  = iFlags;
	}

	return pNode;
}

// --*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*<
// Add new node into binary tree
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
void CSCOL::AddString(const CUSTR & sKey, const int iFlags)
{
	SBIN_TREE *thisNode=psbRoot, *pNode1=NULL;
	int iCmp;

	/** Create Root Node and return **/
	if(psbRoot == NULL)
	{
		iTreeSz++;
		ResetIterator();
		psbRoot = CreateNode(sKey, iFlags);
		return;
	}

	while(thisNode->psbLeft != NULL || thisNode->psbRight != NULL)
	{
		iCmp = s::scmp_ci(sKey, thisNode->sName);

		if(iCmp == 0)
		{
			thisNode->iFlags |= iFlags;
			return; //** This string is already here
		}

		if(iCmp > 0)
		{
			if(thisNode->psbRight!= NULL)
			{
				//Key Goes beetween this node and the right child
				if(s::scmp_ci(sKey, thisNode->psbRight->sName) < 0)
				{
					iTreeSz++;
					ResetIterator();
					pNode1             = CreateNode(sKey, iFlags);
					pNode1->psbRight   = thisNode->psbRight;
					thisNode->psbRight = pNode1;

					return;
				}
				else
					thisNode = thisNode->psbRight;
			}
			else
				break;
		}
		else
		{
			if(thisNode->psbLeft != NULL)
			{
				// Key Goes beetween this node and the right child
				if(s::scmp_ci(sKey, thisNode->psbLeft->sName) > 0)
				{
					iTreeSz++;
					ResetIterator();
					pNode1            = CreateNode(sKey, iFlags);
					pNode1->psbLeft   = thisNode->psbLeft;
					thisNode->psbLeft = pNode1;

					return;
				}
				else
					thisNode = thisNode->psbLeft;
			}
			else
				break;
		}
	}

	if(s::scmp_ci(sKey, thisNode->sName)==0)
	{
		thisNode->iFlags |= iFlags;
		return;
	}

	/** Add new child on left or right side **/
	if(s::scmp_ci(sKey, thisNode->sName) > 0)
		thisNode->psbRight = CreateNode(sKey, iFlags);
	else
		thisNode->psbLeft = CreateNode(sKey, iFlags);

	iTreeSz++;
	ResetIterator();

	return;
}

// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// Look down this tree of calls and see whether sName is a Member or not
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
bool CSCOL::FindString(SBIN_TREE *pbnRoot, const CUSTR & sName)
{
	if(pbnRoot == NULL) return false;

	int iCmp;

	while(pbnRoot->psbLeft != NULL || pbnRoot->psbRight != NULL)
	{
		iCmp = s::scmp_ci(sName, pbnRoot->sName);

		/** This is a string match :) So, return it **/
		if(iCmp == 0) return true;

		if(iCmp > 0)
		{
			if(pbnRoot->psbRight != NULL) pbnRoot = pbnRoot->psbRight;
			else
				break;
		}
		else
		{
			if(pbnRoot->psbLeft != NULL) pbnRoot = pbnRoot->psbLeft;
			else
				break;
		}
	}

	if(pbnRoot != NULL)
	if(s::scmp_ci(sName, pbnRoot->sName) == 0)
		return true;

	return false;
}

// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// ** Print contents of binary tree in sorted order
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
void CSCOL::DebugCollection(const UCHAR *pcIntro, SBIN_TREE *pNode, FILE *fpOut)
{
	if(pNode == NULL) return;

	if(pNode->psbLeft != NULL) DebugCollection(pcIntro,pNode->psbLeft, fpOut);

	if(pcIntro != NULL)
		fprintf(fpOut, "%s %s\n", pcIntro, (const UCHAR *)pNode->sName);
	else
		fprintf(fpOut, "%s\n", (const UCHAR *)pNode->sName);

	if(pNode->psbRight != NULL) DebugCollection(pcIntro,pNode->psbRight, fpOut);
}

// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// We use an integer pointer to count the number of nodes  **
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
void CSCOL::CntTreeNodes(SBIN_TREE *pNode, int *ipCnt)
{
	if(pNode == NULL) return;

	if(pNode->psbLeft  != NULL){*ipCnt+=1; CntTreeNodes(pNode->psbLeft ,ipCnt);}
	if(pNode->psbRight != NULL){*ipCnt+=1; CntTreeNodes(pNode->psbRight,ipCnt);}
}

//Fill array for iteration return array of pointers
//=====================================================>
void CSCOL::FillIterator(SBIN_TREE **ppsbIterate,    //Array to fill
												 const int iSz,             //Size of Array
                         SBIN_TREE *psbRecurse,    //Tree to recurse on
												 int *pIdx)               //Index for filling array
{
	if(psbRecurse == NULL) return;

	if(psbRecurse->psbLeft  != NULL) FillIterator(ppsbIterate, iSz,psbRecurse->psbLeft, pIdx);

	if(*pIdx >= iSz)
	{
		delete [] ppsbIterate;
		ppsbIterate = NULL;
		iIterateSz = iCurrIdx = 0;

		throw CERR("ERROR: CSCOL::FillIterator: Tree/Array size for iterator in CSCOL corrupted!");
	}

	ppsbIterate[*pIdx] = psbRecurse;
	*pIdx += 1;

	if(psbRecurse->psbRight != NULL) FillIterator(ppsbIterate, iSz,psbRecurse->psbRight, pIdx);
}
//Iterator functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const UCHAR *CSCOL::GetFirst()
{
	if(this->ppsbIterate != NULL) //Free iterator
	{
		delete [] ppsbIterate;
		ppsbIterate = NULL;
		iIterateSz = iCurrIdx = 0;
	}

	if(iTreeSz > 0)
	{
		if((ppsbIterate = new SBIN_TREE *[iTreeSz]) == NULL) return NULL;

		iIterateSz = iTreeSz;
		int iCnt=0;
		FillIterator(ppsbIterate, iIterateSz, psbRoot, &iCnt);

		if(ppsbIterate == NULL) return NULL;

		return ppsbIterate[0]->sName; //Get first object in collection
	}

	return NULL;
}

const UCHAR *CSCOL::GetNext()
{
	if(ppsbIterate == NULL) return NULL;

	if(iCurrIdx < (iIterateSz-1))
	{
		iCurrIdx++;
		return ppsbIterate[iCurrIdx]->sName; //Get first object in collection
	}

	if(ppsbIterate != NULL) //Free iterator when last element has been accessed
	{
		delete [] ppsbIterate;
		ppsbIterate = NULL;
		iIterateSz = iCurrIdx = 0;
	}

	return NULL;
}

int CSCOL::itGetFlag()
{
	if(ppsbIterate == NULL || iCurrIdx >= iIterateSz)
	{
		CUSTR s("ERROR: CSCOL::itGetFlag Can not access current iterator!");

		throw CERR(s);
	};

	return ppsbIterate[iCurrIdx]->iFlags;
}

//
//Find a DBO_TREE node with pcKey as name, in other words:
//Find the main entry for a Procedure/Trigger called pcKey
//
CSCOL::SBIN_TREE *CSCOL::FindNode(SBIN_TREE *thisNode, const UCHAR *pcKey)
{
	int iCmp;

	if(thisNode == NULL) return NULL;

	while(thisNode->psbLeft != NULL || thisNode->psbRight != NULL)
	{
		iCmp = s::scmp_ci(pcKey, thisNode->sName);

		/** This is a string match :) So, return it **/
		if(iCmp == 0) return thisNode;

		if(iCmp > 0)
		{
			if(thisNode->psbRight != NULL) thisNode = thisNode->psbRight;
			else
				break;
		}
		else
		{
			if(thisNode->psbLeft != NULL) thisNode = thisNode->psbLeft;
			else
				break;
		}
	}

	if(thisNode != NULL)
		if(s::scmp_ci(pcKey, thisNode->sName) == 0)
		return thisNode;

	return NULL;
}

int CSCOL::itGetFlag(const UCHAR *pc)
{
	SBIN_TREE *pRet;

	if((pRet = FindNode(this->psbRoot,pc))!=NULL)
	{
		return pRet->iFlags;
	}

	return 0;
}

void CSCOL::itUnSetFlag(const int iMask)
{
	if(ppsbIterate == NULL || iCurrIdx >= iIterateSz)
	{
		CUSTR s("ERROR: CSCOL::itUnSetFlag Can not access current iterator!");

		throw CERR(s);
	};

	UNSETFLAG(ppsbIterate[iCurrIdx]->iFlags, iMask);
}

void CSCOL::itSetFlag(const int iMask)
{
	if(ppsbIterate == NULL || iCurrIdx >= iIterateSz)
	{
		CUSTR s("ERROR: CSCOL::itUnSetFlag Can not access current iterator!");

		throw CERR(s);
	};

	SETFLAG(ppsbIterate[iCurrIdx]->iFlags, iMask);
}

// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->

// Initialize work sets hash function
#define INIT_HL_SZ 1024
#define INIT_HL_SZ_MASK (INIT_HL_SZ-1)

CSHCOL::CSHCOL():iTabSz(0),	pphlNodes(NULL)
{
	int i;

	if((pphlNodes = new HL_NOD *[INIT_HL_SZ])==NULL)
		return;

	iTabSz = INIT_HL_SZ;
	for(i=0;i<INIT_HL_SZ;i++)
	{
		pphlNodes[i] = NULL;
	}
}

CSHCOL::~CSHCOL()
{
	FreeHTable();
}

#define H_SEED 111
//
//Hashing function is optimized for hashtable-size (INIT_HL_SZ) with a power of
// two, readjust that size to suit your needs (in power of 2 steps)!
//
unsigned int CSHCOL::hGetKey(const UCHAR *word)
{
	char    c;
	unsigned int h;

	//Use each letter of the string to hash into a certain bucket
	for( h=H_SEED ; ( c=s::cToUpper(*word))!='\0' ; word++ )
	{
		if((c & 2) == 0)
			h ^= ( (h << 5) + c );
		else
			h ^= ( c + (h >> 2) );
	}

	return(h & INIT_HL_SZ_MASK);   //Cut-down to size of actually available buckets
}
#undef H_SEED

#undef INIT_HL_SZ_MASK

// Free information and nullify pointer
void CSHCOL::FreeHTable()
{
	int i;
  HL_NOD *phlTmp, *phlTmp1;

	if(pphlNodes == NULL) return;

	for(i=0;i<INIT_HL_SZ; i++)
	{ 
		phlTmp = pphlNodes[i];
		while(phlTmp != NULL)
		{
			phlTmp1 = phlTmp->pNext;
			delete [] phlTmp->pc;
			delete phlTmp;

			phlTmp = phlTmp1;
		}
	}

	delete [] pphlNodes; //Free Ptrs to nodes and root structure
	pphlNodes=NULL;
}

void CSHCOL::Debug()
{
	int i,iCnt, iMax=0, iCntZero=0;
  HL_NOD *phlTmp;

	printf("DEBUG HASH:\n");

	for(i=0;i<INIT_HL_SZ; i++)
	{ 
		//Count nodes per hash table
		for(phlTmp = pphlNodes[i], iCnt=0;phlTmp != NULL;
        iCnt++, phlTmp = phlTmp->pNext)
		;

		if(iMax < iCnt) iMax = iCnt;

		if(iCnt == 0) iCntZero++;

		printf("%i,", iCnt);

		if((i&31)==31) printf("\n");
	}
	printf("\n");
	printf("Max Slot: %i Zero Count: %i\n", iMax, iCntZero);
}
#undef INIT_HL_SZ

// Insert string into hash table
//------------------------------>
void CSHCOL::Add(const UCHAR *pcFName)
{
	int		hval = hGetKey(pcFName);

	HL_NOD **ppFind = &pphlNodes[hval];

	while(*ppFind != NULL && s::scmp((*ppFind)->pc, pcFName) != 0)
		ppFind = &((*ppFind)->pNext);

	if(*ppFind == NULL)
	{
		*ppFind          = new HL_NOD;
		(*ppFind)->pc    = s::sdup(pcFName);
		(*ppFind)->pNext = NULL;
	}
}

// Find string in hash table
//
bool CSHCOL::Get(const UCHAR *pc)
{
	int		hval = hGetKey(pc);
	HL_NOD *hp;
	
	hp=pphlNodes[hval];
	while(hp != NULL && s::scmp(pc, hp->pc) != 0)
		hp = hp->pNext;

  return(hp != NULL);
}

// Find string in hash table (case insensetive)
//
bool CSHCOL::Get_i(const UCHAR *pc)
{
	int		hval = hGetKey(pc);
	HL_NOD *hp;
	
	hp=pphlNodes[hval];
	while(hp != NULL && s::scmp_ci(pc, hp->pc) != 0)
		hp = hp->pNext;

  return (hp != NULL);
}

// Initialize work sets hash function
#undef INIT_HL_SZ
#undef INIT_HL_SZ_MASK
