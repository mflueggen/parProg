__kernel void parsum (__global uint* starts, __global uint* ranges, __global uint* output, __local uint *tmp)
{
	unsigned int lid = get_local_id(0);
    unsigned int gid = get_global_id(0);

    uint privateSum[4] = {0,0,0,0}; //128 bit
    for(uint i = 0; i <= ranges[gid]; ++i)
    {
        uint word0 = starts[gid*2] + i;
        privateSum[0] += word0;

    }

	tmp[lid*4] = privateSum[0];
	tmp[lid*4+1] = privateSum[1];
	tmp[lid*4+2] = privateSum[2];
	tmp[lid*4+3] = privateSum[3];

	barrier(CLK_LOCAL_MEM_FENCE);

	int foldPoint = get_local_size(0);
	foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid*4] += tmp[(lid + foldPoint)*4];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid*4] += tmp[(lid + foldPoint)*4];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid*4] += tmp[(lid + foldPoint)*4];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid*4] += tmp[(lid + foldPoint)*4];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid*4] += tmp[(lid + foldPoint)*4];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid*4] += tmp[(lid + foldPoint)*4];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid*4] += tmp[(lid + foldPoint)*4];} barrier(CLK_LOCAL_MEM_FENCE);
    foldPoint >>= 1;
    if( lid < foldPoint) {tmp[lid*4] += tmp[(lid + foldPoint)*4];} barrier(CLK_LOCAL_MEM_FENCE);

    if (lid == 0) output[get_group_id(0)*4] = tmp[0];
}
