/*** C/C++ Declarations ***/
%{ 
#include "stdio.h"
#include "string"
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <map>

#include "bighornAST.hh"
#include "sym_tab.hh"

%}

/*Bison Directives*/

%defines
%require "2.3"
/*%error-verbose*/
%skeleton "lalr1.cc"
%name-prefix="bighorn"



/*Debug options*/
/*%debug*/

%union {
//Basic types
int integerVal;
float floatVal;
char* stringVal;
char* stringLit;

//AST types
pNode_t pNode;
pProgram_t pProgram;
pProgramBody_t pProgramBody;
pFuncDec_t pFuncdecl;
pSymtab pDecl;
vector<pFuncDec_t>* pvFuncList;
vector<pStatement_t>* pvStmtList;
pFuncBody_t pFuncBody;
pStatement_t pStmt;
pAssignment_t pAssgmt;
vector<std::string>* pvString;
pExpression_t pExpr;
p_readWritestmt_t pRW;
pIfStatement_t pIf;
pIfNode_t pThen;
pElseNode_t pElse;
pIfNode_t pAugThen;
pElseNode_t pAugElse;
pCondition_t pCond;
pRepeatUntilStatement_t pRepeat;

//Dummy 
void* dtype;
}

%token PROGRAM
%token tBEGIN END
%token FUNCTION
%token READ WRITE
%token IF THEN ELSE ENDIF
%token RETURN
%token REPEAT UNTIL
%token CONTINUE BREAK
%token FLOAT 
%token <integerVal> INT 
%token <integerVal> VOID 
%token <integerVal> STRING
%token <stringVal>  IDENTIFIER
%token <stringLit>  STRINGLITERAL
%token <floatVal>   FLOATLITERAL
%token <integerVal> INTLITERAL
%token ASSIGNMENT ":="
%token NOTEQUAL "!="
%token EOL


/*Semantic Types*/
%type<stringLit>  str
%type<stringVal> id
%type<integerVal> var_type
%type<integerVal> any_type
%type<integerVal> mulop addop compop '+' '-' '*' '/' '<' '>' '=' '!' "!="

%type<pProgram> program
%type<pProgramBody> pgm_body
%type<pvFuncList> func_declarations
%type<pDecl> decl
%type<pFuncdecl> func_decl
%type<pFuncBody> func_body
%type<pvStmtList> stmt_list
%type<pStmt> stmt
%type<pStmt> base_stmt
%type<pAssgmt> assign_stmt
%type<pAssgmt> assign_expr
%type<pvString> arg_id_list
%type<pExpr> expr
%type<pExpr> expr_tail
%type<pExpr> factor
%type<pExpr> postfix_expr
%type<pExpr> factor_tail
%type<pExpr> primary
%type<pRW> write_stmt
%type<pRW> read_stmt
%type<pIf> if_stmt
%type<pElse> else_part
%type<pCond> cond
%type<pElse> elseblk
%type<pThen> thenblk
%type<pStmt> aug_stmt
%type <pIf> aug_if_stmt
%type <pvStmtList> aug_stmt_list
%type <pRepeat> repeat_stmt
%type <pAugThen>aug_then_blk
%type <pAugElse> aug_else_part 
%type <pAugElse> aug_else_blk 

/*Precedence and Associativity*/
%left '*' '/'
%left '+' '-'

//Global Declerations
%{
extern int yylex(bighorn::parser::semantic_type*);

CSymtab* pSymTabRoot=NULL;
CSymtab* pCT;
int gCurrentType;
int blockcount=0;

vector<pFuncDec_t>* g_pFuncList = new vector<pFuncDec_t>;
vector<pStatement_t>* g_pStmtList = NULL;
vector<pStatement_t>* g_paugStmtList = NULL;

pProgram_t g_pASTroot;
%}

/*Rules*/
%%
/*Program*/
program			: pgmhead id tBEGIN pgm_body END				{g_pASTroot = new CProgram(string($2),$4);};


pgmhead			: PROGRAM							{//Create Symbol Table Here. Begin global scope
											pSymTabRoot=new CSymtab(GLOBAL_SCOPE);
											pSymTabRoot->type_name="GLOBAL";
											pCT=pSymTabRoot;
											};


