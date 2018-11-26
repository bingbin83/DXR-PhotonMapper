#pragma once

#include "DXSample.h"
#include "StepTimer.h"
#include "RaytracingHlslCompat.h"
#include "Common.h"
#include "DirectXRaytracingHelper.h"

// The sample supports both Raytracing Fallback Layer and DirectX Raytracing APIs. 
// This is purely for demonstration purposes to show where the API differences are. 
// Real-world applications will implement only one or the other. 
// Fallback Layer uses DirectX Raytracing if a driver and OS supports it. 
// Otherwise, it falls back to compute pipeline to emulate raytracing.
// Developers aiming for a wider HW support should target Fallback Layer.
class PhotonMajorRenderer : public DXSample
{
    enum class RaytracingAPI {
        FallbackLayer,
        DirectXRaytracing,
    };

public:
    PhotonMajorRenderer(UINT width, UINT height, std::wstring name);

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    virtual void OnInit();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnDestroy();
    virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }

private:
    static const UINT FrameCount = 3;
    static const UINT NumGBuffers = 2;
    static const UINT NumRenderTargets = 2;
    static const UINT NumPhotons = 1000000;

    // We'll allocate space for several of these and they will need to be padded for alignment.
    static_assert(sizeof(SceneConstantBuffer) < D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "Checking the size here.");

    union AlignedSceneConstantBuffer
    {
        SceneConstantBuffer constants;
        uint8_t alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
    };
    AlignedSceneConstantBuffer*  m_mappedConstantData;
    ComPtr<ID3D12Resource>       m_perFrameConstants;

    // Raytracing Fallback Layer (FL) attributes
    ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice;
    ComPtr<ID3D12RaytracingFallbackCommandList> m_fallbackCommandList;

    // Fallback Pipeline State Objects for different passes
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackFirstPassStateObject;
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackSecondPassStateObject;
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackThirdPassStateObject;

    WRAPPED_GPU_POINTER m_fallbackTopLevelAccelerationStructurePointer;

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12Device5> m_dxrDevice;
    ComPtr<ID3D12GraphicsCommandList5> m_dxrCommandList;

    // DXR Pipeline State Objects for different passes
    ComPtr<ID3D12StateObject> m_dxrFirstPassStateObject;
    ComPtr<ID3D12StateObject> m_dxrSecondPassStateObject;
    ComPtr<ID3D12StateObject> m_dxrThirdPassStateObject;

    bool m_isDxrSupported;

    // Root signatures for the first pass
    ComPtr<ID3D12RootSignature> m_firstPassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_firstPassLocalRootSignature;
	
	// Root signatures for the second pass
	ComPtr<ID3D12RootSignature> m_secondPassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_secondPassLocalRootSignature;

    // Root signatures for the second pass
    ComPtr<ID3D12RootSignature> m_thirdPassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_thirdPassLocalRootSignature;

    // Descriptors
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorsAllocated;
    UINT m_descriptorSize;

    // Raytracing scene
    SceneConstantBuffer m_sceneCB[FrameCount];
    CubeConstantBuffer m_cubeCB;

    // Geometry
    struct D3DBuffer
    {
        ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
    };

    UINT m_gBufferWidth;
    UINT m_gBufferHeight;
    UINT m_gBufferDepth;

    struct GBuffer
    {
        ComPtr<ID3D12Resource> textureResource;
        D3D12_GPU_DESCRIPTOR_HANDLE uavGPUDescriptor;
        UINT uavDescriptorHeapIndex;
    };

    std::vector<GBuffer> m_gBuffers;

    D3DBuffer m_indexBuffer;
    D3DBuffer m_vertexBuffer;

    D3DBuffer m_indexBufferFloor;
    D3DBuffer m_vertexBufferFloor;

    // Acceleration structure
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
    ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;

    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
    UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

    GBuffer m_stagingRenderTarget;

    // Shader tables
    static const wchar_t* c_hitGroupName;
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_closestHitShaderName;
    static const wchar_t* c_missShaderName;

	struct ShaderTableRes
	{
		ComPtr<ID3D12Resource> m_missShaderTable;
		ComPtr<ID3D12Resource> m_hitGroupShaderTable;
		ComPtr<ID3D12Resource> m_rayGenShaderTable;
	};

	ShaderTableRes m_firstPassShaderTableRes;
	ShaderTableRes m_secondPassShaderTableRes;
	ShaderTableRes m_thirdPassShaderTableRes;

    // Application state
    RaytracingAPI m_raytracingAPI;
    bool m_forceComputeFallback;
	bool m_calculatePhotonMap;
    StepTimer m_timer;
    float m_curRotationAngleRad;
    XMVECTOR m_eye;
    XMVECTOR m_at;
    XMVECTOR m_up;

    void EnableDirectXRaytracing(IDXGIAdapter1* adapter);
    void ParseCommandLineArgs(WCHAR* argv[], int argc);
    void UpdateCameraMatrices();
    void InitializeScene();
    void RecreateD3D();

    void DoFirstPassPhotonMapping();
    void DoSecondPassPhotonMapping();
    void DoThirdPassPhotonMapping();

    void CreateConstantBuffers();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void CreateRaytracingInterfaces();
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);

    void CreateFirstPassRootSignatures();
    void CreateSecondPassRootSignatures();
    void CreateThirdPassRootSignatures();

    void CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline, ComPtr<ID3D12RootSignature>* rootSig);

    void CreateFirstPassPhotonPipelineStateObject();
    void CreateSecondPassPhotonPipelineStateObject();
    void CreateThirdPassPhotonPipelineStateObject();

    void CreateDescriptorHeap();
    void CreateRaytracingOutputResource();
    void CreateStagingRenderTargetResource();
    void CreateGBuffers();
    void BuildGeometry();
    void BuildAccelerationStructures();

    void BuildFirstPassShaderTables();
    void BuildSecondPassShaderTables();
    void BuildThirdPassShaderTables();

    void SelectRaytracingAPI(RaytracingAPI type);
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer();
    void CopyStagingBufferToBackBuffer();
    void CopyGBufferToBackBuffer(UINT gbufferIndex);
    void CalculateFrameStats();
    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
    UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
    WRAPPED_GPU_POINTER CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements);
};