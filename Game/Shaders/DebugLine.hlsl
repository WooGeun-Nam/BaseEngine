cbuffer CameraBuffer : register(b0)
{
    float4x4 ViewProj;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(float4(input.position, 1.0f), ViewProj);
    output.color = input.color;
    return output;
}

float4 PSMain(PS_INPUT input) : SV_Target
{
    return input.color;
}
