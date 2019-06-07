#include <altivec.h>
#include <stdio.h>

vector signed int calc_abs(vector signed int a)
{
    vector signed int vzero = {0, 0, 0, 0};
    vector signed int neg_a = vec_sub(vzero, a);
    vector bool int vpat = vec_cmpgt(vzero, a);

    return vec_sel(a, neg_a, vpat);
}

int main(int argc, char *argv[])
{
	vector signed int a = {-1, -2, 3, -4};
	vector signed int abs = calc_abs(a);


	printf("         a  = {%d, %d, %d, %d}\n", a[0], a[1], a[2], a[3]);
	printf("calc_abs(a) = {%d, %d, %d, %d}\n", abs[0], abs[1], abs[2], abs[3]);
	return 0;
}
