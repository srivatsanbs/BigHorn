//
//  bighorn_types.h
//  bighorn
//


#ifndef __BIGHORNTYPES__
#define __BIGHORNTYPES__

#include <string>
#include <map>

using namespace std;

#define MAX_INDEX 255

typedef enum
{
	INT_TYPE=0,
	FLOAT_TYPE,
	STRING_TYPE,
	VOID_TYPE,
	LIST_TYPE,
	UNKNOWN
}constType_t;

typedef enum IRopcode
{
	ADDI=0,
	ADDF,
	SUBI,
	SUBF,
	MULTI,
	MULTF,
	DIVI,
	DIVF,
	STOREI,
	STOREF,
	GE,
	LT,
	LE,
	GT,
	NE,
	EQ,
	JUMP,
	LABEL,
	READI,
	READF,
	WRITEI,
	WRITEF,
	WRITES,
	JSR,
	PUSH,
	POP,
	RET,
	LINK
}opcode_t;


typedef enum Tinycode
{
	var, 
	str,
	label,
	move,
	addi,
	addr,
	subi,
	subr,
	muli,
	mulr,
	divi,
	divr,
	inci,
	deci,
	cmpi,
	push,
	pop,
	jsr,
	ret,
	tiny_link,
	unlnk,
	cmpr,
	jmp,
	jgt,
	jlt,
	jge,
	jle,
	jeq,
	jne,
	sys_readi,
	sys_readr,
	sys_writei,
	sys_writer,
	sys_writes,
	sys_halt,
	end
}tinycode_t;


typedef class cIRNode
{
public:
	opcode_t m_opcode;
	string m_pstrOpcode;
	string m_pstrOp1;
	string m_pstrOp2;
	string m_pstrresult;
	constType_t opr_type;
	
	
	cIRNode* m_next;
	cIRNode* m_prev;
	
	cIRNode(opcode_t opcd, string opcdstr, string op1, string op2, string res)
	{
		m_opcode = opcd;
		m_pstrOpcode = opcdstr;
		m_pstrOp1 = op1;
		m_pstrOp2 = op2;
		m_pstrresult = res;
		
		//Self-conatined node initially
		m_next = NULL;
		m_prev = NULL;
	}
	
}*pIRNode_t;

typedef struct list
{
	//Some context information too!!
	string* m_pFuncName;
	unsigned int tempCount;
	unsigned int localCount;
	unsigned int m_labelCount;
	unsigned int m_regCount;
		
	unsigned int getIRTempCount()
	{
		return ++tempCount;
	}	

	unsigned int getIRLocalCount()
	{
		return ++localCount;
	}	
	
	unsigned int getIRLabelCount()
	{
		return ++m_labelCount;
	}
	
	unsigned int getRegCount()
	{
		return ++m_regCount;
	}

	pIRNode_t pIRListHead;
	pIRNode_t pIRListTail;
}*plist_t,*pirHandle_t;
#endif
