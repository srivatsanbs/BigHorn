//
//  bighornIR.cc
//  bighorn
//


#include "bighornIR.hh"
#include "bighornAST.hh"
#include <sstream>


#define __IS__TEMP(__X__) ((__X__)?((__X__[0]=='$')?1:0):0)
#define __IS__LITERAL__(__X__) ((__X__)?((((('0'-'0')<=(__X__[0]-'0'))&&((__X__[0]-'0')<=('9'-'0')))?1:0)):0)


extern pirHandle_t IRlistArray;
extern unsigned int g_IRTableCount;


string IRopcodeTable[] = {

	string("ADDI"),		/*ADDI*/
	string("ADDF"),		/*ADDF*/
	string("SUBI"),		/*SUBI*/
	string("SUBF"),		/*SUBF*/
	string("MULTI"),	/*MULTI*/
	string("MULTF"),	/*MULTF*/
	string("DIVI"),		/*DIVI*/
	string("DIVF"),		/*DIVF*/
	string("STOREI"),	/*STOREI*/
	string("STOREF"),	/*STOREF*/
	string("GE"),		/*GE*/
	string("LT"),		/*LT*/
	string("LE"),		/*LE*/
	string("GT"),		/*GT*/
	string("NE"),		/*NE*/
	string("EQ"),		/*EQ*/
	string("JUMP"),		/*JUMP*/
	string("LABEL"),	/*LABEL*/
	string("READI"),	/*READI*/
	string("READF"),	/*READF*/
	string("WRITEI"),	/*WRITEI*/
	string("WRITEF"),	/*WRITEF*/
	string("WRITES"),	/*WRITES*/	
	string("JSR"),		/*JSR*/
	string("PUSH"),		/*PUSH*/
	string("POP"),		/*POP*/
	string("RET"),		/*RET*/
	string("LINK")		/*LINK*/
};

string TinyopcodeTable[] = {

	string("var"),
	string("str"),
	string("label"),
	string("move"),
	string("addi"),
	string("addr"),
	string("subi"),
	string("subr"),
	string("muli"),
	string("mulr"),
	string("divi"),
	string("divr"),
	string("inci"),
	string("deci"),
	string("cmpi"),
	string("push"),
	string("pop"),
	string("jsr"),
	string("ret"),
	string("link"),
	string("unlnk"),
	string("cmpr"),
	string("jmp"),
	string("jgt"),
	string("jlt"),
	string("jge"),
	string("jle"),
	string("jeq"),
	string("jne"),
	string("sys readi"),
	string("sys readr"),
	string("sys writei"),
	string("sys writer"),
	string("sys writes"),
	string("sys halt"),
	string("end")
};


bool cParseAST::start_parseAST(void)
{
	return ((pProgram_t)m_pASTroot)->parseProgramRoot();
}

bool cParseAST::gen_all()
{
	gen_IR();
	genCode();
}

bool cParseAST::gen_IR()
{
	bool ret_val=true;;
	pIRNode_t node;
	
	node = IRlistArray->pIRListHead;
		
	while(node)
	{
		//Print the opcode
		cout<<";";
		cout<< node->m_pstrOpcode << " ";
		
		if(node->m_pstrOp1.compare(""))
			cout<< node->m_pstrOp1 << " ";

		if(node->m_pstrOp2.compare(""))
			cout<< node->m_pstrOp2 << " ";
		
		if(node->m_pstrresult.compare(""))
			cout<< node->m_pstrresult << " ";
		
		if(node->m_next)
			cout<< endl;
		
		node = node->m_next;
	}
	
	cout<<endl;

	return ret_val;

}

bool cParseAST::genCode()
{
	bool ret_val=true;;
		
	genPrologue();
	
	genBody();
	
	genEpilogue();
	
	return ret_val;
}


bool cParseAST::genPrologue(void)
{

	pSymtab psymtab;
	pSymdata pdata;
	map<string, symdata_t*>::iterator it;


	psymtab = ((pProgram_t)m_pASTroot)->m_pPgmBody->getSymTable();
		
	for(it=psymtab->hash_container.begin();it!=psymtab->hash_container.end();it++)
	{
		pdata = it->second;

		//print the tiny code
		if(!pdata->isfunc)
		{
			if(pdata->type!=STRING_TYPE)
			cout<<TinyopcodeTable[var] << " "<<pdata->name<<endl;
			else
			cout<<TinyopcodeTable[str] << " "<<pdata->name<<" "<<pdata->stringlit<<endl;
		}
	}
		
	//push return address
	cout<<TinyopcodeTable[push]<<endl;
		
	//push 4 registers
	cout<<TinyopcodeTable[push]<<" "<<"r0"<<endl;		
	cout<<TinyopcodeTable[push]<<" "<<"r1"<<endl;
	cout<<TinyopcodeTable[push]<<" "<<"r2"<<endl;	
	cout<<TinyopcodeTable[push]<<" "<<"r3"<<endl;	
		
	//jump to main
	cout<<TinyopcodeTable[jsr]<<" "<<"main"<<endl;
		
	//halt
	cout<<TinyopcodeTable[sys_halt]<<endl;
		
	return true;
	
}

