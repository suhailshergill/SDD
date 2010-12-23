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
