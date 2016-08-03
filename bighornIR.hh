//
//  bighornIR.hh
//  bighorn
//

#ifndef __BIGHORNIR__
#define __BIGHORNIR__

#include "bighorn_types.h"
#include <fstream>

//PARSE the AST and generate code
typedef class cParseAST
{

private:
	void* m_pASTroot;
	fstream* m_pfstrm;
	fstream* m_ptinyfstrm;
public:


	typedef enum
	{
		OP_MEMORY=0x0,
		OP_STACK=0x2,
		OP_REGISTER=0x4,
		OP_LITERAL=8
	}operand_t;
	
	cParseAST(void* root)
	{
		m_pASTroot = root;
		
		m_pfstrm=NULL;
	}
		
	bool start_parseAST(void);

	bool gen_all(void);
	
	bool gen_IR(void);
	
	bool genCode(void);
	
	bool genPrologue(void);
	
	bool genBody(void);
	
	bool genEpilogue(void);
	
	bool AnalyzeOperand(list*,string*,map<string,int>&, bool, bool*,string*,string*);
		
}*pParseAST_t;

#endif //__BIGHORN__