id              	: IDENTIFIER							{//action create identifier name semantic record
											 $$=yylval.stringVal;
											};

pgm_body		: decl func_declarations					{$$ = new CProgramBody($1,$2);};

/*Decleration*/
decl			:								{$$ = pCT;};
			  | string_decl decl						{$$ = pCT;}
			  | var_decl  decl						{$$ = pCT;};


str             	: STRINGLITERAL							{$$=yylval.stringLit;};


string_decl     	: STRING id ":=" str ';'					{//Store string in symbol table
											pSymdata data= new symdata_t;
											data->type = STRING_TYPE;
											data->isfunc=false;
											data->stringlit = $4;
											data->name=$2;

											if((pCT->add(data->name,data)) == false)
											{
												cout<<"DECLARATION ERROR "<<data->name<<endl;
												//cleanup;
												delete data;
												exit(1);
											};

											//Check for shadow errors
											if(pCT->check_for_shadow_entries(data->name))
											  cout<<"SHADOW WARNING "<<data->name<<endl;
											};


var_decl        	: var_type id_list';';								
var_type	    	: FLOAT								{//Return the type
											$$=yylval.integerVal;
											gCurrentType=FLOAT_TYPE;
											};

			  | INT								{//Return the type
											$$=yylval.integerVal;
											gCurrentType=INT_TYPE;
											};


any_type        	: var_type | VOID						{//Return the type
											$$=yylval.integerVal;
											gCurrentType=VOID_TYPE;
											};


id_list         	:id 								{//Push the identifier into the hash table
											pSymdata data= new symdata_t;
											data->type=gCurrentType;
											data->name=$1;
											data->isfunc=false;
											if((pCT->add(data->name,data)) == false)
											{
												cout<<"DECLARATION ERROR "<<data->name<<endl;
												//cleanup;
												delete data;
												exit(1);
											}
											else if((pCT->sym_parent) && !(data->name.compare(pCT->sym_parent->type_name)))
											{
												cout<<"DECLARATION ERROR "<<data->name<<endl;
												//cleanup;
												delete data;
												exit(1);
											}
											//Check for shadow errors
											if(pCT->check_for_shadow_entries(data->name))
											  cout<<"SHADOW WARNING "<<data->name<<endl;
											};

											
			 | id_list ',' id 						{//Push the identifier into the hash table
											pSymdata data= new symdata_t;
											data->type=gCurrentType;
											data->name=$3;
											data->isfunc=false;
											if((pCT->add(data->name,data)) == false)
											{
												cout<<"DECLARATION ERROR "<<data->name<<endl;
												//cleanup;
												delete data;
												exit(1);
											}
											else if((pCT->sym_parent) && !(data->name.compare(pCT->sym_parent->type_name)))
											{
												cout<<"DECLARATION ERROR "<<data->name<<endl;
												//cleanup;
												delete data;
												exit(1);
											}
											//Check for shadow errors
											if(pCT->check_for_shadow_entries(data->name))
											  cout<<"SHADOW WARNING "<<data->name<<endl;
											};

/* Function Paramater List */
param_decl_list 	: 
			  | param_decl param_decl_tail;


param_decl      	: var_type id							{//Push the identifier into the hash table
											pSymdata data= new symdata_t;
											data->type=$1;
											data->name=$2;
											data->isfunc=false;
											if((pCT->add(data->name,data)) == false)
											{
												cout<<"DECLARATION ERROR "<<data->name<<endl;
												//cleanup;
												delete data;
												exit(1);
											}
											//Check for shadow errors
											if(pCT->check_for_shadow_entries(data->name))
											  cout<<"SHADOW WARNING "<<data->name<<endl;
											};


param_decl_tail 	: 
			  | ',' param_decl param_decl_tail;



/* Function Declarations */
func_declarations	: 								{$$ = NULL;}
			  | func_declarations func_decl					{g_pFuncList->push_back($2);$$=g_pFuncList;}


