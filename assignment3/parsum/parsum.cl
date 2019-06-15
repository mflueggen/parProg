__kernel void parsum (__global uint* starts, __global uint* ranges, __global uint* output, __local uint *tmp)
{ //TODO remove redundant variables
	unsigned int lid = get_local_id(0);
    unsigned int gid = get_global_id(0);

    uint privateSum[4] = {0,0,0,0}; //128 bit
    uint nextVal[2] = {0,0};
    for(uint i = 0; i <= ranges[gid]; ++i)
    {
        nextVal[0] = starts[gid*2] + i;
        uint carry = nextVal[0] < starts[gid*2];
        nextVal[1] = starts[gid*2+1] + carry; //No overflow since input is max 64 bit

        uint dword0 = privateSum[0] + nextVal[0];
        carry = dword0 < privateSum[0];
        uint dword1 = privateSum[1] + carry + nextVal[1];
        carry = dword1 < privateSum[1];
        uint dword2 = privateSum[2] + carry;
        carry = dword2 < privateSum[2];
        uint dword3 = privateSum[3] + carry; //no overflow possible since max 128 bit

        privateSum[0] = dword0;
        privateSum[1] = dword1;
        privateSum[2] = dword2;
        privateSum[3] = dword3;
    }

	tmp[lid*4] = privateSum[0];
	tmp[lid*4+1] = privateSum[1];
	tmp[lid*4+2] = privateSum[2];
	tmp[lid*4+3] = privateSum[3];

	barrier(CLK_LOCAL_MEM_FENCE);

	int foldPoint = get_local_size(0);

	foldPoint >>= 1;
    if( lid < foldPoint) {
        uint red0 = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        uint carry = red0 < tmp[lid*4];
        uint red1 = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = red1 < tmp[lid*4 +1];
        uint red2 = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = red2 < tmp[lid*4 +2];
        uint red3 = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = red0;
        tmp[lid*4 +1] = red1;
        tmp[lid*4 +2] = red2;
        tmp[lid*4 +3] = red3;
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        uint red0 = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        uint carry = red0 < tmp[lid*4];
        uint red1 = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = red1 < tmp[lid*4 +1];
        uint red2 = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = red2 < tmp[lid*4 +2];
        uint red3 = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = red0;
        tmp[lid*4 +1] = red1;
        tmp[lid*4 +2] = red2;
        tmp[lid*4 +3] = red3;
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        uint red0 = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        uint carry = red0 < tmp[lid*4];
        uint red1 = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = red1 < tmp[lid*4 +1];
        uint red2 = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = red2 < tmp[lid*4 +2];
        uint red3 = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = red0;
        tmp[lid*4 +1] = red1;
        tmp[lid*4 +2] = red2;
        tmp[lid*4 +3] = red3;
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        uint red0 = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        uint carry = red0 < tmp[lid*4];
        uint red1 = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = red1 < tmp[lid*4 +1];
        uint red2 = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = red2 < tmp[lid*4 +2];
        uint red3 = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = red0;
        tmp[lid*4 +1] = red1;
        tmp[lid*4 +2] = red2;
        tmp[lid*4 +3] = red3;
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        uint red0 = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        uint carry = red0 < tmp[lid*4];
        uint red1 = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = red1 < tmp[lid*4 +1];
        uint red2 = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = red2 < tmp[lid*4 +2];
        uint red3 = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = red0;
        tmp[lid*4 +1] = red1;
        tmp[lid*4 +2] = red2;
        tmp[lid*4 +3] = red3;
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        uint red0 = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        uint carry = red0 < tmp[lid*4];
        uint red1 = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = red1 < tmp[lid*4 +1];
        uint red2 = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = red2 < tmp[lid*4 +2];
        uint red3 = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = red0;
        tmp[lid*4 +1] = red1;
        tmp[lid*4 +2] = red2;
        tmp[lid*4 +3] = red3;
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        uint red0 = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        uint carry = red0 < tmp[lid*4];
        uint red1 = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = red1 < tmp[lid*4 +1];
        uint red2 = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = red2 < tmp[lid*4 +2];
        uint red3 = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = red0;
        tmp[lid*4 +1] = red1;
        tmp[lid*4 +2] = red2;
        tmp[lid*4 +3] = red3;
    } barrier(CLK_LOCAL_MEM_FENCE);

    foldPoint >>= 1;
    if( lid < foldPoint) {
        uint red0 = tmp[lid*4] + tmp[(lid + foldPoint)*4];
        uint carry = red0 < tmp[lid*4];
        uint red1 = tmp[lid*4 +1] + carry + tmp[(lid + foldPoint)*4 +1];
        carry = red1 < tmp[lid*4 +1];
        uint red2 = tmp[lid*4 +2] + carry + tmp[(lid + foldPoint)*4 +2];
        carry = red2 < tmp[lid*4 +2];
        uint red3 = tmp[lid*4 +3] + carry + tmp[(lid + foldPoint)*4 +3];

        tmp[lid*4] = red0;
        tmp[lid*4 +1] = red1;
        tmp[lid*4 +2] = red2;
        tmp[lid*4 +3] = red3;
    } barrier(CLK_LOCAL_MEM_FENCE);


    if (lid == 0) {
        output[get_group_id(0)*4] = tmp[0];
        output[get_group_id(0)*4 + 1] = tmp[1];
        output[get_group_id(0)*4 + 2] = tmp[2];
        output[get_group_id(0)*4 + 3] = tmp[3];
    }
}
