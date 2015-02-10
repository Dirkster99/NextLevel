
#ifndef TLIST_H_390485230948752309475
#define TLIST_H_390485230948752309475

//TList classes implement list functions that are used to manage
//abstract properties of an XML function
//
//An abstract property is a language dependent resource string --
//A language dependent resource string is a resource that is given
//in more than one way (or language)
//We use additional keys to store and retrieve those additional
//representations...depending on what the calling app wants to see

#include "custr.h"

//XVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXV
//VZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZ
//ZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZX
//XVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXV
//VZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZ
//ZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZXZXVXVZVZX

//This template class manages a list of words using a key of KeyTyp
//We use single linked lists, so don't store thousands of words here
//unless (bad) performance is no problem
template<class KeyTyp>
class KLst
{
protected:
	typedef struct _klst
	{
		KeyTyp ktKey;
		CUSTR sVal;
		struct _klst *pNext;

	}_KLST;

	_KLST *pRoot;

public:
	KLst(const KeyTyp & sKeyIn, const CUSTR & sValIn); //Constructor from item
	KLst();

	~KLst();

	KLst(const KLst & pbgCopy);           //Copy constructor & Equal operator
	KLst<KeyTyp> & operator=(const KLst & right);
	KLst<KeyTyp> & operator=(const KLst *pRight);

	//Add a new node to list
	void Add(const KeyTyp & sKeyIn, const CUSTR & sValIn);
};

template <class KeyTyp>
KLst<KeyTyp>::KLst():pRoot(NULL)
{
}

//Construct this from a list item
template <class KeyTyp>
KLst<KeyTyp>::KLst(const KeyTyp & sKeyIn, const CUSTR & sValIn)
{
	pRoot        = new _KLST(); //Create a new root node
	pRoot->ktKey = sKeyIn;
	pRoot->sVal  = sValIn;
	pRoot->pNext = NULL;
}

template <class KeyTyp>
KLst<KeyTyp>::~KLst() //Standard DTor
{
	_KLST *pTmp= pRoot , *pNxt;

	while(pTmp != NULL)      //Are there nodes for destruction?
	{
		pNxt = pTmp->pNext;  //Get next node in list
		delete pTmp;        //destroy current node
		pTmp = pNxt;       //set next node to current
	}
}

template <class KeyTyp>
KLst<KeyTyp>::KLst(const KLst & right):pRoot(NULL) //Copy constructor
{
	if(right.pRoot == NULL) return;

	_KLST *pNextIn, *pNextThis;

	pRoot = new _KLST();

	pRoot->sVal  = right.pRoot.sVal;
	pRoot->ktKey = right.pRoot.sKey;
	pRoot->pNext = NULL;

	pNextIn   = right.pRoot; //Ptr to copy and this list
	pNextThis = pRoot;

	while(pNextIn != NULL && pNextThis != NULL)
	{
		pNextThis = (pNextThis->pNext = new _KLST());
		pNextThis->sVal  = right.pRoot.sVal;
		pNextThis->ktKey = right.pRoot.sKey;
		pNextThis->pNext = NULL;

		pNextIn   = pNextIn->pNext;
	}
}

template <class KeyTyp>
KLst<KeyTyp> & KLst<KeyTyp>::operator=(const KLst & right) //Equal operator copies whole list
{
	_KLST *pTmp= pRoot , *pNxt;

	while(pTmp != NULL)      //Are there nodes for destruction?
	{
		pNxt = pTmp->pNext;  //Get next node in list
		delete pTmp;        //destroy current node
		pTmp = pNxt;       //set next node to current
	}

	KLst *pNextIn, *pNextThis;

	sVal = right.sVal;
	sKey = right.sKey;
	pNext  = NULL;

	pNextIn   = right.pNext; //Ptr to copy and this list
	pNextThis = this;

	while(pNextIn != NULL && pNextThis != NULL)
	{
		pNextThis->pNext = new KLst(pNextIn->sKey, pNextIn->sVal);

		pNextThis = pNextThis->pNext;
		pNextIn   = pNextIn->pNext;
	}

	return *this;
}

