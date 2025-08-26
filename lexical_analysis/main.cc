#include <FlexLexer.h>   // provided by flex

int yyFlexLexer::yywrap() { return 1; }

int main() {
    yyFlexLexer lexer;      // create scanner instance
    return lexer.yylex();   // run scanner (returns 0 at EOF)
}

