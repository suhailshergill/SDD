int N = 1;
void foo() {} /* Necessary to trigger the original ICE.  */
void bar (char a[2][N]) { a[1][0] = N; }
int
main (void)
{
}
