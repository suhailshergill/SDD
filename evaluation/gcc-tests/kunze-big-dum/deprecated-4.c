xmas2(t,_,a )
char
*
a;
{
				return!

0<t?
t<3?

xmas2(-79,-13,a+
xmas2(-87,1-_,
xmas2(-86, 0, a+1 )


+a)):

1,
t<_?
xmas2( t+1, _, a )
:3,

xmas2 ( -94, -27+t, a )
&&t == 2 ?_
<13 ?

xmas2 ( 2, _+1, "%s %d %d\n" )

:9:16:
t<0?
t<-72?
xmas2( _, t,
"@n'+,#'/*{}w+/w#cdnr/+,{}r/*de}+,/*{*+,/w{%+,/w#q#n+,/#{l,+,/n{n+,/+#n+,/#;#q#n+,/+k#;*+,/'r :'d*'3,}{w+K w'K:'+}e#';dq#'l q#'+d'K#!/+k#;q#'r}eKK#}w'r}eKK{nl]'/#;#q#n'){)#}w'){){nl]'/+#n';d}rw' i;# ){nl]!/n{n#'; r{#w'r nc{nl]'/#{l,+'K {rw' iK{;[{nl]'/w#q#n'wk nw' iwk{KK{nl]!/w{%'l##w#' i; :{nl]'/*{q#'ld;r'}{nlwb!/*de}'c ;;{nl'-{}rw]'/+,}##'*}#nc,',#nw]'/+kd'+e}+;#'rdq#w! nr'/ ') }+}{rl#'{n' ')# }'+}##(!!/")
:
t<-50?
_==*a ?
putchar(31[a]):

xmas2(-65,_,a+1)
:
xmas2((*a == '/') + t, _, a + 1 )
:

0<t?

xmas2 ( 2, 2 , "%s")
:*a=='/'||

xmas2(0,

xmas2(-61,*a, "!ek;dc i@bK'(q)-[w]*%n+r3#l,{}:\nuwloca-O;m .vpbks,fxntdCeghiry")

,a+1);}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define ch(x) "/|\\"[sgn(x)+1]
#define sgn(x) (x>0?1:x?-1:0)
xmas1(){long a=0,b=0,c=0,d=0,i=0,j=0;char s[21][40]
;memset(s,' ',0x348);while(j++<0x15)s[j][-1]='\0';
srand(time(NULL));while(++i<<15){switch(rand()%7){
  case 0:c&&(c-=sgn(c)*(rand()%(2*c)));
  case 1:c--,c^=-~-~-~-~-~-~-~-8;break;
  case 2:d=18-~!c,c=-sgn(a)+d>>3;break;
  case 3:c=a=(c>0?1:-1)*(d=b=18);break;
  case 4:c=a=sgn(a)*(d=b*=7.0/9);break;
  case 5:sgn(c)*c>2&&(c-=3*sgn(c),d--);
}s[d][19+c]=ch(c);}while(i&21^21)puts(s[31&i++]);}

#include "stdio.h"
#define e 3
#define g (e/e)
#define h ((g+e)/2)
#define f (e-g-h)
#define j (e*e-g)
#define k (j-h)
#define l(x) tab2[x]/h
#define m(n,a) ((n&(a))==(a))

long tab1[]={ 989L,5L,26L,0L,88319L,123L,0L,9367L };
int tab2[]={ 4,6,10,14,22,26,34,38,46,58,62,74,82,86 };

main(m1,s) char *s; {
    int a,b,c,d,o[k],n/*=(int)s*/;
    if(m1==1){ char b[2*j+f-g]; main(l(h+e)+h+e,b); printf(b); }
    else switch(m1-=h){
        case f:
            a=(b=(c=(d=g)<<g)<<g)<<g;
            return(m(n,a|c)|m(n,b)|m(n,a|d)|m(n,c|d));
        case h:
            for(a=f;a<j;++a)if(tab1[a]&&!(tab1[a]%((long)l(n))))return(a);
        case g:
            if(n<h)return(g);
            if(n<j){n-=g;c='D';o[f]=h;o[g]=f;}
            else{c='\r'-'\b';n-=j-g;o[f]=o[g]=g;}
            if((b=n)>=e)for(b=g<<g;b<n;++b)o[b]=o[b-h]+o[b-g]+c;
            return(o[b-g]%n+k-h);
        default:
            if(m1-=e) main(m1-g+e+h,s+g); else *(s+g)=f;
            for(*s=a=f;a<e;) *s=(*s<<e)|main(h+a++,/*(char *)*/m1);
        }
}


subset(Q,O)char**O;{if(--Q){subset(Q,O);O[Q][0]^=0X80;for(O[0][0]=0;O[++O[0][0]]!=0;)if(O[O[0][0]][0]>0)puts(O[O[0][0]]);puts("----------");subset(Q,O);}}


typedef int a;

struct MyStruct1
{
  struct MyStruct2* x;
}
  ;
struct MyStruct2
{
  struct MyStruct1* x;
  a y;
  
}
  ;

struct Mystruct3
{
  a* x;
  a y;
}
  ;


/* Test __attribute__((deprecated)).  Test types without names.  */
/* Origin: Joseph Myers <jsm@polyomino.org.uk> */
/* { dg-do compile } */
/* { dg-options "" } */

struct { int a; } __attribute__((deprecated)) x; /* { dg-warning "type is deprecated" } */
typeof(x) y; /* { dg-warning "type is deprecated" } */
