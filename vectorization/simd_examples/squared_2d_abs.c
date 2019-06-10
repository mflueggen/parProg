#include <altivec.h>
#include <stdio.h>

struct point2d {
	float x, y;	
};

void squared_2d_abs(struct point2d *input, float *output, int num)
{
	int i;
	vector float *vinput  = (vector float *)input;
	vector float *voutput = (vector float *)output;
	vector unsigned char patx = {0x00, 0x01, 0x02, 0x03,
								 0x08, 0x09, 0x0a, 0x0b,
								 0x10, 0x11, 0x12, 0x13,
								 0x18, 0x19, 0x1a, 0x1b};

	vector unsigned char paty = {0x04, 0x05, 0x06, 0x07,
								 0x0c, 0x0d, 0x0e, 0x0f,
								 0x14, 0x15, 0x16, 0x17,
								 0x1c, 0x1d, 0x1e, 0x1f};


	for (i = 0; i < num / 4; i++) {
		vector float va = vinput[2 * i];
		vector float vb = vinput[2 * i + 1];

		vector float vx = vec_perm(va, vb, patx);
		vector float vy = vec_perm(va, vb, paty);
		voutput[i] = vec_add(vec_mul(vx, vx), vec_mul(vy, vy));
	}

	for (i = 4 * (num / 4); i < num; i++) {
		output[i] =   input[i].x * input[i].x
					+ input[i].y * input[i].y;
	}
}

int main(int argc, char *argv[])
{
	int i;
	struct point2d points[   ] __attribute__((aligned(16))) = {{0, 0}, {1, 1}, {3, 4}, {4, 3}, {23, 42}};
	const size_t num = sizeof(points) / sizeof(points[0]);
	float       distances[num] __attribute__((aligned(16)));

	squared_2d_abs(points, distances, num);
	for (i = 0; i < num; i++) {
		printf("squared_2d_abs({%f, %f}) = %f\n", points[i].x, points[i].y, distances[i]);
	}
	return 0;
}
