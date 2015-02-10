
#include "cprop.h"
#include "cerr.h"

CPROP::CPROP():pPropBag(NULL){}

// Assignment operator
CPROP::CPROP(const CPROP & pbgCopy):pPropBag(NULL) //Copy constructor
{
	Prop *pThisTmp, *pTmp;

	if(pbgCopy.pPropBag != NULL)
	{
		pThisTmp = pPropBag = new Prop(*pbgCopy.pPropBag); //Copy first node
		pTmp = pbgCopy.pPropBag->GetNext();

		while(pTmp != NULL)							 //Copy properties node by node
		{
			pThisTmp->Insert(new Prop(*pTmp));

			pThisTmp = pThisTmp->GetNext(); //Copy node and continue
			pTmp     = pTmp->GetNext();
		}
	}
}

CPROP & CPROP::operator=(const CPROP & prpObj)
{
	Prop *pTmp = this->pPropBag; //destroy current list of properties

	while(pTmp != NULL)							 //Are there nodes for destruction?
	{
		Prop *pNxt = pTmp->GetNext();  //Get next node in list
		delete pTmp;                   //destroy current node
		pTmp = pNxt;                   //set next node to current
	}

	pPropBag = NULL;
	Prop *pThisTmp;

	if((pTmp = prpObj.pPropBag)==NULL) return *this;

	pThisTmp = this->pPropBag = new Prop(*pTmp); //Copy first node
	pTmp     = pTmp->GetNext();

	while(pTmp != NULL)							 //Copy properties node by node
	{
		pThisTmp->Insert(new Prop(*pTmp));

		pThisTmp = pThisTmp->GetNext(); //Copy node and continue
		pTmp     = pTmp->GetNext();
	}

	return *this;
}

CPROP & CPROP::operator=(const CPROP *prpPtr)
{
	Prop *pTmp = this->pPropBag; //destroy current list of properties

	while(pTmp != NULL)							 //Are there nodes for destruction?
	{
		Prop *pNxt = pTmp->GetNext();  //Get next node in list
		delete pTmp;                   //destroy current node
		pTmp = pNxt;                   //set next node to current
	}
	pPropBag = NULL;

	if(prpPtr != NULL) *this = *prpPtr;

	return *this;
}

//Standard destructor removes the list of properties
//
CPROP::~CPROP()
{
	delete pPropBag;
}

//Add a new property name and value to list of properties
//
//Return true if insert was successful and false if the new
//property is already in the list
//
bool CPROP::AddProp(const CUSTR & sNameIn, const CUSTR & sValIn)
{
	if(pPropBag == NULL)
	{
		pPropBag = new Prop(sNameIn, sValIn);

		return true;
	}

	//Find appropriate position for insertion in list
	//at end of list or at position between two props
	Prop *pNxt, *pTmp = pPropBag;

	while((pNxt = pTmp->GetNext()) != NULL )
	{
		//property may already be in list with same name!?
		if(pTmp->sName == sNameIn || pNxt->sName == sNameIn) return false;

		if(pTmp->sName > sNameIn && pNxt->sName < sNameIn) break;

		pTmp = pNxt;
	}

	pTmp->Insert(new Prop(sNameIn, sValIn)); //Insert new property in list

	return true;
}

void CPROP::DebugProp() const
{
	Prop *pTmp = pPropBag;

	while(pTmp != NULL)							 //Are there nodes for debugging?
	{
		printf("\tNAME: %s VAL: %s\n",(const char *)pTmp->sName, (const char *)pTmp->sVal);
		pTmp = pTmp->GetNext();
	}
}

int CPROP::Size() const
{
	if(pPropBag == NULL) return 0;

	Prop *pTmp = pPropBag;
	int iCnt=0;

	while(pTmp != NULL) //Are there nodes at all?
	{
		iCnt++;
		pTmp = pTmp->GetNext();
	}

	return iCnt;
}

