typedef unsigned int size_t;


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;




__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;







__extension__ typedef long long int __quad_t;
__extension__ typedef unsigned long long int __u_quad_t;


__extension__ typedef __u_quad_t __dev_t;
__extension__ typedef unsigned int __uid_t;
__extension__ typedef unsigned int __gid_t;
__extension__ typedef unsigned long int __ino_t;
__extension__ typedef __u_quad_t __ino64_t;
__extension__ typedef unsigned int __mode_t;
__extension__ typedef unsigned int __nlink_t;
__extension__ typedef long int __off_t;
__extension__ typedef __quad_t __off64_t;
__extension__ typedef int __pid_t;
__extension__ typedef struct { int __val[2]; } __fsid_t;
__extension__ typedef long int __clock_t;
__extension__ typedef unsigned long int __rlim_t;
__extension__ typedef __u_quad_t __rlim64_t;
__extension__ typedef unsigned int __id_t;
__extension__ typedef long int __time_t;
__extension__ typedef unsigned int __useconds_t;
__extension__ typedef long int __suseconds_t;

__extension__ typedef int __daddr_t;
__extension__ typedef long int __swblk_t;
__extension__ typedef int __key_t;


__extension__ typedef int __clockid_t;


__extension__ typedef void * __timer_t;


__extension__ typedef long int __blksize_t;




__extension__ typedef long int __blkcnt_t;
__extension__ typedef __quad_t __blkcnt64_t;


__extension__ typedef unsigned long int __fsblkcnt_t;
__extension__ typedef __u_quad_t __fsblkcnt64_t;


__extension__ typedef unsigned long int __fsfilcnt_t;
__extension__ typedef __u_quad_t __fsfilcnt64_t;

__extension__ typedef int __ssize_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


__extension__ typedef int __intptr_t;


__extension__ typedef unsigned int __socklen_t;









typedef struct _IO_FILE FILE;






/* typedef struct _IO_FILE __FILE; */
/* typedef long int wchar_t; */

typedef unsigned int wint_t;
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;


typedef struct
{
  __off_t __pos;
  __mbstate_t __state;
} _G_fpos_t;
typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} _G_fpos64_t;





enum
{
  __GCONV_OK = 0,
  __GCONV_NOCONV,
  __GCONV_NODB,
  __GCONV_NOMEM,

  __GCONV_EMPTY_INPUT,
  __GCONV_FULL_OUTPUT,
  __GCONV_ILLEGAL_INPUT,
  __GCONV_INCOMPLETE_INPUT,

  __GCONV_ILLEGAL_DESCRIPTOR,
  __GCONV_INTERNAL_ERROR
};



enum
{
  __GCONV_IS_LAST = 0x0001,
  __GCONV_IGNORE_ERRORS = 0x0002
};



struct __gconv_step;
struct __gconv_step_data;
struct __gconv_loaded_object;
struct __gconv_trans_data;



typedef int (*__gconv_fct) (struct __gconv_step *, struct __gconv_step_data *,
       __const unsigned char **, __const unsigned char *,
       unsigned char **, size_t *, int, int);


typedef wint_t (*__gconv_btowc_fct) (struct __gconv_step *, unsigned char);


typedef int (*__gconv_init_fct) (struct __gconv_step *);
typedef void (*__gconv_end_fct) (struct __gconv_step *);



typedef int (*__gconv_trans_fct) (struct __gconv_step *,
      struct __gconv_step_data *, void *,
      __const unsigned char *,
      __const unsigned char **,
      __const unsigned char *, unsigned char **,
      size_t *);


typedef int (*__gconv_trans_context_fct) (void *, __const unsigned char *,
       __const unsigned char *,
       unsigned char *, unsigned char *);


typedef int (*__gconv_trans_query_fct) (__const char *, __const char ***,
     size_t *);


typedef int (*__gconv_trans_init_fct) (void **, const char *);
typedef void (*__gconv_trans_end_fct) (void *);

