#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

namespace AdvancedRenderingDefaultProject
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }


	private:
		void Rotate(float radians);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Floor Quad Tesselation
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11HullShader> m_hullShader;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> m_domainShader;

		// http://spiralgraphics.biz/packs/terrain_civilization/index.htm?14
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_displacementMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalMap;
		uint32	m_indexCount;

		// Particle Grass
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_grassVertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_grassGS;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_grassTexture;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_grassPointsLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_grassBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_grassIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_grassPS;
		uint32 m_grassIndexCount;

		// Snake Polyline
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_snakeVS;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_snakePS;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_snakeGS;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_snakePointsLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_snakeBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_snakeIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_snakeBuffer2;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_snakeIndexBuffer2;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_snakeTex;
		uint32 m_snakeIndexCount;

		// Implicit Objects
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_implicitVS;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_implicitPS;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_implicitInput;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_implicitBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_implicitIndexBuffer;
		uint32 m_implicitIndexCount;

		// Parametric Torus Object
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_parametricIL;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_parametricVS;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_parametricPS;
		Microsoft::WRL::ComPtr<ID3D11HullShader> m_parametricHS;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> m_parametricDS;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_parametricBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_parametricIndexBuffer;
		uint32 m_parametricIndexCount;

		// Parametric Sphere Object
		Microsoft::WRL::ComPtr<ID3D11HullShader> m_parametricSphereHS;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> m_parametricSphereDS;

		// Constant Buffers
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		TimeBuffer m_timeBufferData;
		CameraBuffer m_cameraBufferData;
		ControlBuffer m_controlBufferData;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_timeBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_cameraBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_controlBuffer;

		// Rasterizer States
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_wireframeRasterState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_filledRasterState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_filledNoCullRasterState;

		// Depth Stencil State
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_noDepthStencil;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencil;

		// Samplers
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

		// Blend State
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_blend;

		// Control vars
		bool m_isImplicit = false;
		float m_isRepeating = 0;
		float m_isDeforming = 0;
		float m_isFractal = 0;
		float m_isShiny = 0;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}

