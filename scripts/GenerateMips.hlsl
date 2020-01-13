// Fullscreen quad
static const float4 positions[] = {
    float4(-1, 1, 1, 1),  // top left
    float4(1, 1, 1, 1),   // top right
    float4(-1, -1, 1, 1), // bottom left
    float4(1, -1, 1, 1),  // bottom right
};

static const float2 texcoords[] = {
    float2(0, 0), // top left
    float2(1, 0), // top right
    float2(0, 1), // bottom left
    float2(1, 1), // bottom right
};

cbuffer GenMipsConstants : register(b0)
{
    uint mipLevel;
    uint slice;
    // used for 3d only to avoid shader math
    float zCoord;
};

struct VSOutPSIn
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

// Should be called with no VB to draw 4 vertices for a fullscreen quad
VSOutPSIn VSMain(dword input : SV_VertexID)
{
    VSOutPSIn ret;
    ret.position = positions[input];
    ret.texcoord = texcoords[input];
    return ret;
}

sampler textureSampler : register(s0);

#if Tex1D
Texture1DArray inputTexture : register(t0);

float4 PSMain(VSOutPSIn input) : SV_Target
{
    float2 pos = float2(input.texcoord.x, slice);
    return inputTexture.SampleLevel(textureSampler, pos, mipLevel);
}
#endif

#if Tex2D
Texture2DArray inputTexture : register(t0);

float4 PSMain(VSOutPSIn input) : SV_Target
{
    float3 pos = float3(input.texcoord.xy, slice);
    return inputTexture.SampleLevel(textureSampler, pos, mipLevel);
}
#endif

#if Tex3D
Texture3D inputTexture : register(t0);

float4 PSMain(VSOutPSIn input) : SV_Target
{
    float3 pos = float3(input.texcoord.xy, zCoord);
    return inputTexture.SampleLevel(textureSampler, pos, mipLevel);
}
#endif