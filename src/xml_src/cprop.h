
#ifndef PROPBAG_H____283463128571340
#define PROPBAG_H____283463128571340

#include "custr.h"
#include "cerr.h"

class CPROP
{
public:
  CPROP();
  ~CPROP();

  CPROP(const CPROP & pbgCopy);           //Copy constructor
  CPROP & operator=(const CPROP & prpObj);
  CPROP & operator=(const CPROP  *prpPtr);

  bool AddProp(const CUSTR & Name, const CUSTR & Val); //Add a new property
  void DebugProp() const;                             //Debug the list of properties

  bool GetProp(int idx, CUSTR & Name, CUSTR & Val) const; //Get a property name and value
  CUSTR & sGetVal(int idx)   const;                      //Get Value for this index
  CUSTR sGetVal(CUSTR sName) const;                     //Get Value for this property name

  int Size() const; //Count properties and return size of bag

protected:
	//===========================================================
	class Prop	          //Use a helper class to model each
	{	                    //property as a node in a linked list
	private:	            //each node has a value and name and
												//names must be unique (case sensitive)
		Prop *pNext;        //in the overall list

	public:
		CUSTR sName;         //Name and value of property
		CUSTR sVal;

		Prop(const Prop & pIn);
		~Prop();

		Prop &operator=(const Prop & pIn);

		//Construct a node from a value and name string
		Prop(const CUSTR & sNameIn="", const CUSTR & sValIn="");

		//Get next property in linked list
		Prop *GetNext();
		
		void Insert(Prop *pNew); //Insert a new property in bag
	};
	// End of helper class, which is a linked list ///////////////

	Prop *pPropBag;
};

class CPROPCOL
{
private:
  typedef struct prop_nod
	{
		UCHAR *pc;      //String used as a hash-key
		struct prop_nod *pNext; //Link to next item
    CPROP m_Props; //Collection of values/key pairs for each key
  
    prop_nod(const UCHAR *pcIn, const CPROP & thisProps):pc(NULL),pNext(NULL)
    {
      this->pc      = s::sdup(pcIn);
      this->m_Props = thisProps;
    }
  } PROP_NOD;

	int iTabSz;
	PROP_NOD **pphlNodes;

	// Free highlighting information and return nullified pointer
	void FreeHTable();

	unsigned int hGetKey(const UCHAR *word) const;
  void CopyThis(const CPROPCOL & tbIn);

public:
	CPROPCOL();
	~CPROPCOL();

  CPROPCOL(const CPROPCOL & tbIn);   //Copy constructor + Equal Operator
  CPROPCOL & operator=(const CPROPCOL & tbIn);
  CPROPCOL & operator=(const CPROPCOL *pProps);

	// Insert string into hash table. 
	void Add(const UCHAR *pcFName, const CPROP & prIn);

	// Find string in hash table
	bool Get(const UCHAR *pc, CPROP & prOut) const;

	//Find string in hash table (case insensetive)
	bool Get_i(const UCHAR *pc) const;

	//Debug statistics
	void Debug() const;
};

#endif
