/********************************************************************/
/*                The BigHorn Compiler                              */
/*                                                                  */
/* Lexer: This is the Lexical analyzer for the BigHorn compiler.    */
/*        It uses the Lex tool generate the Lexical analyzer code   */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/********************************************************************/
/*Pointer for yytext*/
%pointer

%{

#include "bighornAST.hh"
#include "bighorn.tab.hh"
#include "bighornIR.hh"

using namespace bighorn;    
typedef bighorn::parser::token token_t;

#define YY_DECL int bighornlex(bighorn::parser::semantic_type* yylval)

extern pProgram_t g_pASTroot;

%}

%%
PROGRAM							{return token_t::PROGRAM;}

BEGIN							{return token_t::tBEGIN;}
END							{return token_t::END;}
FUNCTION						{return token_t::FUNCTION;}
READ							{return token_t::READ;}
WRITE							{return token_t::WRITE;}
IF							{return token_t::IF;}						
THEN							{return token_t::THEN;}
ELSE							{return token_t::ELSE;}
ENDIF							{return token_t::ENDIF;}
REPEAT							{return token_t::REPEAT;}
UNTIL							{return token_t::UNTIL;}
CONTINUE						{return token_t::CONTINUE;}
BREAK							{return token_t::BREAK;}
RETURN							{return token_t::RETURN;}


INT							{yylval->integerVal = token_t::INT;
							return token_t::INT;}
VOID							{yylval->integerVal = token_t::VOID;
							return token_t::VOID;}
STRING							{yylval->integerVal = token_t::STRING;
							return token_t::STRING;}
FLOAT							{yylval->integerVal = token_t::FLOAT;
							return token_t::FLOAT;}

"+"|"-"|"*"|"/"|"="|"<"|">"|"("|")"|";"|","   		{	                     
                                                        	yylval->integerVal = yytext[0];
                                                        	return yytext[0];
                                                        }

"!="							{return token_t::NOTEQUAL;}

":="							{return token_t::ASSIGNMENT;}

[a-zA-Z][a-zA-Z0-9]*{1,31}      			{
					                	yylval->stringVal = strdup(yytext);
				                       		return token_t::IDENTIFIER;
			        			}


[0-9]+                         				{
                                    				yylval->integerVal = atoi(yytext);
                                    				return token_t::INTLITERAL;
			                                }

[0-9]*\.[0-9]+                  			{
                                   				yylval->floatVal = strtod(yytext, NULL);
                                    				return token_t::FLOATLITERAL;                                			
							}


\"([^\"])*\"{0,80}              			{	                     
                                    				yylval->stringLit = strdup(yytext);
                                    				return token_t::STRINGLITERAL;
			                                }

[\t\n ]                         			;

--.*                            			;

[:space:]|\n                            		{}

.							{
							    printf("ERROR: Invalid Token \"%s\".EXITING SCANNER!\n",yytext);
							    exit(1);
							}
%%

int main(int argc, char** argv)
{
	FILE* pFile=0;
	int ret_val;
    	bighorn::parser h_parser;
	cParseAST* parseAST;
	
	if(argc <= 1)
	{
		printf("Format:micro <filename>\n");
		exit(1);
	}

	pFile  = fopen(argv[1], "r");

	if(!pFile)
	{
		printf("File Not Found:%s", argv[1]);
		exit(0);
	}

	yyin = pFile;

	ret_val = h_parser.parse();
	if(0 == ret_val)
	{
		parseAST = new cParseAST(g_pASTroot);
		if(parseAST->start_parseAST())
			parseAST->gen_all();
	}
	else
	{
		printf("Not accepted\n");
	}
	return 0;
}

void yyerror (char const *s)
{
    //printf ("%s\n", s);
    printf("Not accepted\n");
    exit(1);
}