bool cParseAST::genBody(void)
{
	bool ret_val = true;
	pirHandle_t IRList;
	pIRNode_t IRnode;	
	map<string,int> varTempMap;//Do I need to make this map permanent??
	unsigned int val;
	bool ismove;
	string op1,op2,mem_stack;
	tinycode_t tiny_code;
		
	IRList=IRlistArray;
	IRnode = IRList->pIRListHead;
		
				
	while(IRnode)
	{
		ismove=false;
		switch(IRnode->m_opcode)
		{
			case ADDI://3
			case ADDF:
			case SUBI:
			case SUBF:
			case MULTI:
			case MULTF:
			case DIVI:
			case DIVF:
			{
				switch(IRnode->m_opcode)
				{
					case ADDI:
						tiny_code=addi;
						break;
					case ADDF:
						tiny_code=addr;
						break;	
					case SUBI:
						tiny_code=subi;
						break;
					case SUBF:
						tiny_code=subr;
						break;								
					case MULTI:
						tiny_code=muli;
						break;								
					case MULTF:
						tiny_code=mulr;
						break;								
					case DIVI:
						tiny_code=divi;
						break;								
					case DIVF:
						tiny_code=divr;
						break;								
				}
				
				ret_val = AnalyzeOperand(IRList,&IRnode->m_pstrOp1, varTempMap, true,&ismove, &op1,&mem_stack);
						
				if(!ret_val)
				{
					cout<<"Invalid Operand for instruction"<<endl;
					break;
				}
						
				if(ismove)
				{
					cout<<TinyopcodeTable[move]<<" "<<mem_stack<<" "<<op1<<endl;	
				}
		
				ret_val = AnalyzeOperand(IRList,&IRnode->m_pstrOp2, varTempMap, false,&ismove, &op2,&mem_stack);
						
				if(!ret_val)
				{
					cout<<"Invalid Operand for instruction"<<endl;
					break;
				}
						
				//Frame tiny instruction. An additional move incas IRnode->op1 is a memrory location
				cout<<TinyopcodeTable[tiny_code]<<" "<<op2<<" "<<op1<<endl;
						
				//Map IR result to the tiny op1, which stores the result
				varTempMap[IRnode->m_pstrresult]=varTempMap[IRnode->m_pstrOp1];
				}
				break;
			case STOREI: //2
			case STOREF:	//2
				ret_val = AnalyzeOperand(IRList,&IRnode->m_pstrOp1, varTempMap, false,&ismove, &op1,&mem_stack);
						
				if(!ret_val)
				{
					cout<<"Invalid Operand for instruction"<<endl;
					break;
				}

				ret_val = AnalyzeOperand(IRList,&IRnode->m_pstrresult, varTempMap, false,&ismove, &op2,&mem_stack);
						
				if(!ret_val)
				{
					cout<<"Invalid Operand for instruction"<<endl;
					break;
				}
				
				//Frame tiny instruction
				cout<<TinyopcodeTable[move]<<" "<<op1<<" "<<op2<<endl;
				break;

			case GE:
			case LE:
			case NE:
			case EQ:
			case GT:			
			case LT:						
				{	
					tinycode_t opr =  (IRnode->opr_type = INT_TYPE)?cmpi:cmpr;
					if(GE==IRnode->m_opcode)
					{
						tiny_code=jge;
					}
					else if(LE==IRnode->m_opcode)
					{
						tiny_code=jle;

					}
					else if(NE==IRnode->m_opcode)
					{
						tiny_code=jne;						
					}
					if(GT==IRnode->m_opcode)
					{
						tiny_code=jgt;
					}
					else if(LT==IRnode->m_opcode)
					{
						tiny_code=jlt;

					}
					else if(EQ==IRnode->m_opcode)
					{
						tiny_code=jeq;						
					}						
						
					ret_val = AnalyzeOperand(IRList,&IRnode->m_pstrOp1, varTempMap, false,&ismove, &op1,&mem_stack);
					if(!ret_val)
					{
						cout<<"Invalid Operand for instruction"<<endl;
						break;
					}
					

					ret_val = AnalyzeOperand(IRList,&IRnode->m_pstrOp2, varTempMap, true,&ismove, &op2,&mem_stack);
					if(!ret_val)
					{
						cout<<"Invalid Operand for instruction"<<endl;
						break;
					}
					
					if(ismove)
					{
						cout<<TinyopcodeTable[move]<<" "<<mem_stack<<" "<<op2<<endl;
					}						
					
					

					cout<<TinyopcodeTable[opr]<<" "<<op1<<" "<<op2<<endl;
					cout<<TinyopcodeTable[tiny_code]<<" "<<IRnode->m_pstrresult<<endl;
						break;
				}
			case JUMP:
				{
					cout<<TinyopcodeTable[jmp]<<" "<<IRnode->m_pstrOp1<<endl;
					break;					
				}

			case LABEL:
				{
					cout<<TinyopcodeTable[label]<<" "<<IRnode->m_pstrOp1<<endl;
					break;
				}
			case READI:
				{
					cout<<TinyopcodeTable[sys_readi]<<" "<<IRnode->m_pstrresult<<endl;
					break;
				}				
			case READF:
				{
					cout<<TinyopcodeTable[sys_readr]<<" "<<IRnode->m_pstrresult<<endl;
					break;
				}				
			case WRITEI:
				{
					cout<<TinyopcodeTable[sys_writei]<<" "<<IRnode->m_pstrresult<<endl;
					break;
				}				
			case WRITEF:
				{
					cout<<TinyopcodeTable[sys_writer]<<" "<<IRnode->m_pstrresult<<endl;
					break;
				}
			case WRITES:
				{
					cout<<TinyopcodeTable[sys_writes]<<" "<<IRnode->m_pstrresult<<endl;
					break;

				}
			case JSR:
				{
					//Push the four registers for the final version
					//push 4 registers
					cout<<TinyopcodeTable[push]<<endl;//<<" "<<"r0"<<endl;		
					cout<<TinyopcodeTable[push]<<endl;//<<" "<<"r1"<<endl;
					cout<<TinyopcodeTable[push]<<endl;//<<" "<<"r2"<<endl;	
					cout<<TinyopcodeTable[push]<<endl;//<<" "<<"r3"<<endl;	
					
					cout<<TinyopcodeTable[jsr]<<" "<<IRnode->m_pstrOp1<<endl;
					
					cout<<TinyopcodeTable[pop]<<endl;//<<" "<<"r0"<<endl;		
					cout<<TinyopcodeTable[pop]<<endl;//<<" "<<"r1"<<endl;
					cout<<TinyopcodeTable[pop]<<endl;//<<" "<<"r2"<<endl;	
					cout<<TinyopcodeTable[pop]<<endl;//<<" "<<"r3"<<endl;	
											
					break;					
				}
			case PUSH:
				{
					if(IRnode->m_pstrOp1.compare(""))
					{
						ret_val = AnalyzeOperand(IRList,&IRnode->m_pstrOp1, varTempMap,false,&ismove, &op1,&mem_stack);
						if(!ret_val)
						{
							cout<<"Invalid Operand for instruction"<<endl;
							break;
						}
					
						//Generate PUSH instruction
						cout<<TinyopcodeTable[push]<<" "<<op1<<endl;	
					}
					else
						cout<<TinyopcodeTable[push]<<endl;	

				
				}
				break;
			case POP:
				{
					if(IRnode->m_pstrOp1.compare(""))
					{
						ret_val = AnalyzeOperand(IRList,&IRnode->m_pstrOp1, varTempMap,false,&ismove, &op1,&mem_stack);
						if(!ret_val)
						{
							cout<<"Invalid Operand for instruction"<<endl;
							break;
						}
					
						//Generate POP instruction
						cout<<TinyopcodeTable[pop]<<" "<<op1<<endl;	
					}
					else
						cout<<TinyopcodeTable[pop]<<endl;	

				}
				break;
			case RET:
				{
					cout<<TinyopcodeTable[unlnk]<<" "<<op1<<endl;	
					cout<<TinyopcodeTable[ret]<<" "<<op1<<endl;	

				}
				break;
			case LINK:
				{					
					cout<<TinyopcodeTable[tiny_link]<<" "<<"0"<<endl;
					break;
				}			
			default:
				ret_val=false;
				break;
			}
		
			if(!ret_val)
			return ret_val;
			
			IRnode=IRnode->m_next;
		}
	
	return ret_val;

}


