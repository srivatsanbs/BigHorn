//
//  sym_tab.c
//  Bighorn
//
//  Symbol table implementation

#include "sym_tab.hh"
#include "bighornAST.hh"
#include "bighorn.tab.hh"

using namespace bighorn;    
typedef bighorn::parser::token token_t;

bool CSymtab::add(string symname, pSymdata symdata)
{
	bool retval=true;

	shadow_vector.push_back(symname);

	retval=(hash_container.insert(std::pair<string, symdata_t*>(symname,symdata))).second;
	return retval;
}

bool CSymtab::isvalidsymbol(string symname)
{
	bool retval=true;
	
	if(hash_container.end() ==  hash_container.find(symname))
	{
		if(sym_parent)
		{
			retval = sym_parent->isvalidsymbol(symname);
		}
	}
	else
		retval=true;

	return retval;
}

bool CSymtab::getvalue(string symname,pSymdata pval)
{
	bool retval=true;

	if(hash_container.end() ==  hash_container.find(symname))
	{
		if(sym_parent)
		{
			retval = sym_parent->getvalue(symname,pval);
		}
	}
	else
	{
		pval = &it->second;
		retval=true;
	}
	return retval;

}

bool CSymtab::check_for_shadow_entries(string symname)
{	
	bool retval=false;
	CSymtab* tab;

	tab = this->sym_parent;

	while(tab)
	{
		if(((tab->hash_container).find(symname)) != ((tab->hash_container).end()))
		{
			retval=true;
			break;
		}
		else
		tab = tab->sym_parent;
	}
	return retval;
}

void print_symbol_table(CSymtab* tab)
{

	std::vector<CSymtab*>::iterator it1;	
	std::vector<string>::iterator it2;
	int i;
	string name;
	//typedef std::map<std::string,symdata_t*>::iterator it2;

	if(!tab)
		return;

	cout<<"Symbol table "<<tab->type_name<<endl;
	
	for(i=0; i<tab->shadow_vector.size();i++)
	{
		name = tab->shadow_vector[i];

		if(!((tab->hash_container[name])->isfunc))
		{
			if((tab->hash_container[name]->type) == token_t::STRING)
			cout<<"name "<<name<<" type "<<"STRING"<<" value "<<(tab->hash_container[name])->stringlit<<endl;
			else
			{
				//cout<<"t:"<<token_t::INT<<"	type:"<<(tab->hash_container[name])->type;
				cout<<"name "<<name<<" type "<<(((tab->hash_container[name])->type==token_t::INT)?"INT":"FLOAT")<<endl;
			}
		}		
	}
	
	cout<<endl;
	it1=tab->sym_child.begin();
	while((it1!=tab->sym_child.end()))
	{
		print_symbol_table(*it1);
		it1++;
	}
	

}


bool CSymtab::getsymboldata(string symname, pSymdata data)
{
	bool ret_val;

	if(hash_container.find(symname) != hash_container.end())
	{
		*data = *(hash_container[symname]);
		ret_val=true;
	}
	else if(sym_parent)
	{
		ret_val=sym_parent->getsymboldata(symname, data);
	}
	else
	{
		ret_val=false;
	}
	return ret_val;
}
