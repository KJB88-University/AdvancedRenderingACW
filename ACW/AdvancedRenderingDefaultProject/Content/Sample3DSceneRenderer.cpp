#include "pch.h"
#include "Sample3DSceneRenderer.h"
#include "..\Common\DDSTextureLoader.h"
#include "..\Common\DirectXHelper.h"
#include <random>

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

	float nearPlane = 0.01f;
	float farPlane = 1000.0f;
	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		nearPlane,
		farPlane
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 1.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	static const XMVECTORF32 clipPlanes = { nearPlane, farPlane, 0.0f, 0.0f };

	// Load the camera CB
	XMStoreFloat4(&m_cameraBufferData.eyePos, eye);
	XMStoreFloat4(&m_cameraBufferData.lookAt, at);
	XMStoreFloat4(&m_cameraBufferData.clipPlanes, clipPlanes);

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	m_isRepeating = 0;
	m_isDeforming = 0;
	m_isFractal = 0;

	// Load the control CB
	XMStoreFloat4(&m_controlBufferData.booleans, XMVECTORF32{ m_isRepeating, m_isDeforming, m_isFractal, m_isShiny });
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	double time = timer.GetTotalSeconds();
	m_timeBufferData.deltaTime = (float)time;
	XMStoreFloat3(&m_timeBufferData.padding, XMVECTORF32{ 0.0f, 0.0f, 0.0f });

	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation * 0.5f, XM_2PI));

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

	// COMMON STATEs / SHADERs
	auto context = m_deviceResources->GetD3DDeviceContext();
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	context->UpdateSubresource1(m_timeBuffer.Get(), 0, NULL, &m_timeBufferData, 0, 0, 0);
	context->UpdateSubresource1(m_cameraBuffer.Get(), 0, NULL, &m_cameraBufferData, 0, 0, 0);
	context->UpdateSubresource1(m_controlBuffer.Get(), 0, NULL, &m_controlBufferData, 0, 0, 0);

	UINT stride;
	UINT offset;
	if (!m_isImplicit)
	{
		stride = sizeof(VertexPositionColor);
		offset = 0;

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->RSSetState(m_filledRasterState.Get());
		context->OMSetDepthStencilState(m_noDepthStencil.Get(), 0);
		context->OMSetBlendState(m_blend.Get(), 0, 0xffffffff);
		// FLOOR QUAD
#pragma region FLOOR
		context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, .0);
		context->IASetInputLayout(m_inputLayout.Get());

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
		context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
		context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

		context->HSSetShader(m_hullShader.Get(), nullptr, 0);
		context->DSSetShader(m_domainShader.Get(), nullptr, 0);
		context->DSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
		context->DSSetConstantBuffers(1, 1, m_timeBuffer.GetAddressOf());
		context->DSSetSamplers(0, 1, m_sampler.GetAddressOf());
		context->DSSetShaderResources(0, 1, m_displacementMap.GetAddressOf());

		context->GSSetShader(NULL, nullptr, 0);
		context->DrawIndexed(m_indexCount, 0, 0);
#pragma endregion

	// SNAKE POLYLINE
