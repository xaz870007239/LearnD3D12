Texture2D T_1:register(t0);
Texture2D T_2:register(t1);

RWTexture2D<float4> T_Out:register(u0,space1);

[numthreads(1,1,1)]
void MainCS(int3 inDispatchThreadID:SV_DispatchThreadID){
    T_Out[inDispatchThreadID.xy]=T_1[inDispatchThreadID.xy]+T_2[inDispatchThreadID.xy];
}