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
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_snakeVS;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_snakeGS;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_snakeTexture; // TODO
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_snakePointsLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_snakeBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_snakeIndexBuffer;
		uint32 m_snakeIndexCount;

		// Implicit Objects
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_implicitVS;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_implicitPS;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_implicitInput;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_implicitBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_implicitIndexBuffer;
		uint32 m_implicitIndexCount;

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

		// Samplers
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

		// Control vars
		float m_isRepeating = 0;
		float m_isDeforming = 0;
		float m_isFractal = 0;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}