struct __gconv_trans_data
{

  __gconv_trans_fct __trans_fct;
  __gconv_trans_context_fct __trans_context_fct;
  __gconv_trans_end_fct __trans_end_fct;
  void *__data;
  struct __gconv_trans_data *__next;
};



struct __gconv_step
{
  struct __gconv_loaded_object *__shlib_handle;
  __const char *__modname;

  int __counter;

  char *__from_name;
  char *__to_name;

  __gconv_fct __fct;
  __gconv_btowc_fct __btowc_fct;
  __gconv_init_fct __init_fct;
  __gconv_end_fct __end_fct;



  int __min_needed_from;
  int __max_needed_from;
  int __min_needed_to;
  int __max_needed_to;


  int __stateful;

  void *__data;
};



struct __gconv_step_data
{
  unsigned char *__outbuf;
  unsigned char *__outbufend;



  int __flags;



  int __invocation_counter;



  int __internal_use;

  __mbstate_t *__statep;
  __mbstate_t __state;



  struct __gconv_trans_data *__trans;
};



typedef struct __gconv_info
{
  size_t __nsteps;
  struct __gconv_step *__steps;
  __extension__ struct __gconv_step_data __data [];
} *__gconv_t;

typedef union
{
  struct __gconv_info __cd;
  struct
  {
    struct __gconv_info __cd;
    struct __gconv_step_data __data;
  } __combined;
} _G_iconv_t;

typedef int _G_int16_t __attribute__ ((__mode__ (__HI__)));
typedef int _G_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int _G_uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int _G_uint32_t __attribute__ ((__mode__ (__SI__)));
typedef __builtin_va_list __gnuc_va_list;
struct _IO_jump_t; struct _IO_FILE;

typedef void _IO_lock_t;





struct _IO_marker {
  struct _IO_marker *_next;
  struct _IO_FILE *_sbuf;



  int _pos;

};


enum __codecvt_result
{
  __codecvt_ok,
  __codecvt_partial,
  __codecvt_error,
  __codecvt_noconv
};

struct _IO_FILE {
  int _flags;




  char* _IO_read_ptr;
  char* _IO_read_end;
  char* _IO_read_base;
  char* _IO_write_base;
  char* _IO_write_ptr;
  char* _IO_write_end;
  char* _IO_buf_base;
  char* _IO_buf_end;

  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;



  int _flags2;

  __off_t _old_offset;



  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];



  _IO_lock_t *_lock;

  __off64_t _offset;

  void *__pad1;
  void *__pad2;
  void *__pad3;
  void *__pad4;
  size_t __pad5;

  int _mode;

  char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];

};


typedef struct _IO_FILE _IO_FILE;


struct _IO_FILE_plus;

extern struct _IO_FILE_plus _IO_2_1_stdin_;
extern struct _IO_FILE_plus _IO_2_1_stdout_;
extern struct _IO_FILE_plus _IO_2_1_stderr_;

typedef __ssize_t __io_read_fn (void *__cookie, char *__buf, size_t __nbytes);







typedef __ssize_t __io_write_fn (void *__cookie, __const char *__buf,
     size_t __n);







typedef int __io_seek_fn (void *__cookie, __off64_t *__pos, int __w);


typedef int __io_close_fn (void *__cookie);

extern int __underflow (_IO_FILE *);
extern int __uflow (_IO_FILE *);
extern int __overflow (_IO_FILE *, int);
extern wint_t __wunderflow (_IO_FILE *);
extern wint_t __wuflow (_IO_FILE *);
extern wint_t __woverflow (_IO_FILE *, wint_t);

extern int _IO_getc (_IO_FILE *__fp);
extern int _IO_putc (int __c, _IO_FILE *__fp);
extern int _IO_feof (_IO_FILE *__fp) __attribute__ ((__nothrow__));
extern int _IO_ferror (_IO_FILE *__fp) __attribute__ ((__nothrow__));

