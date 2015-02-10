
#include "cpatrn.h"


CPATRN::CPATRN():phlFirst(NULL), phlLast(NULL), phlNext(NULL) //Reset root pointers
{
}

//Create a new highlighting pattern node...
//------------------------------------------->
CPATRN::PATRN *CPATRN::hlCreateNode2
 (const int iOffBegin, const int iOffEnd, const hlTYPE hlPatrnType
 ,const CUSTR & sServer, const CUSTR & sDB,const CUSTR & sOwner, const CUSTR & sDBObj, const CUSTR & sRawRef
 ,const int iFlags, PATRN *pPrev)
{
  PATRN *pRet;

  if((pRet = new PATRN())==NULL) return NULL;

  pRet->hlPatrnType = hlPatrnType;
  pRet->iOffBegin   = iOffBegin;
  pRet->iOffEnd     = iOffEnd;
  pRet->sServer     = sServer;
  pRet->sDB         = sDB;
  pRet->sOwner      = sOwner;
  pRet->sName       = sDBObj;
  pRet->sRawRef     = sRawRef;
  pRet->iFlags      = iFlags;
  pRet->pNext       = NULL;
  pRet->pPrev       = pPrev;

  return pRet;
}

//Add another color highlighting style to list of styles
//------------------------------------------------------>
void CPATRN::AddPatrn2(const int iOffBegin, const int iOffEnd, const hlTYPE hlPatrnType
                      ,const CUSTR & sRawRef,const CUSTR & sServer,const CUSTR & sDB,const CUSTR & sOwner
                      ,const CUSTR & sDBObj,const int iFlags)
{
  if(sServer.slen()>0 || sDB.slen()>0 || sOwner.slen()>0 || sDBObj.slen()>0)
  if(!(sServer.slen()>0 && sDB.slen()>0 && sOwner.slen()>0 && sDBObj.slen()>0))
  {
    CUSTR sErr("INCOMPLETE REFERENCE:");

    sErr += sServer, sErr += ".", sErr += sDB, sErr += ".", sErr += sOwner, sErr += ".", sErr += sDBObj;
    throw CERR(sErr, 0, "CPATRN::AddPatrn2");
  }

  if(phlFirst == phlLast && phlLast == NULL)
    phlFirst = phlLast = hlCreateNode2(iOffBegin, iOffEnd, hlPatrnType,
                                       sServer,sDB,sOwner,sDBObj, sRawRef,iFlags);
  else
  {
    if(phlLast->iOffBegin < iOffBegin)
    {
      phlLast->pNext = hlCreateNode2(iOffBegin,iOffEnd,hlPatrnType,
                                     sServer,sDB,sOwner,sDBObj, sRawRef, iFlags, phlLast);
      phlLast        = phlLast->pNext;
    }
    else  //Insert sorted by offset (highlightings may arrive in unsorted order)
    {    //So we search for the correct spot and attempt an insert there...
      struct patrn *pTmp = phlLast;
      
      while(pTmp->pPrev != NULL && pTmp->iOffBegin > iOffBegin)
        pTmp = pTmp->pPrev;

      if(pTmp == NULL) //Insert at the very beginning of the list
      {
        phlFirst->pPrev = hlCreateNode2(iOffBegin, iOffEnd, hlPatrnType
                                       ,sServer,sDB,sOwner,sDBObj,sRawRef,iFlags);
        phlFirst        = phlFirst->pPrev;
      }
      else //Insert somewhere in the middle of the list
      {
        struct patrn *pTmp1 = pTmp->pNext;
        pTmp->pNext  = hlCreateNode2(iOffBegin,iOffEnd,hlPatrnType
                                    ,sServer,sDB,sOwner,sDBObj,sRawRef,iFlags,pTmp);
        pTmp         = pTmp->pNext;
        pTmp->pNext  = pTmp1;

        if(pTmp1 != NULL) pTmp1->pPrev = pTmp;
      }
    }
  }
}

