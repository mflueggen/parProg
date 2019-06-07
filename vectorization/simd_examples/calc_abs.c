#include <stdio.h>

#if __ALTIVEC__
#include <altivec.h>
    vector signed int calc_abs(vector signed int a)
    {
        vector signed int vzero = {0, 0, 0, 0};
        vector signed int neg_a = vec_sub(vzero, a);
        vector bool int vpat = vec_cmpgt(vzero, a);

        return vec_sel(a, neg_a, vpat);
    }
#elif __SSE2__
#include <x86intrin.h>

__mm128i calc_abs(__mm128i a)
{
    __mm128i vzero = {0, 0, 0, 0};
    __mm128i neg_a = _mm_sub_epi32 (vzero, a);
    __mm128i pattern = _mm_cmpgt_epi32(vzero, a);
    //in this point do some shifting and bitwise operations. No direkt translation fpr vec_sel
}
#endif

int main(int argc, char *argv[])
{
#if __ALTIVEC__
    __mm128i a = {-1, -2, 3, -4};
	__mm128i abs = calc_abs(a);
#elif __SSE2__
    vector signed int a = {-1, -2, 3, -4};
	vector signed int abs = calc_abs(a);
#endif



	printf("         a  = {%d, %d, %d, %d}\n", a[0], a[1], a[2], a[3]);
	printf("calc_abs(a) = {%d, %d, %d, %d}\n", abs[0], abs[1], abs[2], abs[3]);
	return 0;
}
