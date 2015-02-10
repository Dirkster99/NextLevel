
#include "chartab.h"
#include "custr.h"

#include <stdio.h>
#include <math.h>

//
//String Matching functions and TRIE implementation
//
//===============================================================>
//Determine if a keyword from a mask occures in the string
//===============================================================>
int FindKeyWord(const UCHAR *pcMask, const UCHAR *pcStr, int iKeyCnt, int iKeyLen,
                int & idx, int iIntValsSz, const INTVAL *pivVals)
{
	int idxMask = 0, iPosMask = 0, iOff = 0;

	for( ;pcStr[idx] != '\0' && idxMask<iKeyCnt && iPosMask<iKeyLen; )
	{
		iOff = (idxMask*iKeyLen) + iPosMask;

		if(pcMask[iOff] < '!') //End of Mask -> Match found?
		{
			if(isOutOfRange(pcStr[idx], iIntValsSz-1, pivVals) == true)
			if(idxMask < iKeyCnt)
				return idxMask;

			return -1;
		}

		if((UCHAR)pcMask[iOff] == pcStr[idx]) //Get next Mask or next letter
		{
			idx++;
			iPosMask++; //Get next letter in mask and string
		}
		else
		{
			if(idxMask < iKeyCnt)
				idxMask++;  //Get next mask
			else
				return -1;
		}
	}

	//We have a match so we return the index of the last mask
	//
	if(isOutOfRange((UCHAR)pcMask[iOff], iIntValsSz-1, pivVals) == false &&
		 isOutOfRange(pcStr[idx], iIntValsSz-1, pivVals) == false)
	return idxMask;

	return -1;
}

//===============================================================>
//Match/Mismatch a keyword from a list of keywords indicated by mask
//return -1 on mismatch              (MST_MISMATCH)
//return 0  on match for this letter (MST_MATCH)
//return Index [1...n] of keyword on match for whole string
//===============================================================>
int FindKeyWord(const UCHAR *pcMask, const UCHAR pc, const int iKeyCnt, const int iKeyLen,
                MSTATE & State)
{
	if((UCHAR)pcMask[State.iPos] == pc)
	{
		State.iPos++;
		
		if(pcMask[State.iPos] < '!') return State.iWrd; //This is a match for this word
		
		return MST_MATCH;
	}

	//Current letter is a mismatch --> Look at next keyword
	while(State.iWrd < iKeyCnt && (UCHAR)pcMask[State.iPos] < pc)
	{
		if(pcMask[State.iPos] < '!') return MST_MISMATCH;

		State.iPos += iKeyLen;  //Look at next keyword
		State.iWrd++;           //Increment index to keyword
	}

	if(ISWHITE(pcMask[State.iPos])) return MST_MISMATCH;

	if((UCHAR)pcMask[State.iPos] == pc)
	{
		State.iPos++;
		return MST_MATCH;
	}

	return MST_MISMATCH;
}

//===============================================================>
//Return index of name entity if there is one at the position
//indicated by idxBeg (idxBeg will show end of name entity),
//Otherwise return -1 (Syntax Error)
//
//SkipWhSpace == true -> Skipping leading white spaces
//===============================================================>
int MatchName(const UCHAR *pcStr, bool SkipWhSpace, int & idxBeg)
{
	//skip whitespaces too
	if(SkipWhSpace == true)
		for( ;pcStr[idxBeg] != '\0' && ISWHITE(pcStr[idxBeg]); idxBeg++) ;
	
	if(pcStr[idxBeg] == '\0') return -1; //We must have a character!!!

	int idxRet = idxBeg;

	if(isOutOfRange(pcStr[idxBeg], NAME1SZ-1, NAME1) == true) return -1;

	//Find end of name ('\0' is not in range, so this loop worx :)
	for(idxBeg++; isOutOfRange(pcStr[idxBeg], NAME2SZ-1, NAME2) == false; idxBeg++) ;

	return idxRet;
}

