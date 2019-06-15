// input[0] == start value to count (e.g. 1)
// input[1] == end value to count (e.g. 100)
__kernel void parsum (__global uint* input, __global uint* output, __local uint *tmp)
{
	unsigned int lid = get_local_id(0);
    unsigned int gid = get_global_id(0);

    uint val1 = gid + input[0]; // This is ensured to be always less than the end value by the host
	uint val2 = (val1 + get_global_size(0));
	val2 = val2 * (input[1] / val2); // ensure to add 0 if the second value is greater than the end value; val2<(end/2) due to global_size defined by Host
	tmp[lid] = val1 + val2;

	barrier(CLK_LOCAL_MEM_FENCE);
	int foldPoint = get_local_size(0);
	foldPoint >>= 1;


    if( lid < foldPoint) {tmp[lid] += tmp[lid + foldPoint];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid] += tmp[lid + foldPoint];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid] += tmp[lid + foldPoint];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid] += tmp[lid + foldPoint];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid] += tmp[lid + foldPoint];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid] += tmp[lid + foldPoint];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid] += tmp[lid + foldPoint];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid] += tmp[lid + foldPoint];} barrier(CLK_LOCAL_MEM_FENCE);

    if (lid == 0) output[get_group_id(0)] = tmp[0];
}
