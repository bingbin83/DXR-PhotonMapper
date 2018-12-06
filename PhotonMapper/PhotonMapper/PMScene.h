#pragma once

#include <array>
#include <map>

#include "RaytracingHlslCompat.h"

namespace DXRPhotonMapper
{
    typedef unsigned char ByteBuffer; 

    struct Geometry
    {
        std::vector<Vertex> m_vertices;
        std::vector<Index> m_indices;
    };

    struct Material
    {
        DirectX::XMFLOAT3 m_baseColor;
    };

    struct Light
    {
        DirectX::XMFLOAT3 m_lightPos;
        DirectX::XMFLOAT3 m_lightColor;
    };

    struct Camera
    {
        DirectX::XMFLOAT3 m_cameraPos;
        DirectX::XMFLOAT3 m_eye;
        DirectX::XMFLOAT3 m_look;
        DirectX::XMFLOAT3 m_ref;
        float m_fov;
    };

    typedef struct 
    {
        const ByteBuffer* m_bufferStart;
        std::size_t m_length;
    }BufferHolder;

    class PMScene
    {
    private:

        std::array<UINT, 2> m_screenSize;
	    std::vector<Geometry> m_sceneGeoms;
	    std::vector<Material> m_sceneMaterials;

        
        std::map<std::string, BufferHolder> m_gltfBufferHolders;

        

    public:
        //------------------------------------------------------
        // Constructor
        //------------------------------------------------------
        PMScene(UINT width, UINT height);

        //------------------------------------------------------
        // Destructor
        //------------------------------------------------------
        ~PMScene();

        //------------------------------------------------------
        // LoadGLTFScene
        //------------------------------------------------------
	    bool LoadGLTFScene(const std::string& fileName);

        //------------------------------------------------------
        // LoadJSONScene
        //------------------------------------------------------
	    bool LoadJSONScene(const std::string& fileName);
    };

}

