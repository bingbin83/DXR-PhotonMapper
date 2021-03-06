#ifndef COMPUTE_PASS_0
#define COMPUTE_PASS_0

#define HLSL
#include "RaytracingHlslCompat.h"

#define blocksize 128

// Render Target for visualizing the photons - can be removed later on
RWTexture2D<float4> RenderTarget : register(u0);

// G-Buffers
RWTexture2DArray<uint> GPhotonCount : register(u1);
RWTexture2DArray<uint> GPhotonScan : register(u2);
RWTexture2DArray<uint> GPhotonTempIndex : register(u3);

RWTexture2DArray<float4> GPhotonPos : register(u4);
RWTexture2DArray<float4> GPhotonColor : register(u5);
RWTexture2DArray<float4> GPhotonSortedPos : register(u6);
RWTexture2DArray<float4> GPhotonSortedCol : register(u7);

ConstantBuffer<PixelMajorComputeConstantBuffer> CKernelParams : register(b0);

// Zero out the initial buffer
[numthreads(blocksize, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    GPhotonPos[DTid] = float4(0.0, 0.0, 0.0, -1.0);
    
    // Debug
	/*
    uint3 index;
    index.x = CELL_1D_TO_3D_X(DTid.x);
    index.y = CELL_1D_TO_3D_Y(DTid.x);
    index.z = CELL_1D_TO_3D_Z(DTid.x);
    GPhotonCount[index] = DTid.x;
    GPhotonScan[index] = index.y;
    //GPhotonTempIndex[index] = index.z;

    GPhotonCount[DTid] = GPhotonCount[DTid];
    GPhotonScan[DTid] = GPhotonScan[DTid];
    GPhotonTempIndex[DTid] = GPhotonTempIndex[DTid];
	*/
    
}

#endif // COMPUTE_PASS_0