funchead		: FUNCTION							{//Begin Function Scope
											pCT = new CSymtab(FUNCTION_SCOPE);
											pSymTabRoot->sym_child.insert(pSymTabRoot->sym_child.end(),pCT);
											pCT->sym_parent = pSymTabRoot;
											};

func_decl		: funchead any_type id '('param_decl_list ')' tBEGIN func_body END		{//End Function scope. Set Global scope
													//Start Symbol Table Actions
													pSymdata data= new symdata_t;
													data->type=$2;
													data->name=$3;
													data->isfunc=true;
													if((pSymTabRoot->add(data->name,data)) == false)
													{
														cout<<"DECLARATION ERROR "<<data->name<<endl;
														//cleanup;
														delete data;
														exit(1);
													}
													pCT->type_name=$3;
													pCT=pSymTabRoot;
													//End Symbol Table Actions
													$$ = new CFuncDeclarationNode(data->name, $8);
													};
												
func_body		: decl stmt_list						{$$ = new CFunctionBody($1,$2);g_pStmtList=NULL;};

/* Statement List*/
stmt_list       	: 								{$$ = NULL;}
			  | stmt stmt_list						{if(!g_pStmtList){
											g_pStmtList = new vector<pStatement_t>;
											g_pStmtList->insert(g_pStmtList->end(),$1);}
											else{
											g_pStmtList->insert(g_pStmtList->end(),$1);}
											$$=g_pStmtList;};

stmt			: base_stmt 							{$$=$1;}
			  | if_stmt 							{$$=$1;}
			  | repeat_stmt							{$$=$1;};

base_stmt            	: assign_stmt							{$$=$1;}
			  | read_stmt							{$$=$1;}
			  | write_stmt							{$$=$1;}
			  | return_stmt							{$$=NULL;};

/* Basic Statements */
assign_stmt     	: assign_expr ';'						{$$=$1;};

assign_expr     	: id ":=" expr							{$$ = new CAssignmentStatement(string($1),$3,pCT);};

read_stmt       	: READ '(' arg_id_list ')' ';'					{$$ = new CReadWriteStatement(true, $3,pCT);};

write_stmt      	: WRITE '(' arg_id_list ')' ';'					{$$ = new CReadWriteStatement(false, $3,pCT);};

arg_id_list		: id 								{$$ = new vector<std::string>;$$->push_back(string($1));}
			  | arg_id_list ',' id						{$$->push_back(string($3));};

return_stmt     	: RETURN expr ';';

/* Expressions */
expr            	: factor expr_tail						{if($2){$2->addLeftExp($1);$$=$2;}else{$$ = $1;}};

expr_tail       	: 								{$$ = NULL;}
			  | addop factor expr_tail					{if($3){$3->addLeftExp(new CExpressionNode($1,$2,pCT));$$=$3;}else{$$ = new CExpressionNode($1, $2,pCT);}};

factor          	: postfix_expr factor_tail					{if($2){$2->addLeftExp($1);$$=$2;}else{$$ = $1;}};

factor_tail     	: 								{$$ = NULL;}
			 | mulop postfix_expr factor_tail				{if($3){$3->addLeftExp(new CExpressionNode($1,$2,pCT));$$=$2;}else{$$ = new CExpressionNode($1, $2,pCT);}};

postfix_expr    	: primary 							{$$=$1;}
			  | call_expr							{$$=NULL;};

call_expr       	: id '(' expr_list ')';
expr_list       	: 
			  | expr expr_list_tail;
expr_list_tail  	: 
			  | ',' expr expr_list_tail;

primary         	: '('expr')' 							{$$=$2;}
			  | id								{$$= new CIdentifierNode(string($1));}
			  | INTLITERAL							{$$=new CLiteralNode(INT_TYPE, $<integerVal>1);}
			  | FLOATLITERAL						{$$=new CLiteralNode(FLOAT_TYPE, $<floatVal>1);};

addop           	: '+'								{$$=$1;} 
			  | '-'								{$$=$1;};
mulop           	: '*' 								{$$=$1;}
			  | '/'								{$$=$1;};

