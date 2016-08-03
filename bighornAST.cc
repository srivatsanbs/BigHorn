//
//  bighornAST.cc
//  sigh
//


#include "bighornAST.hh"
#include "bighorn_types.h"
#include "sym_tab.hh"
#include <sstream>
#include <stack>

using namespace std;

extern string IRopcodeTable[];

pirHandle_t IRlistArray=NULL;

stack<pbranch_t> g_branchstack;
stack<pbranch_t> g_loopstack;

bool CProgram::parseProgramRoot()
{
	bool ret_val;
	
	//Parse program name string
	
	//Parse Program Body block
	ret_val = m_pPgmBody->parseProgramBody();
	
	return ret_val;
}


bool CProgramBody::parseProgramBody()
{
	bool ret_val;

	vector<pFuncDec_t>::size_type index,max_size;
	pFuncDec_t pFunc;
	
	max_size = m_pvFuncList->size();
	//Parse Function Lists
	for(index=0; index < max_size;index++)
	{
		pFunc = (*m_pvFuncList)[index];
		
		//Allocate new IR handle and store the handle in the global array
		IRlistArray = new struct list;
		
		IRlistArray->tempCount=0;
		IRlistArray->localCount=0;
		IRlistArray->m_labelCount=0;
		IRlistArray->pIRListHead=NULL;
		IRlistArray->pIRListTail=NULL;
	
		//Parse each function node to generate IR
		ret_val = pFunc->parseFunction(IRlistArray);

		if(!ret_val)
			return ret_val;
	}

	return ret_val;

}

bool CFuncDeclarationNode::parseFunction(pirHandle_t irHandle)
{

	bool ret_val;
	plist_t irlist;
	//pEntry_t pEntry;
	pIRNode_t node;
	stringstream ss;
	//Parse the function name and generate IR for that
	irlist = irHandle;
	
	irlist->m_pFuncName = &m_funcname;
			
	//New IRlist. Allocate
	if(!irlist->pIRListHead)
	{
		irlist->pIRListHead = new cIRNode(LABEL, IRopcodeTable[LABEL],m_funcname,string(""),string(""));
		irlist->pIRListTail = irlist->pIRListHead;
		
		(irlist->pIRListHead)->m_next = NULL;
		(irlist->pIRListHead)->m_prev = NULL;
		
		//Link
		node = new cIRNode(LINK,IRopcodeTable[LINK],string(""),string(""),string(""));
		node->m_next = NULL;
		irlist->pIRListHead->m_next = node;
		node->m_prev = irlist->pIRListHead;
		irlist->pIRListTail = node;
		
	}
	
	//Parse the function body
	ret_val = m_pfuncBody->parseFunctionBody(irHandle);
	
	return ret_val;
}

bool CFunctionBody::parseFunctionBody(pirHandle_t irHandle)
{
	bool ret_val;
	vector<CStatementNode*>::size_type max_size;
	signed int index;
	pStatement_t stmt;
	
	max_size = m_pStmtList->size();
	
	for(index = (max_size-1); index >= 0;index--)
	{
		//Generate IR for each statement
		stmt = (*m_pStmtList)[index];
		ret_val = stmt->parseStatement(irHandle);
	}

	return ret_val;
}

bool CAssignmentStatement::parseStatement(pirHandle_t irHandle)
{
	bool ret_val;
	string result;
	plist_t irlist;	
	pIRNode_t node;
	opcode_t opcode;
	constType_t var_type;
	symdata_t symdata;
	
	irlist = irHandle;	
	
	//Evaluate the Right Hand Expression
	ret_val = m_prvalue->parseExpression(irHandle);
	
	if(!ret_val)
		return ret_val;
	
	//Obtain the Right Hand Result
	result=m_prvalue->getResult();
	
	ret_val=m_pSymtab->getsymboldata(m_lvalue, &symdata);

	if(!ret_val)
		return ret_val;	
	
	//Generate the IR for the whole Assignment statement
	if(symdata.type == INT_TYPE)
	{
		opcode = STOREI;
	}
	else if(symdata.type == FLOAT_TYPE)
	{
		opcode = STOREF;
	}
	
	if(irlist->pIRListHead)
	{
		node = new cIRNode(opcode,IRopcodeTable[opcode],result, string(""), m_lvalue);
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
	}
	else
	{
		cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method";
		ret_val =false;
		return ret_val;		
	}	
	
	return ret_val;
}

