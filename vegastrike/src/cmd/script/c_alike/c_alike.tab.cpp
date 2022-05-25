#include <string>
#include <stdio.h>

bool have_yy_error=false;
std::string parseCalike(char const *filename)
{
	return "";
}

int yylineno=0;
int yyerror(char *s){
  printf("(yy)error:  line %d text --\n",yylineno+1);

  return 1;
}

int yywrap(){
	return 1;
}


int yylex() {
	return 1;
}
