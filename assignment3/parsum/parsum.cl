__kernel void parsum (__global uint* starts, __global uint* ranges, __global uint* output, __local uint *tmp)
{
	unsigned int lid = get_local_id(0);
    unsigned int gid = get_global_id(0);

    uint privateSum = 0;
    for(uint i = 0; i <= ranges[gid]; ++i)
    {
        privateSum += starts[gid] + i;
    }

	tmp[lid] = privateSum;

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