bool CReadWriteStatement::parseStatement(pirHandle_t irHandle)
{
	bool ret_val=true;
	vector<std::string*>::size_type index, max_size;
	constType_t var_type;
	string id_name;
	opcode_t opcode;
	plist_t irlist;	
	pIRNode_t node;
	symdata_t symdata;
	max_size = m_pvIdList->size();
	
	irlist = irHandle;

	
	//Loop through the id list and obtain id type from symbol table
	//If id is not available flag the error and exit
	for(index = 0; index < max_size; index++)
	{
		id_name = (*m_pvIdList)[index];

		ret_val=m_pSymtab->getsymboldata(id_name, &symdata);


		if(!ret_val)
		{
			//variable not found return false
			cout<<"BIGHORN ERROR: Identifier "<<id_name<<" Not Declared"<<endl;
			ret_val =false;
			return ret_val;
		}
		
		if(symdata.type == INT_TYPE)
		{
			opcode = ((m_type == READ))?READI:WRITEI;
		}
		else if(symdata.type == FLOAT_TYPE)
		{
			opcode = ((m_type == READ))?READF:WRITEF;
		}
		else if(symdata.type == STRING_TYPE)
		{
			opcode = WRITES; //only writes supported for string
		}
		
		if(irlist->pIRListHead)
		{
			node = new cIRNode(opcode,IRopcodeTable[opcode],string(""), string(""), id_name);
			node->m_next = NULL;
			irlist->pIRListTail->m_next = node;
			node->m_prev = irlist->pIRListTail;
			irlist->pIRListTail = node;
		}
		else
		{
			cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method"<<endl;
			ret_val =false;
			return ret_val;		
		}
		
	}
	
	return ret_val;
	
}


bool CIdentifierNode::parseExpression(pirHandle_t irHandle)
{
	bool ret_val=true;
	plist_t irlist;
	unsigned int tempcount;
	stringstream ss;
	opcode_t opcode;
	pIRNode_t node;
	string temp_var = "$T";	
	
	
	irlist = irHandle;
	
	//Get the Temp variable Global count
	tempcount = irHandle->getIRTempCount();
	
	//Create a Temp variable
	ss << tempcount;
	temp_var += ss.str();
	
	setResult(temp_var);
	
	result_type = m_vartype;
	
	//Is the var INT or FLOAT?
	opcode = (m_vartype == INT_TYPE)?STOREI:STOREF;
		
	//Create an IR node to store the variable in the IR node
	if(irlist->pIRListHead)
	{
		node = new cIRNode(opcode,IRopcodeTable[opcode],m_varname, string(""), temp_var);
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
	}
	else
	{
		cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method"<<endl;
		ret_val =false;
		return ret_val;		
	}
		
	return ret_val;
}


bool CLiteralNode::parseExpression(pirHandle_t irHandle)
{
	bool ret_val=true;
	plist_t irlist;
	unsigned int tempcount;
	string* litval;
	stringstream ss, sval;
	opcode_t opcode;
	pIRNode_t node;
	string temp_var = "$T";	
	
	irlist = irHandle;
	
	//Get the Temp variable Global count
	tempcount = irHandle->getIRTempCount();
	
	//Create a Temp variable
	ss << tempcount;
	temp_var += ss.str();
	
	setResult(temp_var);
	
	result_type = m_type;
	
	//Is the literal INT or FLOAT?	
	if(INT_TYPE == m_type)
	{
		opcode = STOREI;
		sval << m_intval;
	}
	else if(FLOAT_TYPE == m_type)
	{
		opcode = STOREF;
		sval << m_floatval;
	}
	
	litval = new string(sval.str());
	
	//Create an IR node to store the variable in the IR node
	if(irlist->pIRListHead)
	{
		node = new cIRNode(opcode,IRopcodeTable[opcode],*litval, string(""), temp_var);
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
	}
	else
	{
		cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method"<<endl;
		ret_val =false;
		return ret_val;		
	}
		
	return ret_val;
}


