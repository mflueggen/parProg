#include <stdio.h>
#include <stdlib.h>

#if __AVX__
    size_t alignment = 32;
#else
    size_t alignment = 16;
#endif

void scale(float *, int, float);

size_t readdata(float **data, int dims[3])
{
	fread(dims, sizeof(int), 3, stdin);
	size_t size = dims[0] * dims[1] * dims[2];
	if (!*data) {
		posix_memalign((void **)data, alignment, size * sizeof(float));
	}
	fread(*data, sizeof(float), size, stdin);
	return size;
}

void writedata(float *data, int dims[3])
{
	size_t size = dims[0] * dims[1] * dims[2];
	fwrite(dims, sizeof(int), 3, stdout);
	fwrite(data, sizeof(float), size, stdout);
}

int main()
{
	int dims[3] = {0, 0, 0};
	float *data = NULL;

#if __ALTIVEC__
	fprintf(stderr, "Using AltiVec\n");
#elif __AVX__
    fprintf(stderr, "Using AVX\n");
#elif __SSE2__
    fprintf(stderr, "Using SSE2\n");
#else
    fprintf(stderr, "Using scalar execution\n");
#endif

    while (1) {
		size_t num = readdata(&data, dims);
		/* Call your filters here */
		scale(data, num, 4.0);

		writedata(data, dims);
	}
	free(data);
	return 0;
}

//---------------------------------------------------------
#if __ALTIVEC__

#include <altivec.h>

void scale(float *input, int num, float scale)
{
    int i;
	vector float vscale = {scale, scale, scale, scale};
    for (i = 0; i < num - 4; i += 4) {
		vector float *current = (vector float *)(&input[i]);
		*current = vec_mul(*current, vscale);
    }
	for (; i < num; i++) {
		input[i] *= scale;
	}
}

#elif __AVX__
#include <x86intrin.h>

void scale(float *input, int num, float scale)
{
    int i;
    __m256 vscale = _mm256_set1_ps(scale);
    for (i = 0; i < num - 8; i += 8) {
        __m256 current = _mm256_load_ps(&input[i]);
        current = _mm256_mul_ps(current, vscale);
        _mm256_store_ps(&input[i], current);
    }
    for (; i < num; i++) {
        input[i] *= scale;
    }
}

#elif __SSE2__

#include <x86intrin.h>

void scale(float *input, int num, float scale)
{
    int i;
	__m128 vscale = _mm_set1_ps(scale);
    for (i = 0; i < num - 4; i += 4) {
		__m128 current = _mm_load_ps(&input[i]);
		current = _mm_mul_ps(current, vscale);
		_mm_store_ps(&input[i], current);
    }
	for (; i < num; i++) {
		input[i] *= scale;
	}
}

#else

void scale(float *input, int num, float scale)
{
	int i;
	for (i = 0; i < num; i++) {
		input[i] *= scale;
	}
}

#endif
