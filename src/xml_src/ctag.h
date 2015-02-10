
#ifndef RAW_TAG_H____283463128571340
#define RAW_TAG_H____283463128571340

#include "custr.h"
#include "cprop.h"
#include "cdtd.h"

class CDTD; //compiling RawTag and DTD

//A raw xml tag is a string that represents one of the following items:
//
typedef enum ETAGTYP
{
	TGP_ERROR=0,    //This tag was recognized but has a syntax error
  TGP_OPEN=1,     //opening tag
  TGP_CONTENT=2,  //other than whitespace Text enclosed by opening and closing tags
  TGP_CLOSE=3,    //closing tag
  TGP_CLO_OPEN=4, //simple tag < .../> which is an opening and closing tag
  TGP_PI=5,       //Processing instruction
  TGP_COMMENT=6,  //Comment

  TGP_DOCTYPE=7,  //Comment

	TGP_ELEMENT=8,    //!ELEMENT  (not supported with properties)
	TGP_ATTLIST=9,    //!ATTLIST  (not supported with properties)
	TGP_NOTATION=10,  //!NOTATION (not supported with properties)
	
	TGP_ENTITY=11,      //Properties are supported but no SYSTEM or PUBLIC references
	TGP_ENTITY_PE=12,  //subtype of valid ENTITY decls
	TGP_ENTITY_GE=13, //subtype of valid ENTITY decls

	TGP_INCL=14,     //<![ .... ]>   -> Include section <![CDATA[ ]]>

	//Synchronize TGP_UNKNOWN with STR_RAW_TAG_TYP in RawTag.cpp !!!
  TGP_UNKNOWN=15   //Unknown or empty tag -> this item must have the highest ID!!!
};

class CTAG
{
public:
	int ResolveRefs(CDTD & dtd);

	//Construct a tag from a string
	CTAG(const UCHAR *pcTag=NULL, const ETAGTYP InTagTyp = TGP_UNKNOWN);
	CTAG(const CTAG & tag);                           //Copy constructor
	CTAG & operator=(const CTAG *pTag);
	CTAG & CTAG::operator=(const CTAG & tagObj);
	~CTAG();

	//Read only properties for Raw Tags
	ETAGTYP getTagTyp(){return m_TagTyp;} //Return the type og this tag

	CUSTR  getTagName(){return m_TagName;} //This is the name of a tag

	//Debug tag contents and state
	void DebugTag();

	CPROP Props; //Properties for this XML Tag

	//Process string properties and add to internal property bag
	int ProcessPropList(CUSTR & sProps);

	//Process an entity tag and set properties, and sub-typs accordingly
	int ProcessEntity(CUSTR & sProps);

	int          iContsSz()  const { return m_TagConts.slen(); }
	const UCHAR   *sConts()  const { return m_TagConts;        }
	       CUSTR  getConts() const { return m_TagConts;        }

	//Get number of children
	int ChildTag_Sz(){ return iChildrenActSz; }

	//Add a new child node and return index of insertion [0...n]
	int ChildTag_Add(const CTAG *pTag);

	//Remove one Child node at this index
	//(array will shrink by one and current indexes are invalid)
	int ChildTag_Remove(int idx);

	//Access child tags through array operator
	CTAG & operator[](int idx);

  //Get parent of this tag (this can be null, if this tag is not in an array)
	//========================================================================>
	const CTAG *GetParentName()
	{
		return ptgParent;
	}

private:
	//Post-processing functions for raw tag strings
	//(These are called by the constructor if there
	// was no type supplied at constructing time)
	ETAGTYP ProcessPI();
	ETAGTYP ProcessInstruction();
	ETAGTYP ProcessClosTag();
	ETAGTYP ProcessOpenClose();

	ETAGTYP m_TagTyp;   //Type of this tag
	CUSTR   m_TagName;  //Name of this tag (if available)
	CUSTR   m_TagConts; //Remaining contents of tag (without tag name)

	CTAG **ppChildren;               //Array of children nodes
	int iChildrenSz, iChildrenActSz; //Size of Array and actually used size of array
	CTAG *ptgParent;   //Pointer to parent tag (in case this is a child in an array of children)

	// Private function to access array of children tags +++++++++++++++++++>
	void FreeChildren()
	{
		for(int i=0;i<iChildrenActSz;i++) //Free array of children
		{
			delete ppChildren[i];
		}

		delete [] ppChildren;

		ppChildren = NULL;
		iChildrenSz = iChildrenActSz = 0;
	}

	void CopyArray(const CTAG & rt)
	{
		FreeChildren(); //Get rid of child tags

		if(rt.ppChildren != NULL)     //Copy array of children from caller
		{
			if((ppChildren = new CTAG * [rt.iChildrenSz])!=NULL)
			{
				iChildrenSz    = rt.iChildrenSz;
				iChildrenActSz = rt.iChildrenActSz;

				for(int i=0;i<iChildrenActSz;i++)
				{
					if((ppChildren[i] = new CTAG())!=NULL) *ppChildren[i] = rt.ppChildren[i];
				}
			}	
		}
	}
	// END OF Private function to access array of children tags +++++++++++++++++++>
};

#endif

