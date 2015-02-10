//The class in this file defines a simple stack class for CTAGs
//
//I know this could be solved via STL, however, this class also
//keeps track of state (context). Every individual XML tag has
//a certain context, for example:
//
//1> <head><title>Losing You</title></head>
//2> <w><title>Losing You</title></w>
//
//Here, the title tag occurs in the context of head or w, each
//case can signify a completely different data item. Keeping state
//ensures that we can distinguish between 1> and 2>
//
//More pertinently, there may be unknown tags, such as: <xyz>...</xyz>
//using STN_UNKNOWN ensures that these tags will be ignored even if
//<title> occured inside of such an unknown tag :)
//
//Hence we are able to:
//1) check the syntax (balance of XML tags),
//2) ignore unknown data items,
//3) and process the data items that are known and occure in the correct
//   place (path: e.g. title in 1> has a path of head/title)
//
//I guess this is worth implementing a custom stack class :) ...
//
#ifndef CSTAK_H____283463128571340
#define CSTAK_H____283463128571340

#include "ctag.h"

//In this file we model a state graph to formulate a means
//of checking the symantic of XML tags.

//The state transfer machine changes from a STATE S to R when
//there is an edge that points from S to R or vice versa.
//A TRANSF node (below) defines an edge, the STATE node
//(further below) defines each unique state node of the graph,
//while the STRANS nodes model a state transition from one
//unique node to another

//A transfer definition has a name (string) and a unique id
typedef struct transdef
{
	char *pcName;
	int  iVal;

} TRANSDEF;

//A State has a State-Name,which describes the tag (or edge)
//on which this node was entered. The StateID is a unique ID,
//which identifies a node uniquly together with the iName value
//
//This means two STATE nodes with {x,0} and {y,0} are NOT allowed!
//Instead, only {x,z} and {x,w} with z != w are allowed.
typedef struct state      
{
	int  iName, iStateID;

} STATE;

//State transitions are modelled through
//a Source and Destination state, which means
//that we can transit from Src to Dst and back
typedef struct strans
{
	STATE Src, Dst;

} STRANS;

//This is the root node of the Major state engine
#define STN_ROOT   0
#define ST_ROOT    0

//and this is the state that is used before the root
//node is entered. This state means that the engine
//has not transfered to any stage (yet, 1), or the
//root node was entered and has already been exited
#define STN_CLEAN   -1

//The machine is in this state if the last tag was not known
//This means, we are in the context of an unknown tag, which
//turn means that we should not do anything but continue
//processing the input stream of XML ... until this state is left
#define STN_UNKNOWN  -2

class CSTAK
{
	//given a current state and a new state the state engine
	//can proceed deeper into the tree (right,down) or further
	//towards the root (left, up)
	typedef enum DIRECT{ dLeft=1, dRight=2 };

	typedef struct st_stak
	{  
		CTAG *prwTag;    //XML Tag information
		int iStateID;  //Current Major-State of the engine (STN_UNKNOWN or other)

		int   iStateIdx;  //Index to minor state transition diagram
		CTAG *ptgBaseTag; //Base ptr to XML tag for next tags in the XML stream

		st_stak *pNext; //Link to next node in linked list
		st_stak *pPrev; //Link to previous node in linked list

		st_stak():prwTag(NULL),iStateID(0),iStateIdx(-1), ptgBaseTag(NULL),pNext(NULL), pPrev(NULL)
		{}

	}ST_STAK;

	typedef struct trdef_root //Root for state transition diagram
	{
		TRANSDEF *pTransdef; //Array of state names and integerID equivalents
		int iDefSz;          //Size indicator

		trdef_root():pTransdef(NULL), iDefSz(0) //Standard Ctor for init2default
		{}
	}
	TRDEF_ROOT;

	ST_STAK *pstRoot, *pstLast; //Ptr to root and top of stack
	void ResetStack();

	bool bUsed; //Note if there were any tags ever been pushed onto stack

	typedef struct strans_root //Root for state transition diagram
	{
		STRANS *pTrans;        //Array of transitions and size indicator
		int iSz;
		int iRootStateID;    //Root state for xml parser

		strans_root():pTrans(NULL), iSz(0), iRootStateID(-1) //Standard Ctor for init2default
		{}
	}
	STRANS_ROOT;

	TRDEF_ROOT  MajorTRdefs; //Definitions of interesting states
	STRANS_ROOT MajorSTRans; //Major web of interesting state transitions that are parsed

	void ResetMajorTRdefs();

	//Determine state of top element in the stack
	int CSTAK::GetNextState(CTAG & rt, STRANS_ROOT *ptrsTrans);

	//Minor webs of interesting state transitions that are parsed
	//These webs define the children (if any) for each interesting tag
	//This means, sub-tags belong to a parent-tag if they are enclosed by
	//that parent-tag (open and close tag), and if their is a state transition
	//diagram defined here. TIP: USE CTAG::ChildTag_Sz and CTAG::[] to access sub-tags
	
	//!!! EACH STN definition can have only ONE minor transdef in this context
	//Ergo, having n STN defs yields that we can have never more than n Minor TRansDefs (phew)
	//========================================================================================>
	STRANS_ROOT **ppMinorSTRdefs;
	int iMinorSTRdefsSz, iMinorSTRdefsActSz;

	void ResetMinorTRdefs();
	STRANS_ROOT *CreateTRDefSlot(const STRANS *pTransIn,  //ptr 2 transitions
                               const int iDefSzIn,      //size of transitions
                               const int iRootStateID); //Root state for xml root

	// Check validity of minor web
	//===============================>
	bool CheckMinorWeb(const STRANS *pTransIn, const int iDefSzIn, int *piMinorRoot);
	//
	// Check whether 2 (major/minor or minor/minor) webs collide with state transitions
	//================================================================================>
	int CheckMinorMajor(const STRANS *pTransSrc, const int iDefSzSrc,
                      const STRANS *pTransDst, const int iDefSzDst, CUSTR & sErrSrc);

	void CheckMinorMinor(const STRANS *pTransSrc, const int iDefSzSrc,
                       const STRANS *pTransDst, const int iDefSzDst, CUSTR & sErrSrc);

public:
	CSTAK();
	~CSTAK();

	bool Push(CTAG & rt); //Stack functions
	CTAG *Pop();
	CTAG & Top();
	
	bool empty() //Stack empty == true if there is no node
	{
		if(pstLast != NULL) return false;  //Stack is not empty
		else
			return true;                     //Stack is empty
	}

	//Has there been a tag ever pushed on the stack???
	bool getUsed(){ return bUsed; };

	//Return the current state of the stack engine
	int getStateID()
	{
		if(pstLast == NULL) return STN_CLEAN;
		else
			return pstLast->iStateID;
	}

	//Define "interesting" tags and their "interesting" paths 
	void InitMajorEngine(const STRANS *pTransIn, int iSzIn, const TRANSDEF *pTdefIn, int iDefSzIn);

	//Define "interesting" tags and their "interesting" children
	int InitMinorEngine(const STRANS *pTransIn, const int iDefSzIn);
};

#endif