//Get name and value pair for a property index
#define E_LOC "CPROP::GetProp"
bool CPROP::GetProp(int idx, CUSTR & sName, CUSTR & sVal) const
{
	Prop *pTmp;
	int iCnt;

	//Find the correct node for the index idx
	for(pTmp = pPropBag, iCnt=0; pTmp != NULL && iCnt<idx; iCnt++, pTmp = pTmp->GetNext())
		;

	if(pTmp == NULL)                                  //Available index should
	{                                                 //be tested ahead of accessing!
		CUSTR Src("idx: ");
		throw CERR(E_LOC, ERR_IDX_OOF_RANGE, Src + idx); //Otherwise, things will be thrown!
	}

	sName = pTmp->sName; //Return string property name and value
	sVal  = pTmp->sVal;

	return true;
}
#undef E_LOC

//Get name and value pair for a property index
#define E_LOC "CPROP::sGetVal"
CUSTR & CPROP::sGetVal(int idx) const
{
	Prop *pTmp;
	int iCnt;

	//Find the correct node for the index idx
	for(pTmp = pPropBag, iCnt=0; pTmp != NULL && iCnt<idx; iCnt++, pTmp = pTmp->GetNext())
		;

	if(pTmp == NULL)                                  //Available index should
	{                                                 //be tested ahead of accessing!
		CUSTR Src("idx: ");
		throw CERR(E_LOC, ERR_IDX_OOF_RANGE, Src + idx); //Otherwise, things will be thrown!
	}

	return pTmp->sVal;
}
#undef E_LOC