template <class KeyTyp>
KLst<KeyTyp> & KLst<KeyTyp>::operator=(const KLst *pRight) //Equal operator copies whole list
{
	_KLST *pTmp= pRoot , *pNxt;

	while(pTmp != NULL)      //Are there nodes for destruction?
	{
		pNxt = pTmp->pNext;  //Get next node in list
		delete pTmp;        //destroy current node
		pTmp = pNxt;       //set next node to current
	}

	if(pRight != NULL) *this = *pRight;

	return *this;
}

//
//Add a new key and value pair to list of keys and values
//
template <class KeyTyp>
void KLst<KeyTyp>::Add(const KeyTyp & sKeyIn, const CUSTR & sValIn)
{
	if(pRoot == NULL)
	{
		pRoot        = new _KLST(); //Create a new root node
		pRoot->ktKey = sKeyIn;
		pRoot->sVal  = sValIn;
		pRoot->pNext = NULL;
	
		return;
	}

	_KLST *pkNext = pRoot;

	while(pkNext != NULL)
	{
		if(pkNext->ktKey == sKeyIn)
		{
			pkNext->sVal  = sValIn; //Change Value entry
			return;
		}

		if(pkNext->pNext != NULL) //EOL reached yet???
			pkNext = pkNext->pNext;
		else
		{
			pkNext = (pkNext->pNext = new _KLST()); //Create a new root node at EOL
			pkNext->ktKey = sKeyIn;
			pkNext->sVal  = sValIn;
			pkNext->pNext = NULL;
		
			return;		
		}
	}
}

//<->>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->
//>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<
//-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<<->
//<->>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->
//>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<
//-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<-<>->-<<->>-<<->

//This template class manages a list of words using two keys
//We use single linked lists, so don't store thousands of words here
//because a splay tree or hash table is more efficient for that app
template<class KTp1, class KTp2>
class KKLst
{
protected:

typedef struct kklst2
{
	KTp2   key2;
	CUSTR  sVal;
	struct kklst2 *pNext;

}KKLST2;

typedef struct kklst1
{
	KTp1 key1;
	struct kklst1 *pNext;
	struct kklst2 *pVals;

}KKLST1;

	KKLST1 *pk1Root;

public:
	//Constructors and Dtor
	KKLst();
	KKLst(const KTp1 & Key1, const KTp2 & Key2, const CUSTR & sValIn);
	~KKLst();

	KKLst(const KKLst & pbgCopy);           //Copy constructor & Equal operator
	KKLst<KTp1,KTp2> & operator=(const KKLst & right);
	KKLst<KTp1,KTp2> & operator=(const KKLst *pRight);

	//Add a new node to list
	void Add(const KTp2 & Key2, const KTp1 & Key1, const CUSTR & sValIn);

	//Get a language name with a certain key
	//We input example: en, 0 and get English
	//            then: en, 1 and get Ingles
	//        and then: en, 2 and get Englisch etc...
	bool Get(const KTp1 & Key1, int & idx, CUSTR & sVal);

	void Reset(); //Free data stored in this object
};

template<class KTp1, class KTp2>
KKLst<KTp1,KTp2>::KKLst()
                 :pk1Root(NULL)
{
}

template<class KTp1, class KTp2>
KKLst<KTp1,KTp2>::KKLst(const KTp1 & Key1, const KTp2 & Key2, const CUSTR & sValIn)
{
	pk1Root = new KKLST1();

	pk1Root->pNext = NULL;         //Init first root node
	pk1Root->key1  = Key1;
	pk1Root->pVals = new KKLST2();

	pk1Root->pVals->pNext = NULL;  //Init first value node under first root node
	pk1Root->pVals->sVal  = sValIn;
	pk1Root->pVals->key2  = Key2;
}

