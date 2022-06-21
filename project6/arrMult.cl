kernel
void
ArrayMult( global const float *dA, global const float *dB, local float *prods, global float *dC)
{
	int gid = get_global_id( 0 );
	int numItems = get_local_size(0); 	//# of work items per group
	int tnum = get_local_id(0); 		//thread number in this group

	int wgNum = get_group_id(0); 		//work group number

	prods[tnum] = dA[gid] * dB[gid];

	//all threads execute this simultaneously
	int offset;
	for(offset = 1; offset < numItems; offset *= 2){
		int mask = 2*offset - 1;
		barrier(CLK_LOCAL_MEM_FENCE);	//wait for all threads
		if( (tnum & mask) == 0){
			prods[tnum] += prods[tnum + offset];
		}
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	if(tnum == 0){
		dC[wgNum] = prods[0];
	}
}