bool CExpressionNode::parseExpression(pirHandle_t irHandle)
{
	bool ret_val=true;
	plist_t irlist;	
	string op1, op2, result;
	unsigned int tempcount;	
	stringstream ss;
	opcode_t opcode;
	pIRNode_t node;
	string temp_var = "$T";
	symdata_t symdata;

	irlist = irHandle;
	
	//Evaluating the Expression AST is essentially a post order depth-first walk of the tree
	//Left Hand Expression First
	if(IDEN == m_pLeftExpression->m_exprtype)
	{
		//Identifier, so just get the identifier name
		op1 = m_pLeftExpression->getExprName();
		result_type = m_pLeftExpression->getResultType();
	}
	else
	{
		ret_val = m_pLeftExpression->parseExpression(irHandle);
		if(!ret_val)
			return ret_val;
		op1=m_pLeftExpression->getResult();

		if(LIT == m_pLeftExpression->m_exprtype)
		{
			result_type = m_pLeftExpression->getResultType();
		}
	}
	
	//Right Hand Expression Next
	if(IDEN == m_pRightExpression->m_exprtype)
	{
		op2 = m_pRightExpression->getExprName();
	}
	else
	{
		ret_val = m_pRightExpression->parseExpression(irHandle);
		if(!ret_val)
			return ret_val;		
		op2=m_pRightExpression->getResult();
	}

	//Get the total expression result type from the left sub-expression
	if(IDEN==m_pLeftExpression->m_exprtype)
	{

		ret_val=m_pSymtab->getsymboldata(m_pLeftExpression->getExprName(), &symdata);

		result_type = (constType_t)symdata.type;	
		
		if(result_type == UNKNOWN)
		return false;
	}
	else
	{
		result_type = m_pLeftExpression->getResultType();
	}
	//Find the operation
	switch(m_oper)
	{
		case '+' :  opcode = (result_type == INT_TYPE)?ADDI:ADDF;
					break;
		case '-' :  opcode = (result_type == INT_TYPE)?SUBI:SUBF;
					break;
		case '*' :  opcode = (result_type == INT_TYPE)?MULTI:MULTF;
					break;
		case '/' :  opcode = (result_type == INT_TYPE)?DIVI:DIVF;
					break;
		default:
			ret_val = false;
			return ret_val;
	}
	
	//Generate the TempVariable to store the result
	tempcount = irHandle->getIRTempCount();
	
	//Create a Temp variable
	ss << tempcount;
	temp_var += ss.str();
	
	setResult(temp_var);
	
	//Now Generate the IR for this current expression
	if(irlist->pIRListHead)
	{
		node = new cIRNode(opcode,IRopcodeTable[opcode],op1, op2, temp_var);
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
	}
	else
	{
		cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method"<<endl;
		ret_val =false;
		return ret_val;		
	}

	return ret_val;
}

bool CConditionNode::parseCondition(pirHandle_t irHandle)
{
	bool ret_val = true;
	
	if(1 == m_plExpression->m_exprtype)
	{
		//Identifier, so just get the identifier name
		m_op1 = m_plExpression->getExprName();
		m_opr_type = m_plExpression->getResultType();
	}
	else
	{
		ret_val = m_plExpression->parseExpression(irHandle);
		if(!ret_val)
			return ret_val;
		m_op1=m_plExpression->getResult();
		
		if(2 == m_plExpression->m_exprtype)
		{
			m_opr_type = m_plExpression->getResultType();
		}		
	}
	
	//Right Hand Expression Next
	if(1 == m_prExpression->m_exprtype)
	{
		m_op2 = m_prExpression->getExprName();
	}
	else
	{
		ret_val = m_prExpression->parseExpression(irHandle);
		if(!ret_val)
			return ret_val;		
		m_op2=m_prExpression->getResult();
	}
	
	//Find the operation
	switch(m_oper)
	{
		case '<' :  m_opcode = LT;
					break;
		case '>' :  m_opcode = GT;
					break;
		case '=' :  m_opcode = EQ;
					break;
		case '!' :  m_opcode = NE;
					break;
		default:
			ret_val = false;
			return ret_val;
	}

	return ret_val;

}

