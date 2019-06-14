__kernel void parsum (__global uint* x, __global uint* y, uint a)
{
	const int i = get_global_id (0);

	y [i] += a * x [i];
}