#pragma once

namespace AdvancedRenderingDefaultProject
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct TimeBuffer
	{
		float deltaTime;
		DirectX::XMFLOAT3 padding;
	};

	struct ControlBuffer
	{
		DirectX::XMFLOAT3 booleans;
	};

	struct CameraBuffer
	{
		DirectX::XMFLOAT4 eyePos;
		DirectX::XMFLOAT4 lookAt;
		DirectX::XMFLOAT4 clipPlanes;
	};

	struct VertexPosition
	{
		DirectX::XMFLOAT3 pos;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};
}