template<class KTp1, class KTp2>
void KKLst<KTp1,KTp2>::Reset()
{
	KKLST1 *pk1Next, *pk1Tmp;
	KKLST2 *pk2Vals, *pk2Tmp;

	pk1Next = pk1Root;

	while(pk1Next != NULL) //Delete all rootkey nodes
	{
		pk2Vals = pk1Next->pVals;

		while(pk2Vals != NULL)     //Delete all value nodes for this rootkey
		{
			pk2Tmp = pk2Vals;
			pk2Vals = pk2Vals->pNext;
			delete pk2Tmp;
		}

		pk1Tmp  = pk1Next;        //Get next rootkey node and continue
		pk1Next = pk1Next->pNext;
		delete pk1Tmp;            //Free this root-key node
	}

	pk1Root = NULL;
}

template<class KTp1, class KTp2>
KKLst<KTp1,KTp2>::~KKLst()
{
	Reset();
}

template<class KTp1, class KTp2>
void KKLst<KTp1,KTp2>::Add(const KTp2 & Key2, const KTp1 & Key1, const CUSTR & sValIn)
{
	KKLST1 *pk1Next;
	KKLST2 *pk2Vals;

	pk1Next = pk1Root;

	while(pk1Next != NULL) //Vist rootkey nodes
	{
		if(pk1Next->key1 == Key1) //Found a match at root-key level???
		{
			pk2Vals = pk1Next->pVals;

			while(pk2Vals != NULL)     //Delete all value nodes for this rootkey
			{
				if(pk2Vals->key2 == Key2)
				{
					pk2Vals->sVal = sValIn;
					return;
				}

				if(pk2Vals->pNext != NULL) //Look at next node for this sub-key
					pk2Vals = pk2Vals->pNext;
				else                        //Insert new node for this sub-key
				{
					pk2Vals = (pk2Vals->pNext = new KKLST2());
					
					pk2Vals->pNext = NULL;  //Init first value node under first root node
					pk2Vals->sVal  = sValIn;
					pk2Vals->key2  = Key2;

					return;
				}
			}
		}   //Found a match at root-key level???

		if(pk1Next->pNext != NULL)
			pk1Next = pk1Next->pNext;
		else                        //Insert new root and sub-key here
		{
			pk1Next = (pk1Next->pNext = new KKLST1());

			pk1Next->pNext = NULL;
			pk1Next->key1  = Key1;
			pk2Vals = (pk1Next->pVals = new KKLST2());

			pk2Vals->pNext = NULL;  //Init first value node under new root node
			pk2Vals->sVal  = sValIn;
			pk2Vals->key2  = Key2;

			return;
		}
	}

	if(pk1Root == NULL) //No node inserted ever???
	{
		pk1Root = new KKLST1(); //Insert first root node

		pk1Root->pNext = NULL;
		pk1Root->key1  = Key1;
		pk2Vals = (pk1Root->pVals = new KKLST2());

		pk2Vals->pNext = NULL;  //Init first value node under first root node
		pk2Vals->sVal  = sValIn;
		pk2Vals->key2  = Key2;
	}

	return;
}

//Get a language name with a certain key
//We input for example: en, 0 and get English
//                then: en, 1 and get Ingles
//            and then: en, 2 and get Englisch etc...
template<class KTp1, class KTp2>
bool KKLst<KTp1,KTp2>::Get(const KTp1 & Key1, int & idx, CUSTR & sVal)
{
	KKLST1 *pk1Next;
	KKLST2 *pk2Vals;

	pk1Next = pk1Root;

	while(pk1Next != NULL) //Vist rootkey nodes
	{
		if(pk1Next->key1 == Key1)
		{
			pk2Vals = pk1Next->pVals;
			int iCnt = 0;

			while(pk2Vals != NULL)
			{
				if(iCnt == idx)
				{
					sVal = pk2Vals->sVal;  //Copy return value and increase idx
					idx++;
					return true;
				}

				pk2Vals = pk2Vals->pNext;
				iCnt++;
			}

			return false;
		}

		pk1Next = pk1Next->pNext;
	}

	return false;
}

//DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:
//OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:
//TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:
//TODO: The items below should be implemented if they are needed
//
template<class KTp1, class KTp2>
KKLst<KTp1,KTp2>::KKLst(const KKLst & pbgCopy) //Copy constructor & Equal operator
{
	throw CERR("Copy constructor for template<class KTp1, class KTp2> NOT implemented!!!\n");
}

