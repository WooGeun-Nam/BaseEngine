cbuffer CameraBuffer : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 worldPosition = mul(float4(input.position, 1.0f), world);
    float4 viewPosition = mul(worldPosition, view);
    output.position = mul(viewPosition, projection);

    output.texcoord = input.texcoord;
    return output;
}

Texture2D spriteTexture : register(t0);
SamplerState spriteSampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    return spriteTexture.Sample(spriteSampler, input.texcoord);
}