/* Complex Statements and Condition */
if_stmt         	: IF '(' cond ')' thenblk else_part ENDIF			{$$ = new CIfStatement($3,$5,$6);
											pCT = pCT->sym_parent;};


thenblk 		: thenkeyword decl stmt_list					{$$ = new CIfNode(pCT,$3);g_pStmtList=NULL;};


thenkeyword 		: THEN								{//Begin Then Scope
											std::stringstream s1;
											blockcount++;
											s1<<blockcount;
											CSymtab* temp = new CSymtab(BLOCK_SCOPE);
											temp->type_name = "BLOCK ";
											temp->type_name.append(s1.str());
											pCT->sym_child.insert(pCT->sym_child.end(),temp);
											temp->sym_parent = pCT;
											pCT = temp;
											};

else_part       	:								{$$=NULL;} 
			  | elseblk							{$$ = $1;
											pCT = pCT->sym_parent;
											};

elseblk			: elsekeyword decl stmt_list					{$$ = new CElseNode(pCT,$3);g_pStmtList=NULL;};


elsekeyword		: ELSE								{//Begin Else Scope
											blockcount++;
											std::stringstream s2;
											s2<<blockcount;
											CSymtab* temp = new CSymtab(BLOCK_SCOPE);
											temp->type_name = "BLOCK ";
											temp->type_name.append(s2.str());
											pCT->sym_child.insert(pCT->sym_child.end(),temp);
											temp->sym_parent = pCT;
											pCT = temp;
											};


cond            	: expr compop expr						{$$=new CConditionNode($1,$2,$3);};
compop          	: '<'								{$$ = '<';}
			  | '>'								{$$ = '>';}
			  | '='								{$$ = '=';}
			  | "!="							{$$ = '!';};

/* ECE 573 students use this version of repeat_stmt */
repeat_stmt       	:repeathead decl aug_stmt_list UNTIL '(' cond ')' ';'		{$$ = new CRepeatUntilStatement(pCT,$3,$6);
											g_paugStmtList = NULL;
											pCT = pCT->sym_parent;
											};


repeathead		:REPEAT								{//Begin Repeat Scope
											std::stringstream s3;
											blockcount++;
											s3<<blockcount;
											CSymtab* temp = new CSymtab(BLOCK_SCOPE);
											temp->type_name = "BLOCK ";
											temp->type_name.append(s3.str());
											pCT->sym_child.insert(pCT->sym_child.end(),temp);
											temp->sym_parent = pCT;
											pCT = temp;
											};

/* CONTINUE and BREAK statements. ECE 573 students only */
aug_stmt_list     	:								{$$=NULL;}
			 | aug_stmt aug_stmt_list					{if(!g_paugStmtList){
											g_paugStmtList = new vector<pStatement_t>;
											g_paugStmtList->insert(g_paugStmtList->end(),$1);}
											else{
											g_paugStmtList->insert(g_paugStmtList->end(),$1);}
											$$=g_paugStmtList;};


aug_stmt          	: base_stmt							{$$=$1;} 
			  | aug_if_stmt							{$$=$1;} 
			  | repeat_stmt 						{$$=$1;}
			  | CONTINUE ';' 						{$$=new CContinueStatement;}
			  | BREAK ';' 							{$$=new CBreakStatement;};

/* Augmented IF statements for ECE 573 students */
aug_if_stmt       	: IF '(' cond ')' aug_then_blk aug_else_part ENDIF		{$$ = new CIfStatement($3,$5,$6);
											pCT = pCT->sym_parent;};

aug_then_blk		: thenkeyword decl aug_stmt_list				{$$ = new CIfNode(pCT,$3);g_paugStmtList=NULL;};


aug_else_part     	:								{$$=NULL;}
			 | aug_else_blk 						{$$=$1;pCT = pCT->sym_parent;};

aug_else_blk		: elsekeyword decl aug_stmt_list				{$$=new CElseNode(pCT,$3);g_paugStmtList=NULL;};
%%

void bighorn::parser::error (const location_type& loc, const std::string& msg)
{
    //For Now Do Nothing
    //std::cerr << loc << ": " << msg << std::endl;
}

