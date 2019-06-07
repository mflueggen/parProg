/* Autovectorization example 4 from 
     https://www.gnu.org/software/gcc/projects/tree-ssa/vectorization.html
   Comment out lines 14-16, build again and see more vectorized (GCC 9.1) :) 
 */
#define MAX 255

typedef int aint __attribute__ ((__aligned__(16)));
int a[256], b[256], c[256];
void foo (int n, aint * __restrict__ p, aint * __restrict__ q) {
   int i, j;

   /* feature: support for (aligned) pointer accesses  */
   /* feature: support for constants  */
   while (n--){
      *p++ = *q++ + 5;
   }

   /* feature: support for read accesses with a compile time known misalignment  */
   for (i=0; i<n; i++){
      a[i] = b[i+1] + c[i+3];
   }

   /* feature: support for if-conversion  */
   for (i=0; i<n; i++){
      j = a[i];
      b[i] = (j > MAX ? MAX : 0);
   }
}

