#pragma once
#include "ChiliWin.h"
#include "d3d11.h"



class Graphics
{
public:
	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator = (const Graphics&) = delete;
	~Graphics();
	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept
	{
		// The fourth value (1.0f) represents the alpha channel (fully opaque).
		const float color[] = { red, green, blue, 1.0f };

		// Clears the render target (the area where rendering is performed) with the specified color.
		pContext->ClearRenderTargetView(pTarget, color);
	}
private:
	ID3D11Device* pDevice = nullptr; // Represents the Direct3D device used to manage GPU resources.
	IDXGISwapChain* pSwap = nullptr; // Manages the swap chain for presenting frames to the screen.
	ID3D11DeviceContext* pContext = nullptr; // Executes rendering commands on the GPU.
	ID3D11RenderTargetView* pTarget = nullptr; // Represents the render target (back buffer) where the GPU draws.

};

