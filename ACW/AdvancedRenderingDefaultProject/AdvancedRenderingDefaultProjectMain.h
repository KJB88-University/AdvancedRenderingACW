﻿#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include <memory>

// Renders Direct2D and 3D content on the screen.
namespace AdvancedRenderingDefaultProject
{
	class AdvancedRenderingDefaultProjectMain : public DX::IDeviceNotify
	{
	public:
		AdvancedRenderingDefaultProjectMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~AdvancedRenderingDefaultProjectMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		void AdvancedRenderingDefaultProjectMain::KeyDown(const int keyCode);
		void AdvancedRenderingDefaultProjectMain::KeyUp(const int keyCode);
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}