#pragma region SNAKE
	// ia
		stride = sizeof(VertexPosition);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_snakeBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_snakeIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, .0);
		context->IASetInputLayout(m_snakePointsLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//context->RSSetState(m_filledNoCullRasterState.Get());
		context->RSSetState(m_filledNoCullRasterState.Get());

		// vs
		context->VSSetShader(m_snakeVS.Get(), nullptr, 0);
		context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
		context->VSSetConstantBuffers(1, 1, m_timeBuffer.GetAddressOf());

		// hs ds clear
		context->HSSetShader(NULL, nullptr, 0);
		context->DSSetShader(NULL, nullptr, 0);

		// gs
		context->GSSetShader(m_snakeGS.Get(), nullptr, 0);
		context->GSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
		context->GSSetConstantBuffers(1, 1, m_timeBuffer.GetAddressOf());

		// ps
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->PSSetShader(m_snakePS.Get(), nullptr, 0);
		context->PSSetShaderResources(0, 1, m_snakeTex.GetAddressOf());
		context->DrawIndexed(m_snakeIndexCount, 0, 0);

		context->IASetVertexBuffers(0, 1, m_snakeBuffer2.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_snakeIndexBuffer2.Get(), DXGI_FORMAT_R16_UINT, .0);
		context->DrawIndexed(m_snakeIndexCount, 0, 0);
#pragma endregion

//		// PARTICLES
#pragma region PARTICLES
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

		context->GSSetShader(NULL, nullptr, 0);
		context->GSSetShader(m_grassGS.Get(), nullptr, 0);
		context->GSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
		context->GSSetConstantBuffers(1, 1, m_timeBuffer.GetAddressOf());
		context->GSSetSamplers(0, 1, m_sampler.GetAddressOf());

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		context->PSSetShader(m_grassPS.Get(), nullptr, 0);
		context->PSSetShaderResources(0, 1, m_grassTexture.GetAddressOf());
		context->DrawIndexed(m_grassIndexCount, 0, 0);
		//context->OMSetBlendState(NULL, 0, 0);
#pragma endregion

//		// PARAMETRIC TORUS
		context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, .0);
		context->IASetInputLayout(m_parametricIL.Get());
		context->RSSetState(m_wireframeRasterState.Get());

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		context->VSSetShader(m_parametricVS.Get(), nullptr, 0);
		context->PSSetShader(m_parametricPS.Get(), nullptr, 0);

		context->HSSetShader(m_parametricHS.Get(), nullptr, 0);
		context->DSSetShader(m_parametricDS.Get(), nullptr, 0);
		context->DSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

		context->GSSetShader(NULL, nullptr, 0);
		context->DrawIndexed(m_indexCount, 0, 0);

		// PARAMETRIC SPHERE
		context->RSSetState(m_wireframeRasterState.Get());
		context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, .0);
		context->IASetInputLayout(m_parametricIL.Get());
		context->RSSetState(m_wireframeRasterState.Get());

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		context->VSSetShader(m_parametricVS.Get(), nullptr, 0);
		context->PSSetShader(m_parametricPS.Get(), nullptr, 0);

		context->HSSetShader(m_parametricSphereHS.Get(), nullptr, 0);
		context->DSSetShader(m_parametricSphereDS.Get(), nullptr, 0);
		context->DSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

		context->GSSetShader(NULL, nullptr, 0);
		context->DrawIndexed(m_indexCount, 0, 0);
	}
	// IMPLICIT
	else
	{
		// PS IMPLICITS
		stride = sizeof(VertexPositionColor);
		offset = 0;
		context->RSSetState(m_filledRasterState.Get());
		context->IASetVertexBuffers(0, 1, m_implicitBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_implicitIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, .0);
		context->IASetInputLayout(m_implicitInput.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->VSSetShader(m_implicitVS.Get(), nullptr, 0);
		context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

		context->PSSetShader(m_implicitPS.Get(), nullptr, 0);
		context->PSSetConstantBuffers(0, 1, m_timeBuffer.GetAddressOf());
		context->PSSetConstantBuffers(1, 1, m_controlBuffer.GetAddressOf());

		context->DSSetShader(NULL, nullptr, 0);
		context->HSSetShader(NULL, nullptr, 0);
		context->GSSetShader(NULL, nullptr, 0);

		context->DrawIndexed(m_implicitIndexCount, 0, 0);

	}
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	// VS
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadVSTask2 = DX::ReadDataAsync(L"GrassVS.cso");
	auto loadVSTask3 = DX::ReadDataAsync(L"SnakeVS.cso");
	auto loadVSTask4 = DX::ReadDataAsync(L"ImplicitVS.cso");
	auto loadVSTask5 = DX::ReadDataAsync(L"ParametricVS.cso");

	// PS
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	auto loadPSTask2 = DX::ReadDataAsync(L"ImplicitPixelShader.cso");
	auto loadPSTask3 = DX::ReadDataAsync(L"SnakePS.cso");
	auto loadPSTask4 = DX::ReadDataAsync(L"ParametricPS.cso");
	auto loadPSTask5 = DX::ReadDataAsync(L"GrassPS.cso");

	// HS & DS
	auto loadHSTask = DX::ReadDataAsync(L"HullShader.cso");
	auto loadDSTask = DX::ReadDataAsync(L"DomainShader.cso");
	auto loadHSTask2 = DX::ReadDataAsync(L"ParametricHS.cso");
	auto loadDSTask2 = DX::ReadDataAsync(L"ParametricDS.cso");
	auto loadHSTask3 = DX::ReadDataAsync(L"ParametricSphereHS.cso");
	auto loadDSTask3 = DX::ReadDataAsync(L"ParametricSphereDS.cso");

	// GS
	auto loadGSParticleTask = DX::ReadDataAsync(L"GrassParticleGS.cso");
	auto loadGSSnakeTask = DX::ReadDataAsync(L"SnakeGS.cso");

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

	// Snake Vertex Shader
	auto createVSTask3 = loadVSTask3.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_snakeVS
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
				&m_snakePointsLayout
			)
		);
	});

	// Implicit Vertex Shader
	auto createVSTask4 = loadVSTask4.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_implicitVS
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
				&m_implicitInput
			)
		);
	});

	// Parametric Shapes Vertex Shader
	auto createVStask5 = loadVSTask5.then([this](const std::vector<byte>&fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_parametricVS
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
				&m_parametricIL
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
	});

	// Implicit Pixel Shader = loadPS
	auto createPSTask2 = loadPSTask2.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_implicitPS
			)
		);
	});

	// Snake
	auto createPSTask3 = loadPSTask3.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_snakePS
			)
		);
	});

	// Parametric Shapes Pixel Shader
	auto createPStask4 = loadPSTask4.then([this](const std::vector<byte>&fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_parametricPS
			)
		);
	});

	// Grass PS
	auto createPSTask5 = loadPSTask5.then([this](std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_grassPS
			)
		);
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

	// Parametric Shapes Hull Shader
	auto createHSTask2 = loadHSTask2.then([this](const std::vector<byte>&fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateHullShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_parametricHS
			)
		);
	});

	// Parametric Sphere HS
	auto createHSTask3 = loadHSTask3.then([this](std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateHullShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_parametricSphereHS
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

	// Parametric Shapes Domain Shader
	auto createDSTask2 = loadDSTask2.then([this](const std::vector<byte>&fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateDomainShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_parametricDS
			)
		);
	});

	// Parametric Sphere Domain Shader
	auto createDSTask3 = loadDSTask3.then([this](const std::vector<byte>&fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateDomainShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_parametricSphereDS
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
		HRESULT result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"grass1.dds", nullptr, &m_grassTexture);
	});

	// Snake Geometry Shader
	auto createSnakeGSTask = loadGSSnakeTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_snakeGS
			)
		);
		// Snake Texture
		HRESULT result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"snaketex2.dds", nullptr, &m_snakeTex);
	});

	// Grass Plane Points
	auto createGrassPlaneTask = (createPSTask && createVSTask && createHSTask && createDSTask && createGSParticleTask && createVSTask2 && createVSTask3 && createSnakeGSTask).then([this]()
	{
		//const float minX = -0.5f;
		//const float maxX = 0.5f;
		const float staticY = 0.02f;
		//const float minZ = -0.5f;
		//const float maxZ = 0.5f;

		static VertexPosition grassPointsRandom[1000];
		static unsigned int grassPointIndices[1000];

		std::random_device random;
		std::mt19937 mt(random());
		std::uniform_real_distribution<float> distrib(-0.95f, 0.95f);
		for (int i = 0; i < 200; i++)
		{
			grassPointsRandom[i] = 
				VertexPosition{ XMFLOAT3(distrib(mt), staticY, distrib(mt)) };

			grassPointIndices[i] = i;
		}

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = grassPointsRandom;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(grassPointsRandom), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				m_grassBuffer.GetAddressOf()
			)
		);

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

	// Snake Points
	auto createSnakeTask = (createPSTask && createVSTask && createHSTask && createDSTask && createGSParticleTask && createVSTask2 && createVSTask3 && createSnakeGSTask).then([this]()
	{
		static const VertexPosition snakePoints[] =
		{
			XMFLOAT3(-0.5f, 0.2f, -0.8f),
			XMFLOAT3(-0.5f, 0.2f, -0.6f),
			XMFLOAT3(-0.5f, 0.2f, -0.4f),
			XMFLOAT3(-0.5f, 0.2f, -0.2f),
			XMFLOAT3(-0.5f, 0.2f, 0.0f),
			XMFLOAT3(-0.5f, 0.2f, 0.2f),
			XMFLOAT3(-0.5f, 0.2f, 0.4f),
			XMFLOAT3(-0.5f, 0.2f, 0.6f)
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = snakePoints;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(snakePoints), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				m_snakeBuffer.GetAddressOf()
			)
		);
		static const unsigned short snakePointIndices[] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 4,
			4, 5,
			5, 6,
			6, 7
		};

		m_snakeIndexCount = ARRAYSIZE(snakePointIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = snakePointIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(snakePointIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_snakeIndexBuffer
			)
		);
	});
	auto createSnakeTask2 = (createPSTask && createVSTask && createHSTask && createDSTask && createGSParticleTask && createVSTask2 && createVSTask3 && createSnakeGSTask).then([this]()
	{
		static const VertexPosition snakePoints[] =
		{
			XMFLOAT3(0.5f, 0.2f, -0.8f),
			XMFLOAT3(0.5f, 0.2f, -0.6f),
			XMFLOAT3(0.5f, 0.2f, -0.4f),
			XMFLOAT3(0.5f, 0.2f, -0.2f),
			XMFLOAT3(0.5f, 0.2f, 0.0f),
			XMFLOAT3(0.5f, 0.2f, 0.2f),
			XMFLOAT3(0.5f, 0.2f, 0.4f),
			XMFLOAT3(0.5f, 0.2f, 0.6f)
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = snakePoints;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(snakePoints), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				m_snakeBuffer2.GetAddressOf()
			)
		);
		static const unsigned short snakePointIndices[] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 4,
			4, 5,
			5, 6,
			6, 7
		};

		m_snakeIndexCount = ARRAYSIZE(snakePointIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = snakePointIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(snakePointIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_snakeIndexBuffer2
			)
		);
	});
	
	// Floor Quad Mesh
	auto createCubeTask = (createPSTask && createVSTask && createHSTask && createDSTask && createGSParticleTask && createVSTask2 && createVSTask3 && createSnakeGSTask && createDSTask2 && createHSTask2 && createDSTask3 && createHSTask3).then([this]() {

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

		 HRESULT result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"rockyDisp.dds", nullptr, &m_displacementMap);
		 result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"rockyNormal.dds", nullptr, &m_normalMap);
	});

	// Implicit Placeholder 'Mesh'
	auto createImplicitTask = (createPSTask2 && createVSTask4 && createPSTask && createVSTask && createHSTask && createDSTask && createGSParticleTask && createVSTask2 && createVSTask3 && createSnakeGSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor implicitPlaceholder[] =
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
		vertexBufferData.pSysMem = implicitPlaceholder;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(implicitPlaceholder), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_implicitBuffer
			)
		);

		static const unsigned short implicitPlaceholderIndices[] =
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

		m_implicitIndexCount = ARRAYSIZE(implicitPlaceholderIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = implicitPlaceholderIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(implicitPlaceholderIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				m_implicitIndexBuffer.GetAddressOf()
			)
		);
	});

	// COMMON