bool cParseAST::genEpilogue(void)
{

	//As of now do nothing, except generate end statement	
	cout<<TinyopcodeTable[end]<<endl;
	return true;

}

bool cParseAST::AnalyzeOperand(pirHandle_t IRList,string* inp,map<string,int>& varmap, bool isop1, bool *move,string* op,string* var)
{

	stringstream ss;
	bool result = false;
	int val;
	*move=false;
	char* tempstr = (char*)inp->c_str();
		
	if(__IS__TEMP(tempstr))
	{
		val = varmap.count(*inp);
		if(val == 1)
		{
			*op = "r";
			ss<<varmap[*inp];
			*op +=ss.str();
		}
		else if(!val)
		{
			//else generate the temp and map it to IRNode op2
			*op = "r";
			ss<<IRList->getRegCount();
			*op +=ss.str();								
			varmap[*inp] = IRList->m_regCount;
		}
		result = true;
	}
	else if(__IS__LITERAL__(tempstr))
	{
			*op=*inp;
			result=true;
	}
	else if(isop1)//not temp, not literal, but 1st operand
	{
		*var = *inp;
		*op = "r";
		ss<<IRList->getRegCount();
		*op +=ss.str();								
		varmap[*inp] = IRList->m_regCount;
		*move=true;
		result=true;
	}
	else //not temp, not literal, but target just return back
	{
		*op = *inp;
		result = true;
	}
	return result;
}