//Extend the last highlighting pattern, if it did not change,
//attach new highlighting otherwise
//=================================================================>
void CPATRN::AddExtendedPatrn2(const int iOffBegin, const int iOffEnd,const hlTYPE hlPatrnType, const CUSTR & sRawRef
                              ,const CUSTR & sServer,const CUSTR & sDB,const CUSTR & sOwner,const CUSTR & sDBObj
                              ,const int iFlags)
{
  if(phlLast != NULL)                      //sometime, neighboring items have
  if(phlLast->hlPatrnType == hlPatrnType) //same style, because they are of the
  {                                      //same type, in that case, we extend
    phlLast->iOffEnd = iOffEnd;         //the previous style
    phlLast->iFlags |= iFlags;         //and set additional flags, if any
    return;
  }

  //add completely new highlighting
  AddPatrn2(iOffBegin,iOffEnd, hlPatrnType, sRawRef,sServer,sDB,sOwner,sDBObj,iFlags);
}


//Copy List of patterns and offsets from one list into another
//-------------------------------=============================>
void CPATRN::CopyList(const PATRN *phlSrc)
{
  while(phlSrc != NULL)
  {
    AddPatrn2(phlSrc->iOffBegin, phlSrc->iOffEnd,phlSrc->hlPatrnType
             ,phlSrc->sRawRef,phlSrc->sServer,phlSrc->sDB, phlSrc->sOwner, phlSrc->sName,phlSrc->iFlags);

    phlSrc = phlSrc->pNext;
  }
}

CPATRN::CPATRN(const CPATRN & ptrIn):phlFirst(NULL), phlLast(NULL),phlNext(NULL)
{
  CopyList(ptrIn.phlFirst);
}

CPATRN & CPATRN::operator=(const CPATRN & ptrnIn)
{
  FreeMe();
  CopyList(ptrnIn.phlFirst);

  return *this;
}

CPATRN & CPATRN::operator=(const CPATRN *ptrnIn)
{
  FreeMe();
  if(ptrnIn != NULL) *this = *ptrnIn; //Run Equal operator by ref

  return *this;
}

void CPATRN::operator+=(const CPATRN & ptrnIn)
{
  CopyList(ptrnIn.phlFirst);
}

CPATRN CPATRN::operator+(const CPATRN & ptrnIn) const
{
  CPATRN ret(*this);

  ret += ptrnIn;

  return ret;
}

void CPATRN::FreeMe()
{
  while(phlFirst != NULL)
  {
    phlLast = phlFirst->pNext;
    delete phlFirst;           //Calling standard dtor for CUSTR

    phlFirst = phlLast;
  }
  phlFirst = phlLast= phlNext = NULL;
}

CPATRN::~CPATRN()
{
  FreeMe();
}

//Simple List Iterator function....Start
//-----------------------------------==>
void CPATRN::SetFirst()
{
  phlNext = this->phlFirst;
}

//Simple List Iterator function....Continue
//-----------------------------------=====>
//Depricated bool CPATRN::GetNext(int & iOffBegin, int & iOffEnd, hlTYPE & hlPatrnType, CUSTR & sName, int & iFlags)
//Depricated {
//Depricated   if(phlNext != NULL)
//Depricated   {
//Depricated     iOffBegin   = phlNext->iOffBegin;   //return data and set ptr to next node
//Depricated     iOffEnd     = phlNext->iOffEnd;
//Depricated     hlPatrnType = phlNext->hlPatrnType;
//Depricated     sName       = phlNext->sName;
//Depricated     iFlags      = phlNext->iFlags;
//Depricated 
//Depricated     phlNext = phlNext->pNext;
//Depricated     return true;
//Depricated   }
//Depricated 
//Depricated   return false;
//Depricated }