extern int _IO_peekc_locked (_IO_FILE *__fp);





extern void _IO_flockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern void _IO_funlockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern int _IO_ftrylockfile (_IO_FILE *) __attribute__ ((__nothrow__));

extern int _IO_vfscanf (_IO_FILE * __restrict, const char * __restrict,
   __gnuc_va_list, int *__restrict);
extern int _IO_vfprintf (_IO_FILE *__restrict, const char *__restrict,
    __gnuc_va_list);
extern __ssize_t _IO_padn (_IO_FILE *, int, __ssize_t);
extern size_t _IO_sgetn (_IO_FILE *, void *, size_t);

extern __off64_t _IO_seekoff (_IO_FILE *, __off64_t, int, int);
extern __off64_t _IO_seekpos (_IO_FILE *, __off64_t, int);

extern void _IO_free_backup_area (_IO_FILE *) __attribute__ ((__nothrow__));


typedef _G_fpos_t fpos_t;







extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;









extern int remove (__const char *__filename) __attribute__ ((__nothrow__));

extern int rename (__const char *__old, __const char *__new) __attribute__ ((__nothrow__));














extern FILE *tmpfile (void);

extern char *tmpnam (char *__s) __attribute__ ((__nothrow__));





extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__));

extern char *tempnam (__const char *__dir, __const char *__pfx)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));








extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);


extern int fflush_unlocked (FILE *__stream);







extern FILE *fopen (__const char *__restrict __filename,
      __const char *__restrict __modes);




extern FILE *freopen (__const char *__restrict __filename,
        __const char *__restrict __modes,
        FILE *__restrict __stream);



extern FILE *fdopen (int __fd, __const char *__modes) __attribute__ ((__nothrow__));




extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__));





extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__));








extern int fprintf (FILE *__restrict __stream,
      __const char *__restrict __format, ...);




extern int printf (__const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      __const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg);




extern int vprintf (__const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));





extern int snprintf (char *__restrict __s, size_t __maxlen,
       __const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        __const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));



extern int fscanf (FILE *__restrict __stream,
     __const char *__restrict __format, ...) ;




extern int scanf (__const char *__restrict __format, ...) ;

extern int sscanf (__const char *__restrict __s,
     __const char *__restrict __format, ...) __attribute__ ((__nothrow__));




extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);


extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);

extern int fgetc_unlocked (FILE *__stream);











extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);


extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);








extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     ;






extern char *gets (char *__s) ;





extern int fputs (__const char *__restrict __s, FILE *__restrict __stream);





extern int puts (__const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) ;




extern size_t fwrite (__const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) ;


extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (__const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream) ;








extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream);


extern int fseeko (FILE *__stream, __off_t __off, int __whence);




extern __off_t ftello (FILE *__stream) ;







extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, __const fpos_t *__pos);


extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__)) ;

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__)) ;




extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;








extern void perror (__const char *__s);






extern int sys_nerr;
extern __const char *__const sys_errlist[];





extern int fileno (FILE *__stream) __attribute__ ((__nothrow__)) ;




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;

extern FILE *popen (__const char *__command, __const char *__modes) ;





extern int pclose (FILE *__stream);





extern char *ctermid (char *__s) __attribute__ ((__nothrow__));

extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__)) ;


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__));
typedef char lOL;

lOL*QI[] = {"Use:\012\011dump file\012","Unable to open file '\x25s'\012",
 "\012","   ",""};