bool CIfStatement::parseStatement(pirHandle_t irHandle)
{
	bool ret_val=true;
	plist_t irlist;	
	unsigned int labelcnt;
	pIRNode_t node;
	string temp;
	int jtarget;



	irlist = irHandle;

	//Push Branch Label
	pbranch_t tgt= new branch_s;
	if(m_pelsenode)
	{
		tgt->inter_target = irlist->getIRLabelCount();
		tgt->exit_target = irlist->getIRLabelCount();
	}
	else
		tgt->exit_target = irlist->getIRLabelCount();

	g_branchstack.push(tgt);
	
	//Process the condition first
	ret_val = m_pcond->parseCondition(irHandle);

	if(m_pelsenode)
		jtarget = (g_branchstack.top())->inter_target;
	else
		jtarget = (g_branchstack.top())->exit_target;

	temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << jtarget))->str();
	
	if(irlist->pIRListHead)
	{
		node = new cIRNode(notop(m_pcond->m_opcode),IRopcodeTable[notop(m_pcond->m_opcode)],m_pcond->m_op1, m_pcond->m_op2, string("label" + temp));
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
		node->opr_type=m_pcond->m_opr_type;
	}
	else
	{
		cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method"<<endl;
		ret_val =false;
		return ret_val;		
	}

	ret_val = m_pifnode->parseIf(irHandle);

	if(!ret_val)
	return ret_val;

	if(m_pelsenode)
	{

		temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << tgt->exit_target))->str();
		node = new cIRNode(JUMP,IRopcodeTable[JUMP],string("label" + temp), string(""), string(""));
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;


		temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << jtarget))->str();
		node = new cIRNode(LABEL,IRopcodeTable[LABEL],string("label" + temp), string(""), string(""));
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;

		ret_val = m_pelsenode->parseElse(irHandle);
		if(!ret_val)
		return ret_val;

		temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << tgt->exit_target))->str();
		node = new cIRNode(LABEL,IRopcodeTable[LABEL],string("label" + temp), string(""), string(""));
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;

	}
	else
	{
		temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << jtarget))->str();
		node = new cIRNode(LABEL,IRopcodeTable[LABEL],string("label" + temp), string(""), string(""));
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
	}

	//Pop Branch Label
	tgt = g_branchstack.top();
	g_branchstack.pop();
	delete tgt;	

	return ret_val;
}

bool CIfNode::parseIf(pirHandle_t irHandle)
{

	bool ret_val=true;
	vector<pStatement_t>::size_type max_size;
	pStatement_t pstmt;
	signed int index;

	max_size = m_pstmtlist->size();
	
	//Run through the statements in if block
	for(index = (max_size-1); index >= 0;index--)
	{
		pstmt = (*m_pstmtlist)[index];
		ret_val = pstmt->parseStatement(irHandle);
		if(!ret_val)
		return ret_val;
	}
	
	return ret_val;
}

bool CElseNode::parseElse(pirHandle_t irHandle)
{
	bool ret_val=true;
	pStatement_t pstmt;
	vector<pStatement_t>::size_type max_size;
	signed int index;
	
	max_size = m_pstmtlist->size();
	
	//Generate else statements
	for(index = (max_size-1); index >= 0;index--)
	{
		pstmt = (*m_pstmtlist)[index];
		ret_val = pstmt->parseStatement(irHandle);
		if(!ret_val)
		return ret_val;
	}

	return ret_val;
}

