#include "Graphics.h"

#pragma comment(lib, "d3d11.lib") //prevent linker error

Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd ={};
	sd.BufferDesc.Width = 0; // The width of the render target (default: match window size)
	sd.BufferDesc.Height = 0; //The height of the render target (default: match window size)
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //The format for the back buffer (8-bit BGR + alpha channel)
	sd.BufferDesc.RefreshRate.Numerator = 0; //Refresh rate numerator (0 for automatic configuration)
	sd.BufferDesc.RefreshRate.Denominator = 0; //Refresh rate denominator (0 for automatic configuration) => Manuel Example: Numerator = 60; Denominator = 1 => 60/1 = 60 Hz
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //Scaling mode for the buffer (unspecified, default behaviour)
	sd.SampleDesc.Count = 1; //Number of multisample counts (1 means no multisampling)
	sd.SampleDesc.Quality = 0; //Quality level of multisampling (0 for default)
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //Specifies the buffer usage (used as a render target)
	sd.BufferCount = 1; //Number of buffers (1 for double buffering)
	sd.OutputWindow = hWnd; //Handle to the window where Directx output will be displayed
	sd.Windowed = TRUE; //Window mode (TRUE for windowed mode, FALSE for fullscreen mode)
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //Specifies the swap effect (discard the back buffer content)
	sd.Flags = 0; //Additional flags for the swap chain (none specified here)

	//create device and front/back buffers, and swap chain and rendering context
	D3D11CreateDeviceAndSwapChain(
		nullptr,                     // Adapter: nullptr means the default adapter will be used.
		D3D_DRIVER_TYPE_HARDWARE,    // DriverType: Use the GPU hardware for rendering.
		nullptr,                     // Software: Not used because hardware rendering is specified.
		0,                           // Flags: No special flags for device creation.
		nullptr,                     // FeatureLevels: Use the default set of feature levels.
		0,                           // FeatureLevelsCount: Number of elements in the FeatureLevels array (0 here).
		D3D11_SDK_VERSION,           // SDKVersion: Specifies the Direct3D 11 version.
		&sd,                         // SwapChainDesc: Address of the DXGI_SWAP_CHAIN_DESC structure.
		&pSwap,                      // SwapChain: Address to receive the created swap chain.
		&pDevice,                    // Device: Address to receive the created Direct3D device.
		nullptr,                     // FeatureLevel: Address to receive the actual feature level used (not needed here).
		&pContext                    // ImmediateContext: Address to receive the created device context.
	);
}

Graphics::~Graphics()
{
	if(pContext != nullptr)
	{
		pContext->Release();
	}

	if (pSwap != nullptr)
	{
		pSwap->Release();
	}

	if(pDevice != nullptr)
	{
		pDevice->Release();
	}
}

void Graphics::EndFrame()
{
	pSwap->Present(1u, 0u);
}
