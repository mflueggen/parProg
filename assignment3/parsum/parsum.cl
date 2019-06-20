__kernel void parsum (__global uint* starts, __global uint* ranges, __global uint* output, __local uint *tmp)
{
	private unsigned int lid = get_local_id(0);
    private unsigned int gid = get_global_id(0);

    private uint privateSum[4] = {0,0,0,0}; //128 bit
    private uint nextVal[2] = {0,0};
    private uint dword[4] = {0,0,0,0};
    private uint carry = 0;
    for(uint i = 0; i <= ranges[gid]; ++i)
    {
        nextVal[0] = starts[gid*2] + i;
        carry = nextVal[0] < starts[gid*2];
        nextVal[1] = starts[gid*2+1] + carry; //No overflow since input is max 64 bit

        dword[0] = privateSum[0] + nextVal[0];
        carry = dword[0] < privateSum[0];
        dword[1] = privateSum[1] + carry + nextVal[1];
        carry = dword[1] < privateSum[1];
        dword[2] = privateSum[2] + carry;
        carry = dword[2] < privateSum[2];
        dword[3] = privateSum[3] + carry; //no overflow possible since max 128 bit

        privateSum[0] = dword[0];
        privateSum[1] = dword[1];
        privateSum[2] = dword[2];
        privateSum[3] = dword[3];
    }

	tmp[lid*4] = privateSum[0];
	tmp[lid*4+1] = privateSum[1];
	tmp[lid*4+2] = privateSum[2];
	tmp[lid*4+3] = privateSum[3];

	barrier(CLK_LOCAL_MEM_FENCE);

	int foldPoint = get_local_size(0);

	foldPoint >>= 1;
    if( lid < foldPoint) {
        dword[0] = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        carry = dword[0] < tmp[lid*4];
        dword[1] = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = dword[1] < tmp[lid*4 +1];
        dword[2] = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = dword[2] < tmp[lid*4 +2];
        dword[3] = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = dword[0];
        tmp[lid*4 +1] = dword[1];
        tmp[lid*4 +2] = dword[2];
        tmp[lid*4 +3] = dword[3];
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        dword[0] = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        carry = dword[0] < tmp[lid*4];
        dword[1] = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = dword[1] < tmp[lid*4 +1];
        dword[2] = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = dword[2] < tmp[lid*4 +2];
        dword[3] = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = dword[0];
        tmp[lid*4 +1] = dword[1];
        tmp[lid*4 +2] = dword[2];
        tmp[lid*4 +3] = dword[3];
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        dword[0] = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        carry = dword[0] < tmp[lid*4];
        dword[1] = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = dword[1] < tmp[lid*4 +1];
        dword[2] = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = dword[2] < tmp[lid*4 +2];
        dword[3] = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = dword[0];
        tmp[lid*4 +1] = dword[1];
        tmp[lid*4 +2] = dword[2];
        tmp[lid*4 +3] = dword[3];
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        dword[0] = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        carry = dword[0] < tmp[lid*4];
        dword[1] = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = dword[1] < tmp[lid*4 +1];
        dword[2] = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = dword[2] < tmp[lid*4 +2];
        dword[3] = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = dword[0];
        tmp[lid*4 +1] = dword[1];
        tmp[lid*4 +2] = dword[2];
        tmp[lid*4 +3] = dword[3];
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        dword[0] = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        carry = dword[0] < tmp[lid*4];
        dword[1] = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = dword[1] < tmp[lid*4 +1];
        dword[2] = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = dword[2] < tmp[lid*4 +2];
        dword[3] = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = dword[0];
        tmp[lid*4 +1] = dword[1];
        tmp[lid*4 +2] = dword[2];
        tmp[lid*4 +3] = dword[3];
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        dword[0] = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        carry = dword[0] < tmp[lid*4];
        dword[1] = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = dword[1] < tmp[lid*4 +1];
        dword[2] = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = dword[2] < tmp[lid*4 +2];
        dword[3] = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = dword[0];
        tmp[lid*4 +1] = dword[1];
        tmp[lid*4 +2] = dword[2];
        tmp[lid*4 +3] = dword[3];
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        dword[0] = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        carry = dword[0] < tmp[lid*4];
        dword[1] = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = dword[1] < tmp[lid*4 +1];
        dword[2] = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = dword[2] < tmp[lid*4 +2];
        dword[3] = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = dword[0];
        tmp[lid*4 +1] = dword[1];
        tmp[lid*4 +2] = dword[2];
        tmp[lid*4 +3] = dword[3];
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        dword[0] = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        carry = dword[0] < tmp[lid*4];
        dword[1] = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = dword[1] < tmp[lid*4 +1];
        dword[2] = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = dword[2] < tmp[lid*4 +2];
        dword[3] = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = dword[0];
        tmp[lid*4 +1] = dword[1];
        tmp[lid*4 +2] = dword[2];
        tmp[lid*4 +3] = dword[3];
    } barrier(CLK_LOCAL_MEM_FENCE);


    if (lid == 0) {
        output[get_group_id(0)*4] = tmp[0];
        output[get_group_id(0)*4 + 1] = tmp[1];
        output[get_group_id(0)*4 + 2] = tmp[2];
        output[get_group_id(0)*4 + 3] = tmp[3];
    }
}