bool CRepeatUntilStatement::parseStatement(pirHandle_t irHandle)
{
	bool ret_val=true;
	plist_t irlist;	
	unsigned int top,out;
	pIRNode_t node;	
	string temp;
	pStatement_t pstmt;
	vector<pStatement_t>::size_type max_size;
	signed int index;
	
	irlist=irHandle;
	
	max_size = m_pstmtlist->size();

	pbranch_t tgt= new branch_s;
	tgt->entry_target = irlist->getIRLabelCount();
	tgt->inter_target = irlist->getIRLabelCount();
	tgt->exit_target = irlist->getIRLabelCount();
	g_branchstack.push(tgt);
	g_loopstack.push(tgt);


	temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << tgt->entry_target))->str();

	if(irlist->pIRListHead)
	{
		node = new cIRNode(LABEL,IRopcodeTable[LABEL],string("label" + temp), string(""), string(""));
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
	}
	else
	{
		cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method"<<endl;
		ret_val =false;
		return ret_val;		
	}
	
	//Generate Statement lists
	for(index = (max_size-1); index >= 0;index--)
	{
		pstmt = (*m_pstmtlist)[index];
		ret_val = pstmt->parseStatement(irHandle);
		if(!ret_val)
		return ret_val;
	}

	temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << tgt->inter_target))->str();
	
	node = new cIRNode(LABEL,IRopcodeTable[LABEL],string("label"+temp), string(""), string(""));
	node->m_next = NULL;
	irlist->pIRListTail->m_next = node;
	node->m_prev = irlist->pIRListTail;
	irlist->pIRListTail = node;

	m_pcond->parseCondition(irHandle);

	temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << tgt->entry_target))->str();

	node = new cIRNode(notop(m_pcond->m_opcode),IRopcodeTable[notop(m_pcond->m_opcode)],m_pcond->m_op1, m_pcond->m_op2, string("label" + temp));
	node->m_next = NULL;
	irlist->pIRListTail->m_next = node;
	node->m_prev = irlist->pIRListTail;
	irlist->pIRListTail = node;
	node->opr_type=m_pcond->m_opr_type;

	temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << tgt->exit_target))->str();
	
	node = new cIRNode(LABEL,IRopcodeTable[LABEL],string("label" + temp), string(""), string(""));
	node->m_next = NULL;
	irlist->pIRListTail->m_next = node;
	node->m_prev = irlist->pIRListTail;
	irlist->pIRListTail = node;

	//Pop Branch Label
	tgt = g_branchstack.top();
	g_branchstack.pop();
	g_loopstack.pop();
	delete tgt;	
	
	return ret_val;
}

bool CContinueStatement::parseStatement(pirHandle_t irHandle)
{
	bool ret_val = true;
	pIRNode_t node;		
	pbranch_t tgt;
	plist_t irlist;	

	tgt = g_loopstack.top();

	irlist = irHandle;

	string temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << tgt->inter_target))->str();

	if(irlist->pIRListHead)
	{
		node = new cIRNode(JUMP,IRopcodeTable[JUMP],string("label"+temp), string(""), string(""));
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
	}
	else
	{
		cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method"<<endl;
		ret_val =false;
	}


	return ret_val;
}

bool CBreakStatement::parseStatement(pirHandle_t irHandle)
{
	bool ret_val = true;
	pIRNode_t node;		
	pbranch_t tgt;

	plist_t irlist;	

	tgt = g_loopstack.top();

	irlist = irHandle;

	string temp = dynamic_cast <std::stringstream*>(&(std::stringstream() << tgt->exit_target))->str();

	if(irlist->pIRListHead)
	{
		node = new cIRNode(JUMP,IRopcodeTable[JUMP],string("label" + temp), string(""), string(""));
		node->m_next = NULL;
		irlist->pIRListTail->m_next = node;
		node->m_prev = irlist->pIRListTail;
		irlist->pIRListTail = node;
	}
	else
	{
		cout<<"BIGHORN ERROR: Bighorn Statement Not Inside a method"<<endl;
		ret_val =false;
	}


	return ret_val;
}