binDispl(I,Il)
lOL*Il[];
{ FILE *L;
 unsigned lO;
 int Q,OL[' '^'0'],llO = (-1),

 O=1,l=0,lll=O+O+O+l,OQ=056;
 lOL*llL="%2x ";


 lO = I-(O<<l<<O);
 while (L-l,1)
 { for(Q = 0L;((Q &~(0x10-O))== l);
   OL[Q++] = fgetc(L));
  if (OL[0]==llO) break;
  printf("\0454x: ",lO);
  if (I == (1<<1))
  { for(Q=strlen(QI[O<<O<<1]);Q<strlen(QI[0]);
   Q++)printf((OL[Q]!=llO)?llL:QI[lll],OL[Q]);

   printf(QI[lll]);{}
  }
  for (Q=0L;Q<1<<1<<1<<1<<1;Q+=Q<0100)
  { (OL[Q]!=llO)?
   ((D(OL[Q])==0&&(*(OL+abs(Q-l))=OQ)),
   putchar(OL[Q])):
   putchar(1<<(1<<1<<1)<<1);
  }
  printf(QI[01^10^9]);
  lO+=Q+0+l;}
 }
 D(l) {
}

typedef struct F G;
G aa();G b();G c();G d();G e();G f();G g();G h();G i();G j();G k();G l();G m();G n();G o();G p();G q();G r();G s();G t();G u();G v();G w();G x();G y();G z();
G S();G NN();
void Q();

struct F{
G (*aa)();G (*b)();G (*c)();G (*d)();G (*e)();G (*f)();G (*g)();G (*h)();G (*i)();G (*j)();G (*k)();G (*l)();G (*m)();G (*n)();G (*o)();G (*p)();G (*q)();G (*r)();G (*s)();G (*t)();G (*u)();G (*v)();G (*w)();G (*x)();G (*y)();G (*z)();
G (*S)();G (*NN)();
void(*Q)();
}

X={aa,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,S,NN,Q};
G aa() { printf("z"); return X; }G b() { printf("y"); return X; }G c() { printf("x"); return X; }G d() { printf("w"); return X; }G e() { printf("v"); return X; }G f() { printf("u"); return X; }G g() { printf("t"); return X; }G h() { printf("s"); return X; }G i() { printf("r"); return X; }G j() { printf("q"); return X; }G k() { printf("p"); return X; }G l() { printf("o"); return X; }G m() { printf("n"); return X; }G n() { printf("m"); return X; }G o() { printf("l"); return X; }G p() { printf("k"); return X; }G q() { printf("j"); return X; }G r() { printf("i"); return X; }G s() { printf("h"); return X; }G t() { printf("g"); return X; }G u() { printf("f"); return X; }G v() { printf("e"); return X; }G w() { printf("d"); return X; }G x() { printf("c"); return X; }G y() { printf("b"); return X; }G z() { printf("aa"); return X; }G S() { printf(" "); return X; }G NN() { printf("\n"); return X; }
void Q(){}

typing(){X=g().s().v().S().j().f().r().x().p().S().y().i().l().d().m().S().u().l().c().S().q().f().n().k().v().w().S().l().e().v().i().S().g().s().v().S().o().z().aa().b().S().w().l().t().NN();}





