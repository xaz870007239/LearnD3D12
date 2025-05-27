struct VertexData{
    float4 position:POSITION;
    float4 texcoord:TEXCOORD0;
    float4 normal:NORMAL;
    float4 tangent:TANGENT;
};

struct VSOut{
    float4 position:SV_POSITION;
    float4 normal:NORMAL;
    float4 texcoord:TEXCOORD0;
};

static const float PI=3.141592;
cbuffer globalConstants:register(b0){
    float4x4 ProjectionMatrix;
    float4x4 ViewMatrix;
    float4 misc;
};

Texture2D T_DiffuseTexture:register(t0);
SamplerState samplerState:register(s0);

struct MaterialData{
    float r;
};
StructuredBuffer<MaterialData> materialData:register(t0,space1);
cbuffer DefaultVertexCB:register(b1){
    float4x4 ModelMatrix;
    float4x4 IT_ModelMatrix;
    float4x4 ReservedMemory[1020];
};

VSOut MainVS(VertexData inVertexData){
    VSOut vo;
    vo.normal=mul(IT_ModelMatrix,inVertexData.normal);
    vo.position=mul(ModelMatrix,inVertexData.position);
    vo.texcoord=inVertexData.texcoord;
    return vo;
}

[maxvertexcount(4)]
void MainGS(triangle VSOut inPoint[3],uint inPrimitiveID:SV_PrimitiveID,
    inout TriangleStream<VSOut> outTriangleStream){
    float3 N=normalize(inPoint[0].normal.xyz+inPoint[1].normal.xyz+inPoint[2].normal.xyz);
    float scale=materialData[inPrimitiveID].r;
    float3 offset=N*abs(sin(misc.x*4.0f))*0.2f;
    VSOut vo;
    float3 positionWS=inPoint[0].position.xyz+offset;
    float4 positionVS=mul(ViewMatrix,float4(positionWS,1.0f));
    vo.position=mul(ProjectionMatrix,positionVS);
    vo.normal=inPoint[0].normal;
    vo.texcoord=inPoint[0].texcoord;
    outTriangleStream.Append(vo);

    positionWS=inPoint[1].position.xyz+offset;
    positionVS=mul(ViewMatrix,float4(positionWS,1.0f));
    vo.position=mul(ProjectionMatrix,positionVS);
    vo.normal=inPoint[1].normal;
    vo.texcoord=inPoint[1].texcoord;
    outTriangleStream.Append(vo);

    positionWS=inPoint[2].position.xyz+offset;
    positionVS=mul(ViewMatrix,float4(positionWS,1.0f));
    vo.position=mul(ProjectionMatrix,positionVS);
    vo.normal=inPoint[2].normal;
    vo.texcoord=inPoint[2].texcoord;
    outTriangleStream.Append(vo);
}

float4 MainPS(VSOut inPSInput):SV_TARGET{
    float3 N=normalize(inPSInput.normal.xyz);
    float3 bottomColor=float3(0.1f,0.4f,0.6f);
    float3 topColor=float3(0.7f,0.7f,0.7f);
    float theta=asin(N.y);//-PI/2 ~ PI/2
    theta/=PI;//-0.5~0.5
    theta+=0.5f;//0.0~1.0
    float ambientColorIntensity=1.0;
    float3 ambientColor=lerp(bottomColor,topColor,theta)*ambientColorIntensity;
    float4 diffuseColor=T_DiffuseTexture.Sample(samplerState,inPSInput.texcoord.xy);
    float3 surfaceColor=diffuseColor.rgb;
    return float4(surfaceColor,1.0f);
}