

#ifndef CSCOL_328593764857345
#define CSCOL_328593764857345

#include "defs.h"
#include "custr.h"
#include "cerr.h"

//----------------------------------------------------------------->
//String collection for sorting and storing large amounts of strings
//Suitable for 100...1000 strings or more
//----------------------------------------------------------------->
class CSCOL
{
public:
	CSCOL();
	~CSCOL();

	CSCOL(const CSCOL & pscIn);
	CSCOL & operator=(const CSCOL *pPtr);
	CSCOL & operator=(const CSCOL & scolObj);
	void EmptyColl();

	//Reset string collection
	void ResetTree(){ ResetTree(psbRoot); psbRoot=NULL;}

	//Find a string in the collection
	bool FindString(const UCHAR *pcName)
  {
    CUSTR s(pcName);
    return FindString(psbRoot, s);
  }

	//Print contents of string collection
	void DebugCollection(const UCHAR *pcIntro, FILE *fpOut)
	{
		DebugCollection(pcIntro, psbRoot, fpOut);
	}

	// The tree size is the number of cild-nodes stored in this tree
	const int Sz(){ return iTreeSz; }

	void AddCopyFrom(const CSCOL *pSrc)
	{
		if(pSrc != NULL)
		{
			ResetIterator();              //Reset Iterator collection
      this->CopyTree(pSrc->psbRoot);
		}
	}

	//Add another string into string collection
	void AddString(const CUSTR & sKey, const int iFlag=0);

	void AddString(const UCHAR *pcKey)
	{
		CUSTR s(pcKey);

		AddString(s);
	}

	//Iterator functions
	const UCHAR *GetFirst();
	const UCHAR *GetNext();

	//Set and UnSet Flags for current Iterator
	void itUnSetFlag(const int iMask);
	void itSetFlag(const int iMask);
	int itGetFlag();
	int itGetFlag(const UCHAR *pc);
private:
	typedef struct sbin_tree
	{
		CUSTR sName;  //Name of Item referenced in tree

		int iFlags; // Flags to descibe typ of item
								// Bit 1: -> 1: DB External item, 0: DB Internal item

		struct sbin_tree *psbLeft;
		struct sbin_tree *psbRight;

	} SBIN_TREE;

	SBIN_TREE *psbRoot; //Root node of string collection

	int iTreeSz; //Size of collection (number of nodes)

	//These are used for iterator functions
	SBIN_TREE **ppsbIterate;
	int iIterateSz, iCurrIdx;

	SBIN_TREE *CreateNode(const CUSTR & s, const int iFlags);
	void ResetTree(SBIN_TREE *pNode);
	bool FindString(SBIN_TREE *pbnRoot, const CUSTR & sName);
	void DebugCollection(const UCHAR *pcIntro, SBIN_TREE *pNode, FILE *fpOut);
	void CntTreeNodes(SBIN_TREE *pNode, int *ipCnt);

	void FillIterator(SBIN_TREE **ppsbIterate, int iSz,SBIN_TREE *psbRecurse, int *Idx);
	void ResetIterator();

	void CopyTree(const SBIN_TREE *pNode); //Copy a binary tree into another

	//void AddStr(const UCHAR *pcKey, const int iFlag=0);

	//Find a SBIN_TREE node with pcKey as name
	//=========================================>
	SBIN_TREE *FindNode(SBIN_TREE *thisNode, const UCHAR *pcKey);
};

//*****************************************************************
 //***************************************************************
  //*************************************************************

class CSHCOL
{
private:
	//Higlight information for parser output
	typedef struct hl_nod
	{
		UCHAR *pc;             //String that is used as hash-key

		struct hl_nod *pNext; //Link to next item
	} HL_NOD;

	int iTabSz;
	HL_NOD **pphlNodes;

	// Free highlighting information and return nullified pointer
	void FreeHTable();

	unsigned int hGetKey(const UCHAR *word);
public:
	CSHCOL();
	~CSHCOL();

	CSHCOL(CSHCOL & pshcIn)
	{
		throw CERR("Copy constructor for CSHCOL NOT implemented, yet!!!\n");
	}

	CSHCOL & operator=(const CSHCOL *pShc)
	{
		throw CERR("Equal operator for CSHCOL NOT implemented, yet!!!\n");
	}

	CSHCOL & operator=(const CSHCOL & shcObj)
	{
		throw CERR("Equal operator for CSHCOL NOT implemented, yet!!!\n");
	}

	// Insert string into hash table. 
	void Add(const UCHAR *pcFName);

	// Find string in hash table
	bool Get(const UCHAR *pc);

	// Find string in hash table (case insensetive)
	bool Get_i(const UCHAR *pc);

	//Debug statistics
	void Debug();
};


#endif
