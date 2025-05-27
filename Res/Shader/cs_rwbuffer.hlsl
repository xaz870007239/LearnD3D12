struct Data{
    float3 offset;
};
StructuredBuffer<Data> SB_A:register(t0,space1);//material data ,-0.5,0.0,0.0
StructuredBuffer<Data> SB_B:register(t1,space1);//light data,0.5,0.0,0.0

RWStructuredBuffer<Data> UAV_Out:register(u0,space2);

[numthreads(1,1,1)]
void MainCS(int3 inDispatchThreadID:SV_DispatchThreadID){
    UAV_Out[0].offset=SB_A[0].offset*2.0;
    UAV_Out[1].offset=SB_B[0].offset*2.0;
}