//===============================================================>
//This function worx just like MatchName with the difference that
//we extract a property value, which is limited by " or '
//(We look for somthing like: 'sample' or "sample")
//===============================================================>
int MatchVal(const UCHAR *pcStr, bool SkipWhSpace, int & idxBeg)
{
	//skip whitespaces too
	if(SkipWhSpace == true)
		for( ;pcStr[idxBeg] != '\0' && ISWHITE(pcStr[idxBeg]); idxBeg++) ;
	
	if(pcStr[idxBeg] != '\'' && pcStr[idxBeg] != '"') return -1;

	UCHAR cDel = pcStr[idxBeg];

	int idxRet = idxBeg;

	//Find end of value identifier
	for(idxBeg++; cDel != pcStr[idxBeg] && pcStr[idxBeg] != '\0'; idxBeg++) ;

	if(cDel != pcStr[idxBeg]) return -1;

	return idxRet;
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

TLEAF *CreateTLeaf(const UCHAR *pc, const UCHAR *pcRepl)
{
	TLEAF *pRet;

	pRet = new TLEAF; /**Link to leaf of the trie**/

	if(pRet != NULL)
	{
		pRet->pcRepl = s::sdup(pcRepl);
		pRet->pcCode = s::sdup(pc);
	}

	return pRet;
}

#define ROOT_BLOK_SZ 4 //This is the initial size and latter
#define NODE_BLOK_SZ 2 //grow value for root and other nodes

#if(NODE_BLOK_SZ < 2)
	#define MIN_BLOK_SZ 2
#else
	#define MIN_BLOK_SZ NODE_BLOK_SZ
#endif

//------------------------------------------------------------------------>
//Increase collection of links to support inserting new data in node
//------------------------------------------------------------------------>
void EnlargeNode(TNODE *pTNext, int iLevel)
{
		int iNewSz;
	
		if(iLevel > 0)
			iNewSz = pTNext->iActSz + NODE_BLOK_SZ;
		else
			iNewSz = pTNext->iActSz + ROOT_BLOK_SZ;

		UCHAR   *pcNewHash = new UCHAR[iNewSz];    //Get more Mem
		TLEAF **ppNewLefs = new TLEAF *[iNewSz];
		TNODE **ppNewNext = new TNODE *[iNewSz];

		if(ppNewNext == NULL) return; //No Mem to get anymore :(

		for(int i=0;i<pTNext->iActSz;i++)        //Copy values
		{
			pcNewHash[i] = pTNext->pHash[i];
			ppNewLefs[i] = pTNext->ppLefs[i];
			ppNewNext[i] = pTNext->ppNext[i];
		}

		delete [] pTNext->pHash; //destroy 'old collection of links
		delete [] pTNext->ppLefs;
		delete [] pTNext->ppNext;

		pTNext->pHash  = pcNewHash; //(re)set to 'new collection of links
		pTNext->ppLefs = ppNewLefs;
		pTNext->ppNext = ppNewNext;

		pTNext->iActSz = iNewSz;    //Resize new node
}

//------------------------------------------------------------------------>
//Insert a new leaf node at appropriate spot and increase node size if
//required. The code assumes that pc[idx] is not already present in pTNext
//------------------------------------------------------------------------>
bool InsertNewTLeaf(TNODE *pTNext, int idx, const UCHAR *pc, const UCHAR *pcRepl)
{
	if(pTNext==NULL || pc == NULL) return false;

	UCHAR cHash = pc[idx];

	if(pTNext->iSz >= pTNext->iActSz) //Make new space if we need some
		EnlargeNode(pTNext, idx);


	if(pTNext->iSz < pTNext->iActSz) //Still space available???
	{
		int j;

		for(j=0;j<(pTNext->iSz-1);j++)
		{
			//Insert between these two leafs ???
			if(pTNext->pHash[j] < cHash && pTNext->pHash[j+1] > cHash)
			{
				//Get space for one more entry
				for(int k=pTNext->iSz-1; k>=j; k--)
				{
					pTNext->pHash[k+1]  = pTNext->pHash[k];
					pTNext->ppLefs[k+1] = pTNext->ppLefs[k];
					pTNext->ppNext[k+1] = pTNext->ppNext[k];
				}

				//Insert new leaf code
				pTNext->pHash[j+1]  = cHash;
				pTNext->ppLefs[j+1] = CreateTLeaf(pc, pcRepl);
				pTNext->ppNext[j+1] = NULL;

				pTNext->iSz += 1; //Increase hash count for this node

				return true;
			}
		}

		if(pTNext->pHash[0] > cHash) //New node must be first node?
		{
			//Get space for another first entry
			for(int k=pTNext->iSz-1; k>=0; k--)
			{
				pTNext->pHash[k+1]  = pTNext->pHash[k];
				pTNext->ppLefs[k+1] = pTNext->ppLefs[k];
				pTNext->ppNext[k+1] = pTNext->ppNext[k];
			}
			j=0;
		}
		else
		{
			j = pTNext->iSz - 1; //New node must be last ...

			if(pTNext->pHash[j] < cHash)
				j +=1;
			else     //New node has to be second last
			{
				pTNext->pHash[j+1]  = pTNext->pHash[j];
				pTNext->ppLefs[j+1] = pTNext->ppLefs[j];
				pTNext->ppNext[j+1] = pTNext->ppNext[j];
			}
		}

		pTNext->pHash[j]  = cHash;
		pTNext->ppLefs[j] = CreateTLeaf(pc, pcRepl);
		pTNext->ppNext[j] = NULL;

		pTNext->iSz += 1; //Increase hash count for this node

		return true;
	}

	return false;
}

//--------------------------------------------------------------------->
//Create an intialized trie node and add a replacement leaf if supplied
//--------------------------------------------------------------------->
TNODE *CreateTNode(int iSz, const UCHAR *pc=NULL, const UCHAR *pcRepl=NULL, UCHAR c=0)
{
	if(iSz <= 0) return NULL;

	TNODE *pRet = new TNODE;

	if(pRet != NULL)
	{
		pRet->pHash   = new  UCHAR[iSz]; //Initialize new node with values
		pRet->ppLefs  = new TLEAF *[iSz];
		pRet->ppNext  = new TNODE *[iSz];
		pRet->iActSz  = iSz;
		pRet->iSz     = 0;

		if(pRet->ppNext == NULL) goto OnError;

		if(pc != NULL) //Add a leaf and hash code if available
		{
			if((pRet->ppLefs[0] = CreateTLeaf(pc, pcRepl)) != NULL)
			{
				pRet->ppNext[0] = NULL;
				pRet->pHash[0]  = c;
				pRet->iSz      += 1;
			}
			else
				goto OnError;
		}
	}

	return pRet;

OnError:                 //On panic we take the fast way out :(
	delete [] pRet->pHash;
	delete [] pRet->ppLefs;
	delete [] pRet->ppNext;

	delete pRet;
	return NULL;
}

//
//Create an intialized trie node and attach a replacement leaf if supplied
//------------------------------------------------------------------------>
TNODE *CreateTNode1(int iSz, TLEAF *ptl=NULL, UCHAR c=0)
{
	if(iSz <= 0) return NULL;

	TNODE *pRet = new TNODE;

	if(pRet != NULL)
	{
		pRet->pHash   = new UCHAR[iSz]; //Initialize new node with values
		pRet->ppLefs  = new TLEAF *[iSz];
		pRet->ppNext  = new TNODE *[iSz];
		pRet->iActSz  = iSz;
		pRet->iSz     = 0;

		if(pRet->ppNext == NULL) goto OnError;

		if(ptl != NULL) //Add a leaf and hash code if available
		{
			pRet->ppLefs[0] = ptl;
			pRet->ppNext[0] = NULL;
			pRet->pHash[0]  = c;
			pRet->iSz      += 1;
		}
	}

	return pRet;

OnError:                 //On panic we take the fast way out :(
	delete [] pRet->pHash;
	delete [] pRet->ppLefs;
	delete [] pRet->ppNext;

	delete pRet;
	return NULL;
}

//Return index i if a hash code is already used in this node
//otherwise, return -1 to indicate that this hash code is not in use
/////////////////////////////////////////////////////////////////////
int FindEqualSlot(const UCHAR c, TNODE *pTrNode)
{
	if(pTrNode == NULL) return -1;

	for(int i=0;i<pTrNode->iSz;i++)
	{
		//We have a match return index to this slot...
		if(pTrNode->pHash[i] == c) return i;
	}

	return -1;
}

//------------------------------------------------------------------------>
//Two entries in the trie have collidated at the letter indicated by iLevel
//-> Resolve this and create further nodes until one letter is different
//-> Caller must insert resulting sub-trie
//
//   The function assumes (and tests whether) both strings ptlNode->pcCode
//   and pc are longer than the iLevel
//   (Resolution should be done by caller, otherwise)
//------------------------------------------------------------------------>
TNODE *ResolveColl(TLEAF *ptlNode, const UCHAR *pc, const UCHAR *pcRepl, int iLevel)
{
	int iLen1 = s::slen(ptlNode->pcCode);
	int iLen2 = s::slen(pc);
	int i;

	iLevel++;
	if(iLen1 == iLen2 || iLen1 < iLevel || iLen2 < iLevel)
	{
		//Key strings are equal -> resolution is not possible (necessary)
		if(s::scmp(ptlNode->pcCode, pc) == 0) return NULL;
	}

	int iDiff;

	//Search difference in given strings and note position of that character
	for(iDiff=iLevel;ptlNode->pcCode[iDiff] == pc[iDiff] && iDiff < iLen1; iDiff++)
		;

	//Decrement to one less if have hit the end of a string
	if(iDiff >= iLen1 || iDiff >= iLen2) iDiff--;

	TNODE *ptRoot, *ptNext;

	ptRoot = ptNext = CreateTNode(NODE_BLOK_SZ);

	//Create new path until we hit a difference somewhere
	while(ptNext != NULL && iLevel < iDiff)
	{
		ptNext->pHash[0]  = pc[iLevel];
		ptNext->ppLefs[0] = NULL;
		ptNext->ppNext[0] = CreateTNode(MIN_BLOK_SZ);
		ptNext->iSz += 1;

		ptNext = ptNext->ppNext[0];
		iLevel++;
	}

	if(iLen1 == (iLevel+1)) //Have we hit the end of s1???
	{
		ptNext->pHash[0]  = ptlNode->pcCode[iLevel];
		ptNext->ppLefs[0] = ptlNode;
		ptNext->ppNext[0] = NULL;
		ptNext->iSz += 1;
		
		//Equal letter in last slot???
		if(ptlNode->pcCode[iLevel] == pc[iLevel])
			ptNext->ppNext[0] = CreateTNode(MIN_BLOK_SZ, pc, pcRepl, pc[iLevel+1]);
		else
			InsertNewTLeaf(ptNext, iLevel, pc, pcRepl);
	
		return ptRoot;
	}

	if(iLen2 == (iLevel+1)) //Have we hit the end of s2???
	{
		ptNext->pHash[0]  = pc[iLevel];
		ptNext->ppLefs[0] = CreateTLeaf(pc, pcRepl);
		ptNext->ppNext[0] = NULL;
		ptNext->iSz += 1;
	
		//Equal letter in last slot???
		if(ptlNode->pcCode[iLevel] == pc[iLevel])
			ptNext->ppNext[0] = CreateTNode1(MIN_BLOK_SZ, ptlNode, ptlNode->pcCode[iLevel+1]);
		else
		{
			i=1; //Maintain sorted order of hash index
			if(ptlNode->pcCode[iLevel] < pc[iLevel])
			{
				ptNext->pHash[1]  = ptNext->pHash[0];
				ptNext->ppLefs[1] = ptNext->ppLefs[0];
				ptNext->ppNext[1] = NULL;
				i=0;
			}

			ptNext->ppNext[i] = NULL;
			ptNext->pHash[i]  = ptlNode->pcCode[iLevel];
			ptNext->ppLefs[i] = ptlNode;
			ptNext->iSz      += 1;
		}
		return ptRoot;
	}

	//Remainder must be more than one character and un-equal in this slot
	i=0;
	int j=1;
	if(ptlNode->pcCode[iLevel] < pc[iLevel])
	{
		i=1;
		j=0;
	}

	ptNext->ppNext[i] = NULL;
	ptNext->pHash[i]  = pc[iLevel];
	ptNext->ppLefs[i] = CreateTLeaf(pc, pcRepl);
	ptNext->iSz      += 1;

	ptNext->ppNext[j] = NULL;
	ptNext->pHash[j]  = ptlNode->pcCode[iLevel];
	ptNext->ppLefs[j] = ptlNode;
	ptNext->iSz      += 1;

	return ptRoot;
}

//
// Add another node into a TRIE
/////////////////////////////////
bool trAddEntry(const UCHAR *pc, const UCHAR *pcRepl, TNODE **ppTrRoot, int *piMaxSz)
{
	TNODE *pTNext;
	int iLen,i;

	if((iLen = s::slen(pc)) <= 0) return false;

	if(*ppTrRoot == NULL)               //Create a root if there's none yet
	{                                   //insert new code and return
		if((*ppTrRoot = CreateTNode(ROOT_BLOK_SZ, pc, pcRepl, pc[0]))!=NULL)
			return true;

		return false;
	}
	else
		pTNext = *ppTrRoot;               //Update pointer to root and length of pc

	//Here we record the maximum length of any string ever
	//stored in the trie, this can be used to quickly determine
	//whether a string of a certain length is not in the trie
	if(*piMaxSz < iLen) *piMaxSz = iLen;

	//iLen--;
	int iLevel=0;

	//Now find appropriate node to insert new association
	//From there we have two possible options
	//1>Collision - Check Dublicate, resolve by adding new child
	//2>Insert new hash code in existing node
	while(iLen >= 0)
	{
		if((i = FindEqualSlot(pc[iLevel], pTNext)) < 0)    //Is this slot in use???
			return InsertNewTLeaf(pTNext,iLevel, pc, pcRepl); //No, insert new leaf and return!

		if(iLen > 1) //Collision with leaf node ???
		{
			if(pTNext->ppNext[i] != NULL)
			{
				pTNext = pTNext->ppNext[i]; //Browse further into next level of trie
				iLevel++;
				iLen--;
				continue;
			}
			
			if(pTNext->ppLefs[i] != NULL)
			{
				//Entry above is already leaf - No collision to resolve
				///////////////////////////////////////////////////////
				if(s::slen(pTNext->ppLefs[i]->pcCode) > (iLevel+1))
				{
					//Erase entry in link to leaf only if a resolution took place
					if((pTNext->ppNext[i] = ResolveColl(pTNext->ppLefs[i], pc, pcRepl, iLevel))!=NULL)
						pTNext->ppLefs[i] = NULL;

						return true;
				}
				else
				{
					if(pc[iLevel] != 0)
					{
						pTNext->ppNext[i] = CreateTNode(NODE_BLOK_SZ,pc,pcRepl,pc[iLevel+1]);

						return true;
					}
				}
			}
		}
		else //iLen == 1 so the new string end's in this level
		{
			if(pTNext->ppLefs[i] == NULL)
			{
				pTNext->ppLefs[i] = CreateTLeaf(pc, pcRepl);
				return true;
			}
			else
			{
				//Exchange leaf and new string if leaf has remaining characters
				if(s::slen(pTNext->ppLefs[i]->pcCode) > (iLevel+1))
				{
					TLEAF *ptlNode = pTNext->ppLefs[i];

					pTNext->ppLefs[i] = CreateTLeaf(pc, pcRepl);
					pTNext->ppNext[i] = CreateTNode1(MIN_BLOK_SZ, ptlNode, ptlNode->pcCode[iLevel+1]);
					return true;
				}
				else
					return true; //New word is same as leaf entry
			}
			
		}
	
		return false;
	}

	return false;  //NULL indicates an algorithmic or other exceptional error
	              //in other words: We should never get here if everything's fine
}

#undef ROOT_BLOK_SZ
#undef NODE_BLOK_SZ

// Call this to initialize code translation from a constant table of codes
//------------------------------------------------------------------------>
TNODE *trInitFromTable(const HCODE *pHCodes, UINT iSz, int *piMaxSz, UINT & iCnt)
{
	if(iSz == 0) return NULL;

	TNODE *pTrRoot=NULL;
	bool bRet=true;

	for(iCnt=0; bRet==true && iCnt<iSz;iCnt++)
		bRet = trAddEntry(pHCodes[iCnt].pcCode, pHCodes[iCnt].pcRepl, &pTrRoot,piMaxSz);

	return pTrRoot;
}

//This is the recursive function that is called to free the trie
//The caller frees the root pTrie node
//--------------------------------------------------------------------->
void trFreeTree1(TNODE *pTrie)
{
	int i;

	if(pTrie == NULL) return; //Delete entire Trie at this trie pointer

	for(i=0; i<pTrie->iSz; i++)
	{
		if(pTrie->ppNext[i] != NULL)
		{
			trFreeTree1(pTrie->ppNext[i]);
			delete(pTrie->ppNext[i]);
			pTrie->ppNext[i] = NULL;
		}

		if(pTrie->ppLefs[i] != NULL) //delete HTML code
		{
			delete [] pTrie->ppLefs[i]->pcCode;
			delete [] pTrie->ppLefs[i]->pcRepl;
			delete pTrie->ppLefs[i];

			pTrie->ppLefs[i] = NULL;
		}
	}
	
	pTrie->iSz = 0;
	if(pTrie->pHash != NULL)
	{
		delete [] pTrie->pHash;
		pTrie->pHash = NULL;
	}
	if(pTrie->ppLefs != NULL)
	{
		delete [] pTrie->ppLefs;
		pTrie->ppLefs = NULL;
	}

	if(pTrie->ppNext != NULL)
	{
		delete [] pTrie->ppNext;
		pTrie->ppNext = NULL;
	}
}

void trFreeTree(TNODE *pTrie)
{
	if(pTrie != NULL)
	{
		trFreeTree1(pTrie); // Free Trie pointed to by this trie pointer

		delete pTrie;
	}
}

//Find a hash code in the field of hash's and return index
//or negative number indicating that this leaf is not here...
/////////////////////////////////////////////////////////////////////
int FindNextHash(TNODE *pTRoot, UCHAR cHash)
{
	if(pTRoot == NULL) return -1;

	for(int i=0;i<pTRoot->iSz;i++)
	{
		if(cHash == pTRoot->pHash[i])
			return i;
		else
		{
			if(cHash < pTRoot->pHash[i])
				return -2;
		}
	}
	return -3;
}

// ====================== Find ptr to next element
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>
const UCHAR *trFindString(const UCHAR *pc, TNODE *pTRoot, int iCodeLen)
{
	int i = 0,j=0;

	if(pTRoot == NULL || pc == NULL) return NULL; //Nothing to match here :(

	//Optional parameter set ???
	if(iCodeLen<=0) iCodeLen = s::slen(pc);

	//browse the trie to terminating leaf
	while(i < iCodeLen && (j = FindNextHash(pTRoot, pc[i])) >= 0)
	{
		if((i+1) == iCodeLen) break; //All letters processed

		if(pTRoot->ppNext[j] != NULL)
		{
			pTRoot = pTRoot->ppNext[j];
			i++;
		}
		else
			break;
	}

	//Return string if leaf contains the correct content
	if(pTRoot != NULL && j >= 0)
	if(pTRoot->ppLefs[j] != NULL)
	if(s::scmp_n(pc,pTRoot->ppLefs[j]->pcCode, iCodeLen)==0)
	{
		return pTRoot->ppLefs[j]->pcRepl;
	}

	return NULL;
}

//--------------------------------------------------------------------->
//This code is for debugging purposes only
//We compare two tries and return a greater equal zero
//result if there is a difference between the two tries
//The returned value is the level of the trie, where the
//the difference was detected
//--------------------------------------------------------------------->
int CompareTries(TNODE *trRoot, TNODE *trRoot1, TNODE **ptDeb, int iLev)
{
	int i;

	if(trRoot != NULL && trRoot1 != NULL)
	{
		if(trRoot->iSz != trRoot1->iSz)
		{
			*ptDeb = trRoot;
			return iLev;
		}

		for(i=0; i<trRoot->iSz;i++)
		{
			if(trRoot->pHash[i] != trRoot1->pHash[i])
			{
				*ptDeb = trRoot;
				return iLev;
			}

			if((trRoot->ppNext[i] == NULL && trRoot1->ppNext[i] != NULL) ||
			   (trRoot->ppNext[i] != NULL && trRoot1->ppNext[i] == NULL))
			{
				*ptDeb = trRoot;
				return iLev;
			}

			if((trRoot->ppLefs[i] == NULL && trRoot1->ppLefs[i] != NULL) ||
			   (trRoot->ppLefs[i] != NULL && trRoot1->ppLefs[i] == NULL))
			{
				*ptDeb = trRoot;
				return iLev;
			}

			//Compare replacements only if they are unique over the whole trie
			//if(trRoot->ppLefs[i] != NULL)
			//{
			//	if(wstr::wscmp(trRoot->ppLefs[i]->pcCode, trRoot1->ppLefs[i]->pcCode)!=0) return iLev;
			//
			//	if(wstr::wscmp(trRoot->ppLefs[i]->pcRepl, trRoot1->ppLefs[i]->pcRepl)!=0) return iLev;
			//}
		}

		for(i=0; i<trRoot->iSz;i++) //Look at those sub-tries
		{
			int iRet;
			if((iRet=CompareTries(trRoot->ppNext[i], trRoot1->ppNext[i], ptDeb, iLev+1))>0)
				return iRet;
		}
	}

	//Two empty tries are equal
	if(trRoot == NULL && trRoot1 == NULL) return -1;

	if(trRoot == NULL || trRoot1 == NULL) return iLev;

	return -1;
}

void trDebugTree(TNODE *pTrie, int iLevel) // Debugging function
{
	int i,j;

	if(pTrie == NULL) return;

	wprintf(L"\n");

	for(j=0; j<iLevel; j++)
	{
		wprintf(L" ");
	}
	wprintf(L"iLevel: %i Hash '", iLevel);
	
	for(j=0; j<pTrie->iSz; j++) wprintf(L"%c",pTrie->pHash[j]);

	wprintf(L"'\n");

	for(i=0; i<pTrie->iSz; i++)
	{
		for(j=0; j<iLevel; j++)
		{
			wprintf(L" ");
		}

		wprintf(L"Hash %c ", pTrie->pHash[i]);
		
		if(pTrie->ppLefs[i] != NULL)
			wprintf(L"pc '%s'\n", pTrie->ppLefs[i]->pcCode);
		else
			wprintf(L"\n");
	}

	iLevel++;
	for(i=0; i<pTrie->iSz; i++)
	{
		if(pTrie->ppNext[i] != NULL) //Traverse in and print content on return
		{
			if(pTrie->ppNext[i] != NULL) trDebugTree(pTrie->ppNext[i],iLevel);
		}
	}
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>

#define ZERO_CHAR 48 // The zero character '0'
#define NINE_CHAR 57 // The nine character '9'

#define A_CHAR 65 //The code for upper case a character 'A'
#define F_CHAR 70 //The code for upper case f character 'F'

//Hex power table for code translation
//
#define HEX_POWSZ 9
const UINT HEX_POW[HEX_POWSZ] =
{
0xFFFFFFFF,
0xFFFFFFF,
0xFFFFFF,
0xFFFFF,
0xFFFF,
0xFFF,
0xFF,
0xF,
1
};

//Decimal power table for code translation
//
#define DEC_POWSZ 9
const UINT DEC_POW[DEC_POWSZ] =
{
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

//This function will return zero on error
//Function is limited to eight bit integer!!!
//This code assumes no leading zeros
////////////////////////////////////////////////////
UINT pc2i(const UCHAR *pcIn, PC_TYPE ptType, int iLen)
{
	if(pcIn == NULL) return 0;
	
	int i, iDigit;
	UINT  iRet=0;
	const UINT *pcTable;
	int   iTableSz, idxTable=0;

	if(ptType == PCT_DEC) //Convert hex number?
	{
		pcTable  = DEC_POW;
		iTableSz = DEC_POWSZ;
	}
	else  //Assume hex conversion by default otherwise
	{
		pcTable  = HEX_POW;
		iTableSz = HEX_POWSZ, idxTable=0;
	}

	//Limits of hex UCHAR representation exceeded ???
	//- This MUST be adjusted for longer codes such as UNICODE
	if(iLen > iTableSz) return 0;
	else
	{
		if(iLen < iTableSz) idxTable = iTableSz - iLen;
	}

	for(i=0; i<iLen; i++)
	{
		switch(ptType)
		{
			case PCT_DEC: //convert for decimal number
				if(pcIn[i] >= ZERO_CHAR && pcIn[i] <= NINE_CHAR)
					iDigit = (UCHAR)(pcIn[i] - ZERO_CHAR);
				else
					return 0;
				break;

			default: //Assume hex by default
				UCHAR c = s::cToUpper(pcIn[i]);

				//Setup iDigit to assume a value between 0 and 15
				//
				if(c >= ZERO_CHAR && c <= NINE_CHAR)
					iDigit = (UCHAR)(c - ZERO_CHAR);
				else
				{
					if(c >= A_CHAR && c <= F_CHAR)
						iDigit = (UCHAR)((c - A_CHAR) + 10);
					else
						return 0;
				}
		}

		//Compute power and add
		iRet = iRet + (pcTable[idxTable++] * iDigit);
	}

	return iRet;
}

#undef ZERO_CHAR // The zero character '0'
#undef NINE_CHAR // The nine character '9'

#undef A_CHAR //The code for upper case a character 'A'
#undef F_CHAR //The code for upper case f character 'F'

//Here is the function that gets an array of intervals and a chacter
//and determines whether that character was part of an interval or not
//
//Take a value and an array of intervals and return true
//if the value was part of an interval, otherwise false
bool isOutOfRange(const UCHAR & c, int iIntValsSz, const INTVAL *pivVals)
{
	int l = 0, m;

	while(iIntValsSz >= l) //binary search from Sedgewick's C-Algorithms
	{
		m = (l+iIntValsSz)/2;

		//This value is part of this interval
		if(c >= pivVals[m].cLowLimit && c <= pivVals[m].cUpLimit) return false;
		else
		{
			//The value is not part of this interval
			//So, check left and right side
			//
			if(pivVals[m].cLowLimit > c)
			{
				if(m <= 0) return true; //Its not in any interval
				else
				{                       //c falls between these intervals?
					if(pivVals[m-1].cUpLimit < c) return true;
				}
				
				iIntValsSz = m - 1;
			}
			else //Not left or middle, must be right from m then
			{
				if(m > iIntValsSz) return true; //Its not in any interval
				else
				{                        //c falls between these intervals?
					if(pivVals[m+1].cLowLimit > c) return true;
				}
			
			
				l = m + 1;
			}
		}
	}

	return true;
}