twelvedays(t,_,a)
char
*
a;
{
 return!

0<t?
t<3?

twelvedays(-79,-13,a+
twelvedays(-87,1-_,
twelvedays(-86, 0, a+1 )


+a)):

1,
t<_?
twelvedays(t+1, _, a )
:3,

twelvedays ( -94, -27+t, a )
&&t == 2 ?_
<13 ?

twelvedays ( 2, _+1, "%s %d %d\n" )

:9:16:
t<0?
t<-72?
twelvedays( _, t,
"k,#n'+,#'/*{}w+/w#cdnr/+,{}r/*de}+,/*{*+,/w{%+,/w#q#n+,/#{l,+,/n{n+,/+#n+,/#;#q#n+,/+k#;*+,/'-el' ))# }#r]'K:'K n l#}'w {r'+d'K#!/+#;;'+,#K'{+w' '*# +e}#]!/w :'{+w'nd+'we))d}+#r]!/c, nl#'+,#'rdceK#n+ +{dn]!/-; K#'{+'dn'+,#', }rk }#]!/*{nr' 'k :' }denr'{+]!/w :'+,#:'n##r' n'e)l} r#]!/}#{nw+ ;;'+,#'wd*+k }#]!/ w['*d}' 'reK)]!/ew#' 'r#-ell#}]!/+}:'+d'}#)}drec#'{+]!/ w['+,#K',dk'+,#:'r{r'{+]!/w##'{*'{+', ))#nw' l {n(!!/")

:
t<-50?
_==*a ?
putchar(31[a]):

twelvedays(-65,_,a+1)
:
twelvedays((*a == '/') + t, _, a + 1 )
:

0<t?

twelvedays ( 2, 2 , "%s")
:*a=='/'||

twelvedays(0,

twelvedays(-61,*a,
"!ek;dc i@bK'(q)-[w]*%n+r3#l,{}:\nuwloca-O;m .vpbks,fxntdCeghiry")



,a+1);}

    
   
  
  
 
 





 piglatin(){ char
 oink[9],*igpa,
 atinla=getchar(),iocccwa
 ,apca='A',owla='a',umna=26
 ; for(; (atinla+1)&&(!(((
 atinla-apca)*(apca+umna-atinla)
 >=0)+((atinla-owla)*(owla+umna-
 atinla)>=0))); putchar(atinla),
 atinla=getchar()); for(; atinla+1;
  ){ for( igpa=oink ,iocccwa=(
  (atinla- apca)*( apca+umna-
   atinla)>=0) ; ((((
  atinla-apca )*(apca+
  umna-atinla )>=0) +((atinla-
 owla)*(owla+ umna- atinla)>=0))
 &&"-Pig-" "Lat-in" "COb-fus"
 "ca-tion!!"[ (((atinla- apca)*(apca+
 umna-atinla) >=0)?atinla- apca+owla:
 atinla)-owla ]-'-')||((igpa== oink)&&!(*(
 igpa++)='w') )||! (*( igpa ++)=owla); *
 (igpa++)=(( ( atinla-apca
 )*(apca+ umna - atinla)>=0)
 ?atinla- apca + owla :atinla),
 atinla= getchar())
 ; for( atinla=iocccwa?(( (atinla-
 owla)*(owla+ umna-atinla)>=0 )?atinla-
 owla+apca: atinla): atinla; (((
  atinla-apca)* (apca+umna- atinla)>=0)+(
  (atinla-owla)* (owla+ umna-atinla)>=
   0)); putchar( atinla),atinla
   =getchar()); for(*igpa=0,
    igpa=oink; * igpa; putchar(
     *(igpa++))); for(; (atinla+1)&&(!(((
      atinla-apca )*(apca+
       umna- atinla)>=0
        )+(( atinla-
         owla)*( owla+umna-
           atinla)>=0))); putchar
             (atinla),atinla=
               getchar()); }
                 }
                    

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


a N = 1;
void foo() {}

int foobar(a x, a y)
{
  int z = x + y;
  x = x+3;
  y = y + 123;
  x += x*x;
  y += x;
  return x * y;
}


void bar (char a[2][N]) { a[1][0] = N; }

int
main (void)
{
  void *x;
     typing();

  N = 4;
  x = alloca (2 * N);
  piglatin();
  memset (x, 0, 2 * N);

  typedef struct
  {
    a* x;
    a y;
  }
  MyStruct3_t;


  bar (x);

  typedef struct
  {

    int age;

    char *name;

    enum
      {
 male, female
      }
      sex;

  }
  Person;


 if (N[(char *) x] != N)
   {
     Person p1;
     p1.sex = male;
     p1.name = "Tom";
     typing();
     p1.age = 43;
     Person p2;
     p2.sex = male;
     p2.name = "Jerry";
     p2.age = 32;
     abort ();
   }


  MyStruct3_t s2;
  s2.y = 3;
  s2.x = &s2.y;

  exit (0);
}
