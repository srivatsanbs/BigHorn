//  bighornAST.h
//
//

#ifndef __BIGHORNAST__
#define __BIGHORNAST__

#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "bighorn_types.h"
#include "sym_tab.hh"


using namespace std;

//branch Struct
typedef struct branch_s
{
	int entry_target;
	int inter_target;
	int exit_target;
}*pbranch_t;

//Base Node class
typedef class CNode
{
    
public:

	pSymtab m_pSymtab;
	
	CNode(void)
	{
        
	}

	CNode(pSymtab tab) : m_pSymtab(tab)
	{
	
	}

	pSymtab getSymTable (void)
	{
		return m_pSymtab;
	}

	void setSymTable(pSymtab tab)
	{

	}
    
}*pNode_t;


typedef class CExpressionNode:public CNode
{	

public:

	typedef enum
	{
		EXPR=0,
		IDEN=1,
		LIT=2
	}exprtype_t;
		
	exprtype_t m_exprtype;
	char m_oper;
	CExpressionNode* m_pLeftExpression;
	CExpressionNode* m_pRightExpression;
	string m_presult;
	constType_t result_type;


	CExpressionNode()
	{
	}
	
	CExpressionNode(exprtype_t type):m_exprtype(type)
	{

	}
	
	CExpressionNode(char op, CExpressionNode* rExp,pSymtab tab) : CNode(tab)
	{
		m_pLeftExpression= NULL;
		m_oper = op;
		m_exprtype = EXPR;
		m_pRightExpression = rExp;
	}
	
	void addLeftExp (CExpressionNode* left_expression)
	{
		if(m_pLeftExpression)
		{
			m_pLeftExpression->addLeftExp(left_expression);
		}
		else
		{
			m_pLeftExpression = left_expression;
		}
	}
	
	virtual string getResult(void)
	{
		return m_presult;
	}
	
	virtual void setResult(string set)
	{
		m_presult = set;
	}
	
	virtual constType_t getResultType(void)
	{
		return result_type;
	}
	
	virtual bool parseExpression(pirHandle_t irHandle);
	
	virtual string getExprName(void)
	{
	}
	
	//phew! another virtual.Should never call this
	virtual int getIntVal(void)
	{
		return 0;
	}
	
	virtual float getFloatVal(void)
	{
		return 0;
	}
	
private:

}*pExpression_t;




typedef class CIdentifierNode:public CExpressionNode
{

private:

	string m_result;
public:

	constType_t m_vartype;
	std::string m_varname;

	//used in ravlue of an expression
	CIdentifierNode(string name) : m_varname(name),CExpressionNode(IDEN)
	{
	}
	
	//Return result temp variable
	virtual string getResult(void)
	{
		return m_presult;
	}
	
	virtual void setResult(string set)
	{
		m_presult = set;
	}	
	virtual string getExprName(void)
	{
		return m_varname;
	}

	virtual constType_t getResultType(void)
	{
		return m_vartype;
	}

	virtual bool parseExpression(pirHandle_t irHandle);	
	
	
}*pIdentfier_t;


typedef class CLiteralNode:public CExpressionNode
{
private:
public:
	constType_t m_type;
	int m_intval;
	float m_floatval;
	string m_presult;	
	
	CLiteralNode(constType_t type, float val)
	{
		m_exprtype = LIT;
		m_type = type;
				
		if(INT_TYPE == type)
		{
			m_intval = (int) val;
		}
		else
			m_floatval = float(val);
	}
	
	virtual string getResult(void)
	{
		return m_presult;
	}
	
	virtual void setResult(string set)
	{
		m_presult = set;
	}

	virtual constType_t getResultType(void)
	{
		return m_type;
	}
	
	int getIntVal(void)
	{
		return m_intval;
	}

	float getFloatVal(void)
	{
		return m_floatval;
	}	
	virtual bool parseExpression(pirHandle_t irHandle);	

}*pLiteral_t;

typedef class CStatementNode:public CNode
{
private:

public:
	typedef enum
	{
		ASSIGNMENT=0,
		READ,
		WRITE,
		RETURN
	}stmttype_t;
	
	stmttype_t m_type;
	
	CStatementNode()
	{
	
	}

	CStatementNode(pSymtab tab) : CNode(tab)
	{
	
	}

	
	CStatementNode(stmttype_t type,pSymtab tab) : m_type(type),CNode(tab)
	{
	
	}
	
	virtual bool parseStatement(pirHandle_t irHandle)
	{
		//default implementation
		return true;
	}

	opcode_t notop(opcode_t op)
	{
		switch(op)
		{
			case GE:
				return LT;
			case GT:
				return LE;
			case LE:
				return GT;
			case LT:
				return GE;
			case NE:
				return EQ;
			case EQ:
				return NE;
			default:
				op;
		}
	}
	
}*pStatement_t;

typedef class CAssignmentStatement:public CStatementNode
{

private:
	
	string m_lvalue;
	pExpression_t m_prvalue;
	
public:
	CAssignmentStatement(string id, pExpression_t expression,pSymtab tab) : CStatementNode(ASSIGNMENT,tab)
	{
		m_lvalue = id;
		m_prvalue = expression;
	}
	
	virtual bool parseStatement(pirHandle_t irHandle);
	
	~CAssignmentStatement()
	{
		delete m_prvalue;
	}

}*pAssignment_t;

