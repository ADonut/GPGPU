
%{
//
// Note: it seems like it should be as simple as just using yylineno
// from the parser, but, since we need to do lookahead for a number of rules,
// that would return the wrong line number in many cases.  Instead, we keep
// track of the line number on a per rule basis by adding a YY_USER_ACTION
// hook.
//

extern YYLTYPE yyloc;

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; 
%}

%option yylineno
%option nounput

NUM							[0-9]
ALPHA						[A-Za-z]
ALPHANUM					[A-Za-z0-9]
HEXDIGIT					[A-Za-z0-9]

%%

;[^\r\n]*					{ /* Comment */ }

pc							{  
								yylval.reg.isFloat = 0;
								yylval.reg.isVector = 0;
								yylval.reg.index = 31;
								return TOK_REGISTER; 
							}
[fi]{NUM}+					{ 
								yylval.reg.isFloat = (yytext[0] == 'f');
								yylval.reg.isVector = 0;
								yylval.reg.index = atoi(yytext + 1);
								return TOK_REGISTER; 
							}
[vs]{NUM}+					{ 
								yylval.reg.isFloat = 0;
								yylval.reg.isVector = (yytext[0] == 'v');
								yylval.reg.index = atoi(yytext + 1);
								return TOK_REGISTER; 
							}
[vs][fi]{NUM}+				{ 
								yylval.reg.isVector = (yytext[0] == 'v');
								yylval.reg.isFloat = (yytext[1] == 'f');
								yylval.reg.index = atoi(yytext + 2);
								return TOK_REGISTER; 
							}
"=="						{ return TOK_EQUAL_EQUAL; }
">="						{ return TOK_GREATER_EQUAL; }
"<="						{ return TOK_LESS_EQUAL; }
"<>"						{ return TOK_NOT_EQUAL; }
"<<"						{ return TOK_SHL; }
">>"						{ return TOK_LSR; }
">>>"						{ return TOK_ASR; }
"&~"						{ return TOK_AND_NOT; }
[\(\)\{\}\~\=\,\[\]\+\-\*\/\&\|\^\<\>]		{ return yytext[0]; }
-?{NUM}+"."{NUM}+			{ 
								yylval.floatval = strtof(yytext, NULL);
								return TOK_FLOAT_LITERAL; 
							}
-?{NUM}+					{ 
								yylval.intval = atoi(yytext);
								return TOK_INTEGER_LITERAL; 
							}
0x{HEXDIGIT}+				{
								// Hexadecimal string
								yylval.intval = strtol(yytext + 2, NULL, 16);
								return TOK_INTEGER_LITERAL;
							}
mem_[bsl][x]?				{
								strcpy(yylval.str, yytext);
								return TOK_MEMORY_SPECIFIER;
							}
mem_linked					{
								strcpy(yylval.str, yytext);
								return TOK_MEMORY_SPECIFIER;
							}
\"[^"]*\"					{
								strcpy(yylval.str, yytext + 1);
								yylval.str[strlen(yylval.str) - 1] = '\0';
								return TOK_LITERAL_STRING;
							}
nop							{ return TOK_NOP; }
[_A-Za-z][_A-Za-z0-9]+ 		{ 
								struct Symbol *sym = lookupSymbol(yytext);
								if (sym == NULL)
									sym = createSymbol(yytext, SYM_LABEL, 0);
								
								yylval.sym = sym;
								if (sym->type == SYM_KEYWORD)
									return TOK_KEYWORD;
								else if (sym->type == SYM_CONSTANT)
									return TOK_CONSTANT;
								else
									return TOK_IDENTIFIER; 
							}
\.word						{ return TOK_WORD; }
\.short						{ return TOK_SHORT; }
\.byte						{ return TOK_BYTE; }
\.float						{ return TOK_FLOAT; }
\.string					{ return TOK_STRING; }
[ \t\r\n]					{ }