%{
#include"parser.h"
%}

%option noyywrap

%%

\r\n {return END;}

"<<<"\r\n {return DATAEND;}

\r\n(.|\n)+/"<<<"\r\n {return DATA;}

AT/"+" {return AT;}

"=" {return EQUAL;}

"?" {return QST;}

"+"[A-Z]+ {return COMMAND;}

"="[0-9a-zA-Z".""=""_"]+/"," {return STARTPARAM;}

","[0-9a-zA-Z".""=""_"]+/"," {return PARAM;}

","[0-9a-zA-Z".""=""_"]+/\r\n {return LASTPARAM;}

"="[0-9a-zA-Z".""=""_"]+/\r\n {return SINGLEPARAM;}

. {return ERROR;}

%%
