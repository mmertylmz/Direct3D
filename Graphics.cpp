#include "Graphics.h"
#include "dxerr.h"
#include <sstream>

namespace wrl = Microsoft::WRL;

#pragma comment(lib, "d3d11.lib") //prevent linker error

// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )
#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#else
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#endif

Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
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

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//for checking results of d3d functions
	HRESULT hr;

	//create device and front/back buffers, and swap chain and rendering context
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr,                     // Adapter: nullptr means the default adapter will be used.
		D3D_DRIVER_TYPE_HARDWARE,    // DriverType: Use the GPU hardware for rendering.
		nullptr,                     // Software: Not used because hardware rendering is specified.
		swapCreateFlags,             // Flags: No special flags for device creation.
		nullptr,                     // FeatureLevels: Use the default set of feature levels.
		0,                           // FeatureLevelsCount: Number of elements in the FeatureLevels array (0 here).
		D3D11_SDK_VERSION,           // SDKVersion: Specifies the Direct3D 11 version.
		&sd,                         // SwapChainDesc: Address of the DXGI_SWAP_CHAIN_DESC structure.
		&pSwap,                      // SwapChain: Address to receive the created swap chain.
		&pDevice,                    // Device: Address to receive the created Direct3D device.
		nullptr,                     // FeatureLevel: Address to receive the actual feature level used (not needed here).
		&pContext                    // ImmediateContext: Address to receive the created device context.
	));

	//gain access to texture subresource in swap chain (back buffer)
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));
}

void Graphics::EndFrame()
{
	HRESULT hr;

#ifndef NDEBUG
	infoManager.Set();
#endif

	if (FAILED(hr = pSwap->Present(1u, 0u)))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			throw GFX_EXCEPT(hr);
		}
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	// The fourth value (1.0f) represents the alpha channel (fully opaque).
	const float color[] = { red, green, blue, 1.0f };

	// Clears the render target (the area where rendering is performed) with the specified color.
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

void Graphics::DrawTestTriangle()
{
	namespace wrl = Microsoft::WRL;
	HRESULT hr;

	struct Vertex
	{
		float x;
		float y;
	};

	//create vertex buffer (1 2d triangle at center of screen)
	const Vertex vertices[] =
	{
		{ 0.0f, 0.5f },
		{ 0.5f, -0.5f },
		{ -0.5f, -0.5f }
	};

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;
	GFX_THROW_INFO(pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));

	//Bind vertex buffer to pipeline
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, &pVertexBuffer, &stride, &offset);
	pContext->Draw((UINT)std::size(vertices), 0u);

}

// Graphics exception stuff
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}
const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Directx Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription(hr, buf, sizeof(buf));
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}

const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Directx Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
