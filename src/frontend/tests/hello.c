/* struct MyStruct1 */
/* { */
/*   struct MyStruct2* x; */
/* } */
/*   ; */
/* struct MyStruct2  */
/* { */
/*   struct MyStruct1* x; */
/*   int y; */
  
/* } */
/*   ; */

typedef int a;

struct Mystruct3
{
  a* x;
  int y;
}
  ;

int foo(int x) 
{
  return x;
}


int main() {
  a* z;
  
  /* struct MyStruct2 s2; */
  /* s2.y = 2; */
  /* struct MyStruct1 s1; */
  /* s1.x = &s2; */
  /* s2.x = &s1; */
  
//return 0;
}