#pragma region COMMON
	// Rasterizer States
	CD3D11_RASTERIZER_DESC m_wireframeRasterDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	m_wireframeRasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	m_wireframeRasterDesc.CullMode = D3D11_CULL_NONE;
	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&m_wireframeRasterDesc, m_wireframeRasterState.GetAddressOf());

	CD3D11_RASTERIZER_DESC m_filledRasterDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	m_filledRasterDesc.FillMode = D3D11_FILL_SOLID;
	m_filledRasterDesc.CullMode = D3D11_CULL_BACK;
	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&m_filledRasterDesc, m_filledRasterState.GetAddressOf());

	CD3D11_RASTERIZER_DESC m_filledNoCullRasterDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	m_filledNoCullRasterDesc.FillMode = D3D11_FILL_SOLID;
	m_filledNoCullRasterDesc.CullMode = D3D11_CULL_NONE;
	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&m_filledNoCullRasterDesc, m_filledNoCullRasterState.GetAddressOf());

	// Constant Buffers
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			&m_constantBuffer
		)
	);

	CD3D11_BUFFER_DESC cameraBufferDesc(sizeof(CameraBuffer), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&cameraBufferDesc,
			nullptr,
			&m_cameraBuffer
		)
	);

	CD3D11_BUFFER_DESC timeBufferDesc(sizeof(TimeBuffer), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&timeBufferDesc,
			nullptr,
			&m_timeBuffer
		)
	);

	CD3D11_BUFFER_DESC controlBufferDesc(sizeof(ControlBuffer), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&controlBufferDesc,
			nullptr,
			m_controlBuffer.GetAddressOf()
		)
	);

	// Sampler
	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateSamplerState(&samplerDesc, m_sampler.GetAddressOf()));

	// Depth Stencil State
	CD3D11_DEPTH_STENCIL_DESC noDepthStencilDesc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
	noDepthStencilDesc.DepthEnable = false;
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateDepthStencilState(&noDepthStencilDesc, m_noDepthStencil.GetAddressOf()));

	CD3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
	depthStencilDesc.DepthEnable = true;
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateDepthStencilState(&depthStencilDesc, m_noDepthStencil.GetAddressOf()));

	CD3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(D3D11_DEFAULT);
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	m_deviceResources->GetD3DDevice()->CreateBlendState(&blendDesc, m_blend.GetAddressOf());
#pragma endregion

	// Join block
	createCubeTask.then([this]() {
		m_loadingComplete = true;
	});

}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	// BOOLS
	m_loadingComplete = false;

	// BUFFERS
	m_constantBuffer.Reset();
	m_timeBuffer.Reset();
	m_cameraBuffer.Reset();

	// COMMON
	m_pixelShader.Reset();
	m_wireframeRasterState.Reset();
	m_filledRasterState.Reset();
	m_sampler.Reset();

	// FLOOR
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
	m_hullShader.Reset();
	m_domainShader.Reset();

	// GRASS
	m_grassGS.Reset();
	m_grassVertexShader.Reset();
	m_grassTexture.Reset();
	m_grassPointsLayout.Reset();
	m_grassBuffer.Reset();
	m_grassIndexBuffer.Reset();

	// SNAKE
	m_snakeGS.Reset();
	m_snakeVS.Reset();
	m_snakeBuffer.Reset();
	m_snakeIndexBuffer.Reset();
	m_snakePointsLayout.Reset();

	// D3D Resources
	m_deviceResources.reset();
}