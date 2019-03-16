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
		uint32	m_indexCount;

		// Particle Grass
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_grassVertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_grassGS;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_grassTexture;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_grassPointsLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_grassBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_grassIndexBuffer;
		uint32 m_grassIndexCount;

		// Snake Polyline
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_snakeVS; // TODO
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_snakeGS; // TODO
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_snakeTexture; // TODO

		// Constant Buffers
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		TimeBuffer m_timeBufferData;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_timeBuffer;

		// Rasterizer States
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_filledRasterState;

		// Samplers
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}

