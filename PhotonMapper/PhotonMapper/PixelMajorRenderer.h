#pragma once

#include "PhotonBaseRenderer.h"
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
class PixelMajorRenderer : public PhotonBaseRenderer
{

public:
    PixelMajorRenderer(const DXRPhotonMapper::PMScene& scene, UINT width, UINT height, std::wstring name);

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
    static const UINT NumRenderTargets = 1;
    static const UINT NumGBuffers = 4;
    static const UINT NumPhotonCountBuffer = 1;
    static const UINT NumPhotonScanBuffer = 1;
    static const UINT NumPhotonTempIndexBuffer = 1;
    static const UINT NumPhotons = 100000;

	bool cameraNeedsUpdate = false;

    // We'll allocate space for several of these and they will need to be padded for alignment.
    static_assert(sizeof(SceneConstantBuffer) < D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "Checking the size here.");

    union AlignedSceneConstantBuffer
    {
        SceneConstantBuffer constants;
        uint8_t alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
    };

	union AlignedComputeConstantBuffer
	{
		PixelMajorComputeConstantBuffer constants;
		uint8_t alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
	};

    AlignedSceneConstantBuffer*  m_mappedConstantData;
    ComPtr<ID3D12Resource>       m_perFrameConstants;

	AlignedComputeConstantBuffer*  m_mappedComputeConstantData;
	ComPtr<ID3D12Resource> m_computeConstantRes;

    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackFirstPassStateObject;
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackSecondPassStateObject;

    ComPtr<ID3D12StateObject> m_dxrFirstPassStateObject;
    ComPtr<ID3D12StateObject> m_dxrSecondPassStateObject;

	// Compute Stage attributes
	ComPtr<ID3D12PipelineState> m_computeInitializePSO;
	ComPtr<ID3D12PipelineState> m_computeInitializePSO2;
	ComPtr<ID3D12PipelineState> m_computeFirstPassPSO;
	ComPtr<ID3D12PipelineState> m_computeSecondPassPSO;
	ComPtr<ID3D12PipelineState> m_computeThirdPassPSO;

    // Root signatures for the first pass
    ComPtr<ID3D12RootSignature> m_firstPassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_firstPassLocalRootSignature;
	
	// Root signatures for the second pass
	ComPtr<ID3D12RootSignature> m_secondPassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_secondPassLocalRootSignature;

	// Root signature for the compute pass
    ComPtr<ID3D12RootSignature> m_computeRootSignature;

    // Raytracing scene
    SceneConstantBuffer m_sceneCB[FrameCount];
    CubeConstantBuffer m_cubeCB;

	// Compute Constant Buffer
	PixelMajorComputeConstantBuffer m_computeConstantBuffer;

    UINT m_gBufferWidth;
    UINT m_gBufferHeight;
    UINT m_gBufferDepth;
    std::vector<GBuffer> m_gBuffers;

	GBuffer m_photonCountBuffer;
	GBuffer m_photonScanBuffer;
	GBuffer m_photonTempIndexBuffer;


    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
    UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

    // Shader tables
    static const wchar_t* c_hitGroupName;
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_closestHitShaderName;
    static const wchar_t* c_missShaderName;

    static const LPCWSTR c_computeShaderPass0;
    static const LPCWSTR c_computeShaderPass01;
    static const LPCWSTR c_computeShaderPass1;
    static const LPCWSTR c_computeShaderPass2;
	static const LPCWSTR c_computeShaderPass3;

	struct ShaderTableRes
	{
		ComPtr<ID3D12Resource> m_missShaderTable;
		ComPtr<ID3D12Resource> m_hitGroupShaderTable;
		ComPtr<ID3D12Resource> m_rayGenShaderTable;
	};

	ShaderTableRes m_firstPassShaderTableRes;
	ShaderTableRes m_secondPassShaderTableRes;


    // Application state
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
	void DoComputePass(ComPtr<ID3D12PipelineState>& computePSO, UINT xThreads, UINT yThreads, UINT zThreads);

    void CreateConstantBuffers();
	void CreateComputeConstantBuffer();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void CreateRaytracingInterfaces();
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);

    void CreateFirstPassRootSignatures();
    void CreateSecondPassRootSignatures();
	void CreateComputeFirstPassRootSignature();


    void CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline, ComPtr<ID3D12RootSignature>* rootSig);
    void CreateFirstPassPhotonPipelineStateObject();
    void CreateSecondPassPhotonPipelineStateObject();
	void CreateComputePipelineStateObject(const LPCWSTR& compiledShaderName, ComPtr<ID3D12PipelineState>& computePipeline);
    void CreateDescriptorHeap();
    void CreateRaytracingOutputResource();
    void CreateGBuffers();
	void CreatePhotonCountBuffer(GBuffer& gBuffer);

    void BuildFirstPassShaderTables();
    void BuildSecondPassShaderTables();
    void SelectRaytracingAPI(RaytracingAPI type);
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer();
	void CopyUAVData(GBuffer& source, GBuffer& destination);
    void CopyGBUfferToBackBuffer(UINT gbufferIndex);
    void CalculateFrameStats();

    // fence for exclusive scan
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
    Microsoft::WRL::Wrappers::Event m_fenceEvent;
    void ScanWaitForGPU(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);
};
