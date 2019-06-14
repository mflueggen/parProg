// input[0] == start value to count (e.g. 1)
// input[1] == end value to count (e.g. 100)
__kernel void parsum (__global uint* input, __global uint* output)
{
	int val1 = get_global_id (0) + input[0]; // This is ensured to be always less than the end value by the host
	int val2 = (val1 + get_global_size (0));
	val2 = val2 * (input[1] / val2); // ensure to add 0 if the second value is greater than the end value; val2<(end/2) due to global_size defined by Host
	val1 += val2;

	output [get_global_id (0)] = val1;

	output [get_global_id (0)] = get_num_groups(0);
}