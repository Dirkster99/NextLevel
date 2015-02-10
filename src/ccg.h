
#ifndef CCG_328593764857345
#define CCG_328593764857345

#include "custr.h"
#include "cscol.h"
#include "defs.h"
#include "cproccol.h"

//class CPROCCOL; //Build CG from CPROCOL and name of procedure

/**********
  A CallGraph (CG) is a tree with 0<n<=N childnodes
	Each child has a String Key and Childs are only added to and never removed

  For x example:
	x calls y
	y calls z and w
	w calls v has the following CallGraph:

	x
	+- y
	   +- z
		 +- w
		    +- v                                                    
******************                                                **********/
//FLAGGING defintions for iFlags in CG (below)
//
#define F_RECURSION 0x01 //Set this flag for nodes that occur
                         //twice in a call graph (on second node)

#define F_EXTERN_CALL 0x02 //Call to external database procedure
#define F_EXTRES_CALL 0x04 //This is an external database procedure with resolved HTML LINK

class CCG
{
public:
	CCG();

  //Construct callgraph from procedural collection and procedure object
  CCG(CPRCCOL *prcColl, CPROC *pPRC
     ,const THIS_DB & thisDB               //Name of Server,Database,Owner ... we are running in
     ,const CUSTR   & sOutPath);

  ~CCG();

	CCG(CCG & prcIn)
	{
		throw CERR("Copy constructor for CCG NOT implemented, yet!!!\n");
	}

	CCG & operator=(const CCG *pPtr)
	{
		throw CERR("Equal operator for CCG NOT implemented, yet!!!\n");
	}

	CCG & operator=(const CCG & ccg)
	{
		throw CERR("Equal operator for CCG NOT implemented, yet!!!\n");
	}

//	void DebugCG(CG *pcgCalls, FILE *fpOut);
//	void DebugCallGraph(CG *pcgCalls, int iSpace, ULONG lMask, bool *piSpaceFlag, FILE *fpOut);

  //=======================================================>
	//Print a Pretty Tree with all Bells'n Whistles'n all that
	//=======================================================>
	void PrintJSCallGraph_HTML(FILE *fpOut = stdout);

private:
	typedef struct cg       // This node implements the CG tree
	{
		CUSTR sServer,sDB, sOwner,sName;   //Reference to database objekt
			int iProcID;             //Unique identifier for this procedure/trigger
		CUSTR sFileName;          //Filename for target when linking in HTML

		struct cg **ppCG;
		int    CGSz;

		int iFlags;

		cg():iProcID(0), ppCG(NULL),CGSz(0),iFlags(0) //CTor for default values
		{}

	} CG;

	CG *pcgRoot;      //Root of callgraph tree
  //Name of proc for which we manage the callgraph
	CUSTR sServer,sDB,sOwner,sProcName;
	CUSTR sThisServer,sThisDB,sThisOwner;

	// ================== Private Private Stuff =====================
	//A Callpath is a linked list of nodes which shows a particular
	//calling path through a CallGraph, we maintain this structure
	//to prevent falling into recursive loops and to keep track of
	//callers that we may need to insert at the root level of the tree
	//================================================================>
	typedef struct call_path
	{
		struct             cg *pcgNode; //Pointer to CallGraph Node
		int                    idx;     //Index used in this root
		struct call_path      *pNext;   //Pointer to next CallGraph Pointer in Path
		
		call_path():pcgNode(NULL), idx(0), pNext(NULL) //CTor for default values
		{}
	} CALL_PATH;

  //callpath structure is used to stick a needle into the callgraph
  //and point at a unique node in the tree, the pointer is then moved
  //as we browse the tree for more callers...
	typedef struct cp_root
	{
		struct call_path *pFirst;
		struct call_path *pLast;

		cp_root():pFirst(NULL), pLast(NULL) //CTor for default values
		{}
		
	} CP_ROOT;

	CALL_PATH *CreateCPNode(CG *pcgNode, int idxIn);
  bool FindCG_ByName(const UCHAR *pcServer,const UCHAR *pcDB
                    ,const UCHAR *pcOwn,const UCHAR *pcName,CP_ROOT *plRoot);

	void DisplayList(CP_ROOT *plRoot);
	void DebugList(CP_ROOT *plRoot);
	void InsertCG(CG *pcgNode, int idxIn, CP_ROOT *pcpRoot);
	void DestroyList(CP_ROOT *plRoot);
	int CPLen(CP_ROOT *plRoot);
	void DestroyLastCP(CP_ROOT *plRoot);

	CG *AllocCG(const UCHAR *pcServ,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcName
	           ,int ID, CUSTR & sFileName);

	void AllocChildren(CG *pcgThis, int iChildCnt);

	//Convert exec dependencies of procs into callgraph items
	void ExecDeps2CG(CG *pCG, CDEPS *pdpsProc, CPRCCOL *prcDBProcs
                  ,const UCHAR *pcThisServ,const UCHAR *pcThisDB, const CUSTR & sOutPath);

	//Create a Callgraph node with a name pcKey and the ChildNodes from pdcTree
	///////////////////////////////////////////////////////////////////////////
  CG *CreateCGNode(CPROC & rcProced, CPRCCOL *prcCol
                  ,const UCHAR *pcThisServ, const UCHAR *pcThisDB, const CUSTR & sOutPath);

	//Set Recursion flag, if required, so we do not recurse more than twice
	//in the callgraph (we flag first recursion and stop iteration there)
	//====================================================================>
	void ResolveRecursion(CG *pcgNode, CP_ROOT *pcpPath);

	//======================================================================>
	//Find the next direct call candidate in current set of child nodes in CG
	//based on the last cg node and index in the call_path structure
	//======================================================================>
	const UCHAR *FindNextDirectCalls(CP_ROOT *pcpRoot, CPRCCOL *pdoTree, int *piChildCnt,
                                      CUSTR & sServOut,CUSTR & sDBOut, CUSTR & sOwnerOut);

  //
  //Free a complete CallGraph from memory
  //=======================================>
  void FreeCG(CG **ppcgCalls);
  void FreeCGBranch(CG *pcgCalls);

  //=================================================================================>
  //Print a Pretty Tree with all Bells'n Whistles'n all and interactivity and all that
  //=================================================================================>
  void PrintJSCallGraph_HTMLBody(CG *pcgCalls,FILE *fpOut,int iCntLevel,const CUSTR & s
                                ,const THIS_DB & thisDB);
};

#endif