template<class KTp1, class KTp2>
KKLst<KTp1,KTp2> & KKLst<KTp1,KTp2>::operator=(const KKLst & right)
{
	throw CERR("Equal operator for template<class KTp1, class KTp2> NOT implemented!!!\n");

	return *this;
}

template<class KTp1, class KTp2>
KKLst<KTp1,KTp2> & KKLst<KTp1,KTp2>::operator=(const KKLst *pRight)
{
	throw CERR("Equal operator for template<class KTp1, class KTp2> NOT implemented!!!\n");

	return *this;
}

//DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:
//OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:
//TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:

//$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^
//%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&
//^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$
//&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%&$%^$%^&%^&$^&$%

//This template class manages a list of items using one unique key
//That is, there can be a list of words for each unique key, so the
//relation between key and item is <1>:MANY
//
//We use single linked lists, so don't store thousands of words here
//because a splay tree or hash table is more efficient for that app
//
template<class KN>
class KNLst
{
protected:

typedef struct kklst2
{
	CUSTR  sVal;
	struct kklst2 *pNext;

}KKLST2;

typedef struct kklst1
{
	KN key1;
	struct kklst1 *pNext;
	struct kklst2 *pVals;

}KKLST1;

	KKLST1 *pk1Root;

public:
	//Constructors and Dtor
	KNLst();
	KNLst(const KN & Key1, const CUSTR & sValIn);
	~KNLst();

	KNLst(const KNLst & pbgCopy);           //Copy constructor & Equal operator
	KNLst<KN> & operator=(const KNLst & right);
	KNLst<KN> & operator=(const KNLst *pRight);

	//Add a new node to list
	void Add(const KN & Key1, const CUSTR & sValIn);

	void Reset();  //Free data stored in this object
	void Debug(); //Print contents of list

	int GetNextItem(int & iX,int & iY, KN & key, CUSTR & sVal);
};

template<class KN>
KNLst<KN>::KNLst():pk1Root(NULL)
{
}

template<class KN>
void KNLst<KN>::Reset()
{
	KKLST1 *pk1Next, *pk1Tmp;
	KKLST2 *pk2Vals, *pk2Tmp;

	pk1Next = pk1Root;

	while(pk1Next != NULL) //Delete all rootkey nodes
	{
		pk2Vals = pk1Next->pVals;

		while(pk2Vals != NULL)     //Delete all value nodes for this rootkey
		{
			pk2Tmp = pk2Vals;
			pk2Vals = pk2Vals->pNext;
			delete pk2Tmp;
		}

		pk1Tmp  = pk1Next;        //Get next rootkey node and continue
		pk1Next = pk1Next->pNext;
		delete pk1Tmp;            //Free this root-key node
	}

	pk1Root=NULL;
}

template<class KN>
KNLst<KN>::~KNLst()
{
	Reset();
}

template<class KN>
KNLst<KN>::KNLst(const KN & Key1, const CUSTR & sValIn)
{
	pk1Root = new KKLST1();

	pk1Root->pNext = NULL;         //Init first root node
	pk1Root->key1  = Key1;
	pk1Root->pVals = new KKLST2();

	pk1Root->pVals->pNext = NULL;  //Init first value node under first root node
	pk1Root->pVals->sVal  = sValIn;
}

