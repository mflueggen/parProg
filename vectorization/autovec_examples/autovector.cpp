/* Example of not vectorizable loop  as of g++-9 (Homebrew GCC 9.1.0), x86_64 */

#include <cstdio>

int main()
{
	const int N = 200000;
	const int M = 10000;
	float a[N];
	float b[N];
	float c[N];
	float result[N];
    
	/* works well */
	for (int i = 0; i < N; i++) {
		a[i] =       i + 0.0124;
		b[i] = 0.5 * i + 0.4567; 
		c[i] = 1.5 * i + 0.7890; 
    }    

	/* will fail */
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; ++j)
			result[j] = a[j] + b[j] - c[j] + i;
	}

	printf("result: {%f .. %f}\n;", result[0], result[N - 1]);
    return 0;
}

