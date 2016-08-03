//
//  sym_tab.hh
//  Bighorn
//
//Header file for Symbol Table Implementation

#ifndef __SYMTAB__
#define __SYMTAB__

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;


//Forward Declaration
class CSymtab;

typedef enum scopetype
{
	GLOBAL_SCOPE=0,
	DEFAULT_SCOPE=GLOBAL_SCOPE,
	FUNCTION_SCOPE,
	BLOCK_SCOPE,
}scopetype_t;

typedef struct sIdtype	
{
	int type;
	string name;
	bool isfunc;
	string stringlit;
}symdata_t,*pSymdata;


typedef class CSymtab
{

public:
	scopetype_t sym_type;
	string type_name;

	vector<CSymtab*>  sym_child;
	CSymtab* sym_parent;

	vector<string> shadow_vector;

	std::map<string,symdata_t*> hash_container;
	std::map<string,symdata_t>::iterator it;
	
	CSymtab(scopetype_t scope) : sym_type(scope) 
	{
		sym_parent=NULL;
	}

	~CSymtab() {}

	bool add(string symname, pSymdata symdata);
	bool isvalidsymbol(string symname);
	bool getvalue(string symname,pSymdata val);
	bool getsymboldata(string symname, pSymdata data);

	bool check_for_shadow_entries(string symname);
	//bool check_for_duplicate(string symname);

private:

}*pSymtab;

void print_symbol_table(CSymtab* tab);

#endif
