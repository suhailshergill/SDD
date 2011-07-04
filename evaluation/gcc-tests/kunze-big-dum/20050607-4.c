#include <stdio.h>
#include <malloc.h>
#include <setjmp.h>
#include <ctype.h>
#define new(PP) (PP *) malloc(sizeof(PP)) 
typedef struct q {
    jmp_buf ppp;
    long qq;
    struct q *P;
    struct q *p;
} 
PP;

PP *P;
int aaaaaa=2;
int aaaaaaa=1;

long qqq;


aaAaaa(aa,aaa)
char *aa;
char *aaa;
{
    char aaaa = 0;
    if ((((( aaa )))))
    {
        aaaa = *aa;
        *aa=0;
        aa+=strlen(aa+1);
        P =new(PP);
        P->P=P;
        P->p=P;
    }

    if ((((( !setjmp(P->ppp) ))))) 
        {
        if ((((( !isdigit(*aa) )))))
            longjmp(P->ppp,aaaaaaa);
        else {
            P->p->P = new(PP);
            P->p->P->P = P;
            P->p->P->p = P->p;
            P->p = P->p->P;

            P->qq = *aa--;
            P = P->p;
            aaAaaa(aa,0);
        }
    } else {
        if ( !aaaa&&!*aa ) 
           longjmp(P->p->ppp,aaaaaaa);

        if ((((( (P->qq=aaaa)<10     &&!
                 (isdigit(aaaa))     ||!
                 (isdigit(*aa)       ||!
                 *aa                        )))))) 
        {
            fprintf(stderr,"Usage %c%s <number>\n",
            (aaa[0]?7:aaaa),aaa+!aaa[0]);
            exit(1);
        }
    }
}


ppPppp(pp,ppp)
PP **pp, *ppp;
{
    int aa;
    if ((((( !(aa=setjmp(ppp->ppp))||aa==aaaaaa )))))
    {
        if ((((( *pp==ppp )))))
        {
            ppp = (*pp)->p;

            if ( qqq<47 ) return;
            if ( ppp->qq!=48 ) return;

            while ( ppp->qq==48 ) 
            {
                printf("%ld\n",qqq-45);
                *pp = ppp;
                ppp = ppp->p;
            }
            qqq -= 1;
            longjmp(ppp->ppp,aaaaaaa);
        } else {
            PP *p;

            ppPppp(pp,ppp->p);
            for (p=ppp;p!=*pp;p=p->p)
            {
                int qq=4;
                if ((((( qqq<47                            &&
                         (qq=0,p->qq+=p->p->qq-96)>=48-qqq ||
                         qqq>46                            &&
                         (p->qq-=p->p->qq)<0                   ))))) 
                {
                    p->qq += qqq+qq;
                    if ( p->p==P && qqq<=46 )
                    {
                        P->p->P = new(PP);
                        P->p->P->P = P;
                        P->p->P->p = P->p;
                        *pp = P = P->p = P->p->P;
                        P->qq = 48;
                    }

                    p->p->qq+=qq==0;
                    p->p->qq-=qq!=0;
                }
                else
                {
                    p->qq += 48;
                }
            }
            if ( ppp->P==P ) longjmp(ppp->ppp,aaaaaaa);
        }
    }
    else
    {
        qqq += 1; 

        while (48==P->qq )
        {
            P->P->p = P->p;
            P = P->p->P = P->P;

        }

        if ( ppp!=ppp->p->p || qqq<47 )
            longjmp(ppp->ppp,aaaaaa);
        else
        {
            printf("At most one remains\n");
            exit(0);
        }
    }
}


mindem(aaa,aaaa)
int aaa;
char **aaaa;
{
    aaAaaa(aaa==aaaaaaa?aaaa[0]:aaaa[1],aaaa[0]);
    qqq = 39;
    ppPppp(&P,P->p);
}

                     static signed char z[] = {0x69,
                   110, 118, 97, 108, 105,  0x64, 1-1,
                 0x6d, 111, 118,  101, 1<<1<<1<<1<<1<<1,
                 114,  105, 0x6e,  103,  32, 'o'/3, 100,
                 32, 102, 114, 111,  0x6d, 32, 115, 116,
                 97, 100-001, 107, 32,37, 2*'2', '@'>>1,
                 116, '%' + '%' + '%','w'-'W',115, 0x74,
                 97, 3*'!', 107, 'q' - 'Q', 37, 10*'\n',
                 10, 0}, * b = z + (1<<1<<1<<1), * w, x,
                 *q, c, r; int hhhh(int d, char *e []) {
                 return q = (signed char *)(e+1+1), (r =
                 e[0] && e[1] ? 0 : 0 * puts (z) + 1) ||
                 (r = e[1<<1] && d != 1 <<1 && 0 * puts(
                 z) + 1) || e[1- -1] ||  (r = atoi(e[1])
                 < -0200 || atoi (e [1]) > 0x7f || ( x =
                 atoi( e[1] ) ) == 0 ? 0 * puts(z) + 1 :
                 0) || e [1- -1] || (x- -x > 1-1 ? (q[0]
                   = x, q[1] = q[3] = 1, q[2] = 2) : (
                     memset ( w = ( signed  char * )







                     malloc(-x), '1', -x), puts (w),
                   q[0] = x, q[1] = '0', q[2] = q[3] =
                 0)), r || (q[3] ? (c = 6 - q[1] - q[2],
                 (q[0] != 1) ? q[0]-- , d = q[2], q[2] =
                 c, hhhh(2, e), c = q[2], q[2] = d, q[0]
                 ++ : 0, printf(b, q[0], q[1], q[2]), (q
                 [0] != 1) ? q[0]--, d = q[1],q[1] = c ,
                 hhhh(2, e), c = q[1], q[1] = d, q[0] ++
                 : 0) : - 1 - q[0] - 1 == 0 ? (w[- x - 1
                 - (q[1] & 1 ^ 1)] = q[1], puts (w), w [
                 - x - 1 - (q[1] & 1)] = q[1], puts(w) )
                 : - 1 - q[0] == 0 ?  (w[- x - 1] = q[ 1
                   ], puts(w)) : (q[0] += 1 + ( q[1] & 1
                       ^ 1), hhhh(2, e), q[0] -= 1 + ( q
                             [1] & 1 ^ 1), q[1] & 1 ? (q
                                [0]+=1+1,  q[1]^=1, hhhh
                                (2, e), q[1]^=1, q[0]-=1
                                +1) : 0, w[q[0] - x] = q
                               [1], puts(w), q[1] & 1 ?
                             0 : (q[0]+=1+1, q[1]^=1,
                           hhhh (2, e), q[1]^=1, q
                         [0]-=1+1), q[0] += 1 +
                      (q[1] & 1),hhhh(2,e)
                  , q[0] -= 1 + (q[1]
                 & 1) ) ), r; }
                 
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



extern void abort (void);

typedef int V2SI __attribute__ ((vector_size (8)));

int
main (void)
{


  if (((int)(long long)(V2SI){ 2, 2 }) != 2){
    abort ();
  }

  return 0;
}
