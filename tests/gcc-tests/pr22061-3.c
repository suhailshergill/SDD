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
void foo() {} /* Necessary to trigger the original ICE.  */
void bar (char a[2][N]) { a[1][0] = N; }

int
main (void)
{
  void *x;

  N = 4;
  x = alloca (2 * N);
  memset (x, 0, 2 * N);

  struct MyStruct3
  {
    a* x;
    a y;
  }
  ;


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
  

 typedef struct MyStruct3 MyStruct3_t;

 if (N[(char *) x] != N)
   {
     Person p1;
     p1.sex = male;
     p1.name = "Tom";
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