//Add a new item for a unique key
//A key can have multible items, so we either insert a new key and item
//or insert a new item under an existing key, or simply find key and item
//and add nothing at all :(
//
template<class KN>
void KNLst<KN>::Add(const KN & Key1, const CUSTR & sValIn)
{
	KKLST1 *pk1Next;
	KKLST2 *pk2Vals;

	pk1Next = pk1Root;

	while(pk1Next != NULL) //Vist rootkey nodes
	{
		if(pk1Next->key1 == Key1) //Found a match at root-key level???
		{
			pk2Vals = pk1Next->pVals;

			while(pk2Vals != NULL)     //Delete all value nodes for this rootkey
			{
				//Value is already stored under this key
				if(pk2Vals->sVal == sValIn) return;

				if(pk2Vals->pNext != NULL) //Look at next node for this sub-key
					pk2Vals = pk2Vals->pNext;
				else                        //Insert new node for this sub-key
				{
					pk2Vals = (pk2Vals->pNext = new KKLST2());
					
					pk2Vals->pNext = NULL;  //Init first value node under first root node
					pk2Vals->sVal  = sValIn;

					return;
				}
			}
		}   //Found a match at root-key level???

		if(pk1Next->pNext != NULL)
			pk1Next = pk1Next->pNext;
		else                        //Insert new root and sub-key here
		{
			pk1Next = (pk1Next->pNext = new KKLST1());

			pk1Next->pNext = NULL;
			pk1Next->key1  = Key1;
			pk2Vals = (pk1Next->pVals = new KKLST2());

			pk2Vals->pNext = NULL;  //Init first value node under new root node
			pk2Vals->sVal  = sValIn;

			return;
		}
	}

	if(pk1Root == NULL) //No node inserted ever???
	{
		pk1Root = new KKLST1(); //Insert first root node

		pk1Root->pNext = NULL;
		pk1Root->key1  = Key1;
		pk2Vals = (pk1Root->pVals = new KKLST2());

		pk2Vals->pNext = NULL;  //Init first value node under first root node
		pk2Vals->sVal  = sValIn;
	}

	return;
}

template<class KN>
void KNLst<KN>::Debug()
{
	KKLST1 *pk1Next;
	KKLST2 *pk2Vals;

	pk1Next = pk1Root;

	while(pk1Next != NULL) //Delete all rootkey nodes
	{
		printf("%s\n", (const char *)pk1Next->key1);
		pk2Vals = pk1Next->pVals;

		while(pk2Vals != NULL)     //Print all value nodes for this rootkey
		{
			printf("%s\n", (const char *)pk2Vals->sVal);
			pk2Vals = pk2Vals->pNext;
		}

		pk1Next = pk1Next->pNext;
	}
	printf("\n");
}

template<class KN>
int KNLst<KN>::GetNextItem(int & iX,int & iY, KN & key, CUSTR & sVal)
{
	KKLST1 *pk1Next;
	KKLST2 *pk2Vals;
	int iXCnt=0;

	pk1Next = pk1Root;

	while(pk1Next != NULL) //Delete all rootkey nodes
	{
		if(iXCnt == iX)
		{
			int iYCnt=0;
			pk2Vals = pk1Next->pVals;

			while(pk2Vals != NULL)     //Print all value nodes for this rootkey
			{
				if(iYCnt == iY)
				{
					key  = pk1Next->key1;  //Return data
					sVal = pk2Vals->sVal;
					iY++;                    //Get next item
					return 1;              //and return item found
				}

				pk2Vals = pk2Vals->pNext;
				iYCnt++;
			}

			if(iYCnt >= iY) //Item not found here,
			{              //so, get first item in next level
				iY=0;
				iX++;
			}
		}

		pk1Next = pk1Next->pNext;
		iXCnt++;
	}

	return 0;
}

//DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:
//OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:
//TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:
//TODO: The items below should be implemented if they are needed
//
template<class KN>
KNLst<KN>::KNLst(const KNLst & pbgCopy) //Copy constructor & Equal operator
{
	throw CERR("Copy constructor for template<class KN> NOT implemented!!!\n");
}

template<class KN>
KNLst<KN> & KNLst<KN>::operator=(const KNLst & right)
{
	throw CERR("Equal operator for template<class KN> NOT implemented!!!\n");

	return *this;
}

template<class KN>
KNLst<KN> & KNLst<KN>::operator=(const KNLst *pRight)
{
	throw CERR("Equal operator for template<class KN> NOT implemented!!!\n");

	return *this;
}

//DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:
//OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:
//TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:TODO:ODOT:DOTO:OTOD:

#endif

//EOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEO
//OFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOF
//FEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFEFEOEOFOFE

