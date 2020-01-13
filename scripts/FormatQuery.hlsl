RWBuffer<uint> QueryBuffer : register(u0);

// Read QueryBuffer like an array of UINT64s
// returns 1 element
uint2 Read64(uint idx)
{
    uint2 result;

    result.x = QueryBuffer[idx * 2];
    result.y = QueryBuffer[(idx * 2) + 1];

    return result;
}

// Writes QueryBuffer like an array of UINT64s
void Write64(uint2 val, uint idx)
{
    QueryBuffer[idx * 2] = val.x;
    QueryBuffer[(idx * 2) + 1] = val.y;
}

uint2 Add64(uint2 lhs, uint2 rhs)
{
    return AddUint64(lhs, rhs);
}

// used for stream-overflow predicates
// Accumulates data for all streams
// And compares NumPrimitivesWritten to PrimitivesStorageNeeded
cbuffer Constants : register(b0)
{
    uint NumStreams;
    uint Unused;   
};

[numthreads(1, 1, 1)]
void FormatQueryCS()
{
    uint Result = 0;

    for(uint Stream = 0; Stream < NumStreams; Stream++)
    {
        uint2 NumPrimitivesWritten = Read64(Stream * 2);
        uint2 PrimitivesStorageNeeded = Read64((Stream * 2) + 1);

        if((NumPrimitivesWritten.x != PrimitivesStorageNeeded.x) ||
           (NumPrimitivesWritten.y != PrimitivesStorageNeeded.y))
        {
            // If any 1 stream had overflow, then the result is set to 1
            Result = 1;
        }
    }   

    QueryBuffer[0] = Result;
    QueryBuffer[1] = 0;
}


// this shader is used to combine to arrays of 64-bit counters
// the query buffer holds a set of instances.  Each instance contains an array of 64-bit values.
// After running this shader, Instance0 holds the sum of all instances
cbuffer Constants : register(b0)
{
    uint NumInstances;         // the total number of instances
    uint NumValuesPerInstance; // the number of 64-bit values per instance
};

[numthreads(1, 1, 1)]
void AccumulateQueryCS()
{
    for(uint Instance = 1; Instance < NumInstances; Instance++)
    {
        for(uint Counter = 0; Counter < NumValuesPerInstance; Counter++)
        {
            // Read 64-bits from Instance0
            uint2 Instance0Value = Read64(Counter);

            // Read 64-bits from the current instance
            uint2 InstanceIValue = Read64(Instance * NumValuesPerInstance + Counter);

            // Combine
            uint2 Result = Add64(Instance0Value, InstanceIValue);

            // Write the result to Instance0
            Write64(Result, Counter);
        }
    }
}