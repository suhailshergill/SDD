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
