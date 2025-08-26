%{
    #include <string>
    #include <cstdio>
    #include <cstdlib>
    #include <cstdarg>
    #include <vector>
    #include <algorithm>
    #include <variant>

    #include "tokens.h"

    #define TRUE 1
    #define FALSE 0

    using namespace std;
    using YYSTYPE = std::variant<int, std::string>;

    extern YYSTYPE yylval;

    YYSTYPE yylval;
    FILE *yyin;

    bool anyErrors = false;

    static string fileName = "";
    static int lineNum = 1;
    int EM_tokPos = 0;

    static vector<int> lineStarts;

    static inline void ensureLineStartsInitialized()
    {
        if (lineStarts.empty()) lineStarts.push_back(0);
    }

    void EM_newline()
    {
        ++lineNum;
        ensureLineStartsInitialized();
        lineStarts.push_back(EM_tokPos);
    }

    void EM_error(int pos, const char *message, ...)
    {
        va_list ap;
        ensureLineStartsInitialized();

        anyErrors = true;

        int j = static_cast<int>(upper_bound(lineStarts.begin(), lineStarts.end(), pos - 1) - lineStarts.begin()) - 1;

        if (!fileName.empty()) {
            std::fprintf(stderr, "%s:", fileName.c_str());
        }

        if (j >= 0) {
            int num = j + 1;
            int col = pos - lineStarts[j];
            std::fprintf(stderr, "%d.%d: ", num, col);
        }

        va_start(ap, message);
        std::vfprintf(stderr, message, ap);
        va_end(ap);

        std::fprintf(stderr, "\n");
    }

    void EM_reset(const std::string &fname)
    {
        anyErrors = false;
        fileName = fname;
        lineNum = 1;
        lineStarts.clear();
        lineStarts.push_back(0);

        yyin = std::fopen(fname.c_str(), "r");
        if (!yyin) {
            EM_error(0, "cannot open");
            std::exit(1);
        }
    }

    inline void EM_reset(const char *fname)
    {
        EM_reset(std::string(fname));
    }

    void adjust() {
    }
%}


%%
" "	 {adjust(); continue;}

"for"  	 {adjust(); return FOR;}
"while"  	 {adjust(); return WHILE;}
"if" {adjust(); return IF;}
"else" {adjust(); return ELSE;}
"or" {adjust(); return OR;}
"function" {adjust(); return FUNCTION;}
"type" {adjust; return TYPE;}
"nil" {adjust; return NIL;}
"var" {adjust(); return VAR;}
"then" {adjust(); return THEN;}
"let" {adjust(); return LET;}
"break" {adjust(); return BREAK;}
"do" {adjust(); return DO;}
"end" {adjust(); return END;}
"in" {adjust(); return IN;}
"of" {adjust(); return OF;}
"to" {adjust(); return TO;}
"type" {adjust(); return TYPE;}

[a-z][a-z0-9]*  {adjust(); yylval = string(yytext); return ID;}
[0-9]+	 {adjust(); yylval = stoi(string(yytext)); /*cout << get<int>(yylval) << endl;*/ return INT;}

"\n"	 {adjust(); EM_newline(); continue;}


","	 {adjust(); return COMMA;}

.	 {adjust(); EM_error(EM_tokPos,"Illegal token");}
%%
