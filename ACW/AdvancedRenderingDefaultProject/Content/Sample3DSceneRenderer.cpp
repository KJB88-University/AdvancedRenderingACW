#include "pch.h"
#include "Sample3DSceneRenderer.h"
#include "..\Common\DDSTextureLoader.h"
#include "..\Common\DirectXHelper.h"

using namespace AdvancedRenderingDefaultProject;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, .0);
	context->IASetInputLayout(m_inputLayout.Get());

	// COMMON STATEs / SHADERs
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetState(m_rasterState.Get());

	// FLOOR QUAD
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

	context->DSSetShader(m_domainShader.Get(), nullptr, 0);
	context->DSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	context->HSSetShader(m_hullShader.Get(), nullptr, 0);

	context->GSSetShader(NULL, nullptr, 0);
	context->DrawIndexed(m_indexCount, 0, 0);

	// PARTICLES
	stride = sizeof(VertexPosition);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_grassBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_grassIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, .0);
	context->IASetInputLayout(m_grassPointsLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetState(m_filledRasterState.Get());

	context->VSSetShader(m_grassVertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	context->HSSetShader(NULL, nullptr, 0);
	context->DSSetShader(NULL, nullptr, 0);

	context->GSSetShader(m_grassGS.Get(), nullptr, 0);
	context->GSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	context->GSSetShaderResources(0, 1, m_grassTexture.GetAddressOf());
	context->GSSetSamplers(0, 1, m_sampler.GetAddressOf());

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->DrawIndexed(m_grassIndexCount, 0, 0);
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	// VS
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadVSTask2 = DX::ReadDataAsync(L"VertexShaderPositionOnly.cso");

	// PS
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// HS & DS
	auto loadHSTask = DX::ReadDataAsync(L"HullShader.cso");
	auto loadDSTask = DX::ReadDataAsync(L"DomainShader.cso");

	// GS
	auto loadGSParticleTask = DX::ReadDataAsync(L"GrassParticleGS.cso");

	// Passthrough Vertex Shader
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
	});

	// Grass Vertex Shader
	auto createVSTask2 = loadVSTask2.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_grassVertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_grassPointsLayout
			)
		);
	});

	// Basic Pixel Shader
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);

		// Rasterizer State
		CD3D11_RASTERIZER_DESC m_wireframeRasterDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
		m_wireframeRasterDesc.FillMode = D3D11_FILL_WIREFRAME;
		m_deviceResources->GetD3DDevice()->CreateRasterizerState(&m_wireframeRasterDesc, m_rasterState.GetAddressOf());

		CD3D11_RASTERIZER_DESC m_filledRasterDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
		m_filledRasterDesc.FillMode = D3D11_FILL_SOLID;
		m_deviceResources->GetD3DDevice()->CreateRasterizerState(&m_filledRasterDesc, m_filledRasterState.GetAddressOf());

	});

	// Floor Quad Hull Shader
	auto createHSTask = loadHSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateHullShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_hullShader
			)
		);
	});

	// Floor Quad Domain Shader
	auto createDSTask = loadDSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateDomainShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_domainShader
			)
		);
	});

	// Particle Geometry Shader
	auto createGSParticleTask = loadGSParticleTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_grassGS
			)
		);

		// Grass Blade Texture
		HRESULT result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"grassBlade.dds", nullptr, &m_grassTexture);

		// Sampler
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateSamplerState(&samplerDesc, m_sampler.GetAddressOf()));
	});

	// Grass Plane Points
	auto createGrassPlaneTask = (createPSTask && createVSTask && createHSTask && createDSTask && createGSParticleTask && createVSTask2).then([this]()
	{
		const float minX = -0.5f;
		const float maxX = 0.5f;
		const float staticY = 0.02f;
		const float minZ = -0.5f;
		const float maxZ = 0.5f;

		static const VertexPosition grassPoints[] =
		{
			// First Row
			XMFLOAT3(minX, staticY, minZ), // Back Left
			XMFLOAT3(-0.4f, staticY, minZ),
			XMFLOAT3(-0.3f, staticY, minZ),
			XMFLOAT3(-0.3f, staticY, minZ),
			XMFLOAT3(-0.2f, staticY, minZ),
			XMFLOAT3(-0.1f, staticY, minZ),
			XMFLOAT3(0.0f, staticY, minZ),
			XMFLOAT3(0.1f, staticY, minZ),
			XMFLOAT3(0.2f, staticY, minZ),
			XMFLOAT3(0.3f, staticY, minZ),
			XMFLOAT3(0.4f, staticY, minZ),
			XMFLOAT3(maxX, staticY, minZ), 	// Back Right

			// Second Row
			XMFLOAT3(minX, staticY, -0.4f),
			XMFLOAT3(-0.4f, staticY, -0.4f),
			XMFLOAT3(-0.3f, staticY, -0.4f),
			XMFLOAT3(-0.3f, staticY, -0.4f),
			XMFLOAT3(-0.2f, staticY, -0.4f),
			XMFLOAT3(-0.1f, staticY, -0.4f),
			XMFLOAT3(0.0f, staticY, -0.4f),
			XMFLOAT3(0.1f, staticY, -0.4f),
			XMFLOAT3(0.2f, staticY, -0.4f),
			XMFLOAT3(0.3f, staticY, -0.4f),
			XMFLOAT3(0.4f, staticY, -0.4f),
			XMFLOAT3(maxX, staticY, -0.4f),

			// Third Row
			XMFLOAT3(minX, staticY, -0.3f),
			XMFLOAT3(-0.4f, staticY, -0.3f),
			XMFLOAT3(-0.3f, staticY, -0.3f),
			XMFLOAT3(-0.3f, staticY, -0.3f),
			XMFLOAT3(-0.2f, staticY, -0.3f),
			XMFLOAT3(-0.1f, staticY, -0.3f),
			XMFLOAT3(0.0f, staticY, -0.3f),
			XMFLOAT3(0.1f, staticY, -0.3f),
			XMFLOAT3(0.2f, staticY, -0.3f),
			XMFLOAT3(0.3f, staticY, -0.3f),
			XMFLOAT3(0.4f, staticY, -0.3f),
			XMFLOAT3(maxX, staticY, -0.3f),

			// Fourth Row
			XMFLOAT3(minX, staticY, -0.2f),
			XMFLOAT3(-0.4f, staticY, -0.2f),
			XMFLOAT3(-0.3f, staticY, -0.2f),
			XMFLOAT3(-0.3f, staticY, -0.2f),
			XMFLOAT3(-0.2f, staticY, -0.2f),
			XMFLOAT3(-0.1f, staticY, -0.2f),
			XMFLOAT3(0.0f, staticY, -0.2f),
			XMFLOAT3(0.1f, staticY, -0.2f),
			XMFLOAT3(0.2f, staticY, -0.2f),
			XMFLOAT3(0.3f, staticY, -0.2f),
			XMFLOAT3(0.4f, staticY, -0.2f),
			XMFLOAT3(maxX, staticY, -0.2f),

			// Fifth Row
			XMFLOAT3(minX, staticY, -0.1f),
			XMFLOAT3(-0.4f, staticY, -0.1f),
			XMFLOAT3(-0.3f, staticY, -0.1f),
			XMFLOAT3(-0.3f, staticY, -0.1f),
			XMFLOAT3(-0.2f, staticY, -0.1f),
			XMFLOAT3(-0.1f, staticY, -0.1f),
			XMFLOAT3(0.0f, staticY, -0.1f),
			XMFLOAT3(0.1f, staticY, -0.1f),
			XMFLOAT3(0.2f, staticY, -0.1f),
			XMFLOAT3(0.3f, staticY, -0.1f),
			XMFLOAT3(0.4f, staticY, -0.1f),
			XMFLOAT3(maxX, staticY, -0.1f),

			// Sixth Row
			XMFLOAT3(minX, staticY, 0.0f),
			XMFLOAT3(-0.4f, staticY, 0.0f),
			XMFLOAT3(-0.3f, staticY, 0.0f),
			XMFLOAT3(-0.3f, staticY, 0.0f),
			XMFLOAT3(-0.2f, staticY, 0.0f),
			XMFLOAT3(-0.1f, staticY, 0.0f),
			XMFLOAT3(0.0f, staticY, 0.0f),
			XMFLOAT3(0.1f, staticY, 0.0f),
			XMFLOAT3(0.2f, staticY, 0.0f),
			XMFLOAT3(0.3f, staticY, 0.0f),
			XMFLOAT3(0.4f, staticY, 0.0f),
			XMFLOAT3(maxX, staticY, 0.0f),

			// Seventh Row
			XMFLOAT3(minX, staticY, 0.1f),
			XMFLOAT3(-0.4f, staticY, 0.1f),
			XMFLOAT3(-0.3f, staticY, 0.1f),
			XMFLOAT3(-0.3f, staticY, 0.1f),
			XMFLOAT3(-0.2f, staticY, 0.1f),
			XMFLOAT3(-0.1f, staticY, 0.1f),
			XMFLOAT3(0.0f, staticY, 0.1f),
			XMFLOAT3(0.1f, staticY, 0.1f),
			XMFLOAT3(0.2f, staticY, 0.1f),
			XMFLOAT3(0.3f, staticY, 0.1f),
			XMFLOAT3(0.4f, staticY, 0.1f),
			XMFLOAT3(maxX, staticY, 0.1f),

			// Eighth Row
			XMFLOAT3(minX, staticY, 0.2f),
			XMFLOAT3(-0.4f, staticY, 0.2f),
			XMFLOAT3(-0.3f, staticY, 0.2f),
			XMFLOAT3(-0.3f, staticY, 0.2f),
			XMFLOAT3(-0.2f, staticY, 0.2f),
			XMFLOAT3(-0.1f, staticY, 0.2f),
			XMFLOAT3(0.0f, staticY, 0.2f),
			XMFLOAT3(0.1f, staticY, 0.2f),
			XMFLOAT3(0.2f, staticY, 0.2f),
			XMFLOAT3(0.3f, staticY, 0.2f),
			XMFLOAT3(0.4f, staticY, 0.2f),
			XMFLOAT3(maxX, staticY, 0.2f),

			// Ninth Row
			XMFLOAT3(minX, staticY, 0.3f),
			XMFLOAT3(-0.4f, staticY, 0.3f),
			XMFLOAT3(-0.3f, staticY, 0.3f),
			XMFLOAT3(-0.3f, staticY, 0.3f),
			XMFLOAT3(-0.2f, staticY, 0.3f),
			XMFLOAT3(-0.1f, staticY, 0.3f),
			XMFLOAT3(0.0f, staticY, 0.3f),
			XMFLOAT3(0.1f, staticY, 0.3f),
			XMFLOAT3(0.2f, staticY, 0.3f),
			XMFLOAT3(0.3f, staticY, 0.3f),
			XMFLOAT3(0.4f, staticY, 0.3f),
			XMFLOAT3(maxX, staticY, 0.3f),

			// Tenth Row
			XMFLOAT3(minX, staticY, 0.4f),
			XMFLOAT3(-0.4f, staticY, 0.4f),
			XMFLOAT3(-0.3f, staticY, 0.4f),
			XMFLOAT3(-0.3f, staticY, 0.4f),
			XMFLOAT3(-0.2f, staticY, 0.4f),
			XMFLOAT3(-0.1f, staticY, 0.4f),
			XMFLOAT3(0.0f, staticY, 0.4f),
			XMFLOAT3(0.1f, staticY, 0.4f),
			XMFLOAT3(0.2f, staticY, 0.4f),
			XMFLOAT3(0.3f, staticY, 0.4f),
			XMFLOAT3(0.4f, staticY, 0.4f),
			XMFLOAT3(maxX, staticY, 0.4f),

			// Eleventh Row
			XMFLOAT3(minX, staticY,  maxZ), // Front Left
			XMFLOAT3(-0.4f, staticY, maxZ),
			XMFLOAT3(-0.3f, staticY, maxZ),
			XMFLOAT3(-0.3f, staticY, maxZ),
			XMFLOAT3(-0.2f, staticY, maxZ),
			XMFLOAT3(-0.1f, staticY, maxZ),
			XMFLOAT3(0.0f, staticY, maxZ),
			XMFLOAT3(0.1f, staticY, maxZ),
			XMFLOAT3(0.2f, staticY, maxZ),
			XMFLOAT3(0.3f, staticY, maxZ),
			XMFLOAT3(0.4f, staticY, maxZ),
			XMFLOAT3(maxX, staticY,  maxZ) // Front Right

		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = grassPoints;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(grassPoints), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				m_grassBuffer.GetAddressOf()
			)
		);
		static const unsigned short grassPointIndices[] =
		{
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
			15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
			25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
			35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
			45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
			55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
			65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
			75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
			85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
			95, 96, 97, 98, 99, 100, 101, 102, 103, 104,
			105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
			115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
			125, 126, 127, 128, 129, 130, 131
			
		};

		m_grassIndexCount = ARRAYSIZE(grassPointIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = grassPointIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(grassPointIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_grassIndexBuffer
			)
		);

	});

	// Floor Quad Mesh
	auto createCubeTask = (createPSTask && createVSTask && createHSTask && createDSTask && createGSParticleTask && createVSTask2).then([this]() {

		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor floorQuad[] =
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = floorQuad;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(floorQuad), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short floorIndices[] =
		{
			0,2,1, // -x
			1,2,3,

			4,5,6, // +x
			5,7,6,

			0,1,5, // -y
			0,5,4,

			2,6,7, // +y
			2,7,3,

			0,4,6, // -z
			0,6,2,

			1,3,7, // +z
			1,7,5,
		};

		m_indexCount = ARRAYSIZE(floorIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = floorIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(floorIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				m_indexBuffer.GetAddressOf()
			)
		);
	});

	// Join block
	createCubeTask.then([this]() {
		m_loadingComplete = true;
	});
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
	m_hullShader.Reset();
	m_domainShader.Reset();
	m_grassGS.Reset();
}