typedef class CReadWriteStatement:public CStatementNode
{
private:

	vector<std::string>* m_pvIdList;

public:

	CReadWriteStatement(bool isRead, vector<std::string>* list,pSymtab tab) : CStatementNode(tab)
	{
		m_type = (isRead)? READ : WRITE;
		m_pvIdList = list;
	}
	
	virtual bool parseStatement(pirHandle_t irHandle);

	~CReadWriteStatement()
	{
		delete m_pvIdList;
	}	
	
}*p_readWritestmt_t;

typedef class CFunctionBody:public CNode
{
private:
	vector<CStatementNode*>* m_pStmtList;
public:

	CFunctionBody(pSymtab symtab, vector<CStatementNode*>* stmt_list) : CNode(symtab)
	{
		m_pStmtList=stmt_list;
	}
	
	bool parseFunctionBody(pirHandle_t irHandle);

	~CFunctionBody()
	{
		for(int i=0;i<m_pStmtList->size();i++)
		{
			delete (*m_pStmtList)[i];
		}
	}
	
	//Symbol table interface should be here??
}*pFuncBody_t;


typedef class CFuncDeclarationNode:public CNode
{
private:

		pFuncBody_t m_pfuncBody;			
		string m_funcname;

public:

	CFuncDeclarationNode(string name, pFuncBody_t func_body) : m_funcname(name),m_pfuncBody(func_body)
	{
		m_pfuncBody = func_body;
	}
	
	bool parseFunction(pirHandle_t irHandle);
	
	~CFuncDeclarationNode()
	{
		delete m_pfuncBody;
	}

}*pFuncDec_t;


typedef class CProgramBody: public CNode
{

private:

	vector<pFuncDec_t>* m_pvFuncList;
public:

	CProgramBody(pSymtab symtab, vector<pFuncDec_t>* list) : CNode(symtab)
	{
		m_pvFuncList = list;
	}
	

	bool parseProgramBody(void);

	~CProgramBody()
	{
		for(int i=0;i<m_pvFuncList->size();i++)
		{
			delete (*m_pvFuncList)[i];
		}

	}
  
}*pProgramBody_t;


typedef class CProgram : public CNode
{
	public:
	
	std::string m_csProgramName;
	pProgramBody_t m_pPgmBody;
	
	CProgram(string name, pProgramBody_t pPgmBody)
	{
		m_csProgramName = name;
		m_pPgmBody = pPgmBody;        
	}
   
  
	void GetProgramName(string& progName)
	{
		progName = m_csProgramName;
	}
   
	pProgramBody_t GetProgramBody(void)
	{
		return (m_pPgmBody);
	}
	
	bool parseProgramRoot();

	~CProgram()
	{
		delete m_pPgmBody;
	}
       
}*pProgram_t;

typedef class CConditionNode:public CNode
{

public:
	pExpression_t m_plExpression;
	pExpression_t m_prExpression;
	char m_oper;
	string m_op1,m_op2;
	constType_t	m_opr_type;
	opcode_t m_opcode;

	CConditionNode(pExpression_t le,char op,pExpression_t re)
	{
		m_plExpression = le;
		m_prExpression = re;
		m_oper = op;
	}
		
	bool parseCondition(pirHandle_t irHandle);

}*pCondition_t;

typedef class CRepeatUntilStatement:public CStatementNode
{
public:
	vector<pStatement_t>* m_pstmtlist;
	pCondition_t m_pcond;
private:

public:
	CRepeatUntilStatement(pSymtab symtab,vector<pStatement_t>* list,pCondition_t cond) : CStatementNode(symtab)
	{
		m_pstmtlist = list;
		m_pcond = cond;
	}
	
	bool parseStatement(pirHandle_t irHandle);
		
}*pRepeatUntilStatement_t;

typedef class CIfNode:public CNode
{
private:
public:
	vector<pStatement_t>* m_pstmtlist;
	
	CIfNode(pSymtab symtab,vector<pStatement_t>* list) : CNode(symtab)
	{
		m_pstmtlist = list;
	} 

	bool parseIf(pirHandle_t irHandle);
	
}*pIfNode_t;

typedef class CElseNode:public CNode
{
private:
public:
	vector<pStatement_t>* m_pstmtlist;
	
	CElseNode(pSymtab symtab,vector<pStatement_t>* list) : CNode(symtab)
	{
		m_pstmtlist = list;
	} 

	bool parseElse(pirHandle_t irHandle);

}*pElseNode_t;


typedef class CIfStatement:public CStatementNode
{

public:
	pIfNode_t m_pifnode;
	pElseNode_t m_pelsenode;
	pCondition_t m_pcond;	
private:

public:
	CIfStatement(pCondition_t cond,pIfNode_t ifnode, pElseNode_t elsenode)
	{
		m_pcond = cond;	
		m_pifnode = ifnode;
		m_pelsenode = elsenode;
	}
	
	virtual bool parseStatement(pirHandle_t irHandle);

}*pIfStatement_t;

typedef class CContinueStatement:public CStatementNode
{
private:
public:
	CContinueStatement()
	{

	}

	virtual bool parseStatement(pirHandle_t irHandle);

}pContStatement_t;

typedef class CBreakStatement:public CStatementNode
{
private:
public:
	CBreakStatement()
	{

	}

	virtual bool parseStatement(pirHandle_t irHandle);

}pBreakStatement_t;
#endif