//Get name and value pair for a property index
CUSTR CPROP::sGetVal(CUSTR sName) const
{
	Prop *pTmp;
	int iCnt;

	//Find the correct node for the index idx
	for(pTmp = pPropBag, iCnt=0; pTmp != NULL; iCnt++, pTmp = pTmp->GetNext())
	{
		if(pTmp->sName == sName)
			return pTmp->sVal;
	}

	return "";
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


CPROP::Prop::Prop(const Prop & pIn):pNext(NULL)
{
	sVal  = pIn.sVal;
	sName = pIn.sName;
}

CPROP::Prop::~Prop()
{
	Prop *pTmp= this->pNext , *pNxt;

	while(pTmp != NULL)				//Are there nodes for destruction?
	{
		pNxt = pTmp->pNext;   //Get next node in list
		pTmp->pNext = NULL;  //Bugfix: DOn't call dtor recursively here
		delete pTmp;        //destroy current node
		pTmp = pNxt;       //set next node to current
	}
}

CPROP::Prop & CPROP::Prop::operator=(const Prop & pIn)
{
	Prop *pNextIn, *pNextThis;

	sVal  = pIn.sVal;
	sName = pIn.sName;
	pNext = NULL;

	pNextIn   = pIn.pNext; //Ptr to copy and this list
	pNextThis = this;

	while(pNextIn != NULL && pNextThis != NULL)
	{
		pNextThis->pNext = new Prop(pNextIn->sName, pNextIn->sVal);

		pNextThis = pNextThis->pNext;
		pNextIn   = pNextIn->pNext;
	}


	return *this;
}

//Construct a node from a value and name string
CPROP::Prop::Prop(const CUSTR & sNameIn, const CUSTR & sValIn):pNext(NULL)
{
	sVal  = sValIn;
	sName = sNameIn;
}

//Get next property in linked list
CPROP::Prop *CPROP::Prop::GetNext(){ return pNext;}

void CPROP::Prop::Insert(Prop *pNew) //Insert a new property in bag
{
	pNew->pNext = this->pNext;
	this->pNext = pNew;
}


// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->

// Initialize work sets hash function
#define INIT_HL_SZ 32
#define INIT_HL_SZ_MASK (INIT_HL_SZ-1)

CPROPCOL::CPROPCOL():iTabSz(0),	pphlNodes(NULL)
{
	if((pphlNodes = new PROP_NOD *[INIT_HL_SZ])==NULL)
		return;

	iTabSz = INIT_HL_SZ;
	for(int i=0;i<INIT_HL_SZ;i++) pphlNodes[i] = NULL;
}

CPROPCOL::~CPROPCOL()
{
	FreeHTable();
}

void CPROPCOL::CopyThis(const CPROPCOL & propsIn)
{
  this->FreeHTable();

  if(propsIn.pphlNodes == NULL || propsIn.iTabSz <= 0) return; //Nothing to copy here :(

	if((this->pphlNodes = new PROP_NOD *[propsIn.iTabSz])==NULL) //Construct new table and copy each node
		return;

	this->iTabSz = propsIn.iTabSz;
	for(int i=0;i<this->iTabSz;i++)
	{
    this->pphlNodes[i] = NULL;

    PROP_NOD **pphInsHere = &pphlNodes[i], *hpTab = propsIn.pphlNodes[i];

    while(hpTab != NULL )
    {
      *pphInsHere = new PROP_NOD(hpTab->pc,hpTab->m_Props);           //Copy this table related info
      pphInsHere = &((*pphInsHere)->pNext);     //Look at end in destination

      hpTab=hpTab->pNext; //Look at next node in source collection
    }
	}
}

// Copy Constructors and Equal Operators
CPROPCOL::CPROPCOL(const CPROPCOL & propsIn):iTabSz(0),	pphlNodes(NULL)
{CopyThis(propsIn);}

CPROPCOL & CPROPCOL::operator=(const CPROPCOL & tbIn){CopyThis(tbIn); return *this;}
CPROPCOL & CPROPCOL::operator=(const CPROPCOL *pProps)
{
  if(pProps != NULL) *this = *pProps;

	return *this;
}

#define H_SEED 11
//
//Hashing function is optimized for hashtable-size (INIT_HL_SZ) with a power of
// two, readjust that size to suit your needs (in power of 2 steps)!
//
unsigned int CPROPCOL::hGetKey(const UCHAR *word) const
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

// Free information stored in this collection and nullify root ptr
void CPROPCOL::FreeHTable()
{
	int i;
  PROP_NOD *phlTmp, *phlTmp1;

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

void CPROPCOL::Debug() const
{
	int i,iCnt, iMax=0, iCntZero=0;
  PROP_NOD *phlTmp;

	printf("DEBUG CPROPCOL HASH_NODES:\n");

	for(i=0;i<INIT_HL_SZ; i++)
	for(phlTmp = pphlNodes[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
  {
    printf("Slot %i Prop Key %s\n", i, phlTmp->pc);
    phlTmp->m_Props.DebugProp();
  }

	for(i=0;i<INIT_HL_SZ; i++)
	{ 
	  //Count nodes per hash table
	  for(iCnt=0,phlTmp = pphlNodes[i];phlTmp != NULL; iCnt++, phlTmp = phlTmp->pNext)
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
void CPROPCOL::Add(const UCHAR *pcFName, const CPROP & prIn)
{
	int		hval = hGetKey(pcFName);

	PROP_NOD **ppFind = &pphlNodes[hval];

	while(*ppFind != NULL && s::scmp((*ppFind)->pc, pcFName) != 0)
		ppFind = &((*ppFind)->pNext);

	if(*ppFind == NULL)
	{
		*ppFind            = new PROP_NOD(pcFName, prIn);
	}
  else //TODO: Add properties, if this key is already in collection
  {
    
  }
}

// Find string in hash table
//
bool CPROPCOL::Get(const UCHAR *pc, CPROP & prOut) const
{
	int		hval = hGetKey(pc);
	PROP_NOD *hp;
	
	hp=pphlNodes[hval];
	while(hp != NULL && s::scmp_ci(pc, hp->pc) != 0)
		hp = hp->pNext;

  if(hp != NULL)
  {
    prOut = hp->m_Props; //Copy properties for caller to return
    return true;
  }
  return false;
}

// Find string in hash table (case insensetive)
//
bool CPROPCOL::Get_i(const UCHAR *pc) const
{
	int		hval = hGetKey(pc);
	PROP_NOD *hp;
	
	hp=pphlNodes[hval];
	while(hp != NULL && s::scmp_ci(pc, hp->pc) != 0)
		hp = hp->pNext;

  return (hp != NULL);
}

// Initialize work sets hash function
#undef INIT_HL_SZ
#undef INIT_HL_SZ_MASK