//Simple List Iterator function....Continue
//-----------------------------------=====>
bool CPATRN::GetNext(int & iOffBegin, int & iOffEnd, hlTYPE & hlPatrnType
                    ,CUSTR & sServIn,CUSTR & sDBIn,CUSTR & sOwnIn,CUSTR & sDBObjIn
                    ,CUSTR & sRawRef,int & iFlags)
{
  if(phlNext != NULL)
  {
    if(phlNext->sServer.slen()>0 || phlNext->sDB.slen()>0 ||
       phlNext->sOwner.slen()>0 || phlNext->sName.slen()>0)
    if(!(phlNext->sServer.slen()>0 && phlNext->sDB.slen()>0 &&
       phlNext->sOwner.slen()>0 && phlNext->sName.slen()>0))
    {
      CUSTR sErr("INCOMPLETE REFERENCE:");

      sErr += phlNext->sServer, sErr += ".", sErr += phlNext->sDB
    , sErr += ".", sErr += phlNext->sOwner, sErr += ".", sErr += phlNext->sName;
      throw CERR(sErr, 0, "CPATRN::GetNext2");
    }

    iOffBegin   = phlNext->iOffBegin;   //return data and set ptr to next node
    iOffEnd     = phlNext->iOffEnd;
    hlPatrnType = phlNext->hlPatrnType;

    sServIn  = phlNext->sServer;
    sDBIn    = phlNext->sDB;     //Copy reference to Database Object
    sOwnIn   = phlNext->sOwner;
    sDBObjIn = phlNext->sName;
    sRawRef  = phlNext->sRawRef;
    iFlags   = phlNext->iFlags;

    phlNext = phlNext->pNext;
    return true;
  }

  return false;
}

//Simple List Search function for previous nodes of actual node
//(This is primarely used to resolve cursor references inside of one file)
//-----------------------------------==========================---------+>
bool CPATRN::GetFindPrev(const hlTYPE hlPatrnType, CUSTR & sName)
{
  if(phlNext == NULL) return false; //No data in list, no previous to search for ...

  PATRN *pBrowse = phlNext->pPrev; //Start searching at previous nodes, if any

  while(pBrowse != NULL)
  {
    if(pBrowse->hlPatrnType == hlPatrnType)
    {
      if(pBrowse->sRawRef.scmp_ci(sName) == 0)
        return true;
    }
    pBrowse = pBrowse->pPrev;
  }

  return false;
}

//Simple List Search function for next nodes of actual node
//(This is primarely used to resolve cursor references inside of one file)
//-----------------------------------==========================---------+>
bool CPATRN::GetFindNext(const hlTYPE hlPatrnType, CUSTR & sName)
{
  if(phlNext == NULL) return false; //No data in list, no previous to search for ...

  PATRN *pBrowse = phlNext->pNext; //Start searching at previous nodes, if any

  while(pBrowse != NULL)
  {
    if(pBrowse->hlPatrnType == hlPatrnType)
    {
      if(pBrowse->sRawRef.scmp_ci(sName) == 0)
        return true;
    }
    pBrowse = pBrowse->pNext;
  }

  return false;
}

//Sometimes failure is part of the algorithm, and this is fine, as long as,
//failure correction IS PART of the algorithm, as well
//So, here we call this to set a correct db_object_owner, if it was assumed wrongly
//In some exotic cases the owner needs to be changed once all objects have been parsed
//This is done to re-align an ambigous reference to db objects belonging to a different owner
//than the refering object
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CPATRN::ResetRef2AmbigousOwner(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcName
                                   ,const UCHAR *pcNewOwner
                                   ,const int iMask) //Should be something like mFLAG_ASSUMED_OWNER
{
  PATRN *pBrowse = this->phlFirst; //Start searching at first node

  while(pBrowse != NULL)
  {
    if FLAG(pBrowse->iFlags, iMask)                  //Reset highlighting flags for assumed owners
    if(pBrowse->sServer.scmp_ci(pcServer) == 0 &&
       pBrowse->sDB.scmp_ci(pcDB)         == 0 &&    //to object we are currently looking for
       pBrowse->sOwner.scmp_ci(pcOwner)   == 0 &&
       pBrowse->sName.scmp_ci(pcName)     == 0
      )
    {
      pBrowse->sOwner = pcNewOwner;
    }
    pBrowse = pBrowse->pNext;
  }
}


