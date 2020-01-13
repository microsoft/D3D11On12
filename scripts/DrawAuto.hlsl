// used by DrawAuto to convert BufferFilledSize into VertexCount

RWBuffer<uint> SuffixBuffer : register(u0);

cbuffer Constants : register(b0)
{
    uint VBOffset;
    uint VBStride;   
};

[numthreads(1, 1, 1)]
void DrawAutoCS()
{
    uint BufferFilledSize = SuffixBuffer[0];

    uint VertexByteCount = 0;

    if(VBOffset < BufferFilledSize)
    {
        VertexByteCount = BufferFilledSize - VBOffset;
    }

    uint VertexCount = 0;

    if(VBStride > 0)
    {
        VertexCount = VertexByteCount / VBStride;
    }

    SuffixBuffer[1] = VertexCount;
    SuffixBuffer[2] = 1; // InstanceCount
    SuffixBuffer[3] = 0; // StartVertexLocation
    SuffixBuffer[4] = 0; // StartInstanceLocation
}