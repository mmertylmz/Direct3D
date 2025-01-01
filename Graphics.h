#pragma once
#include "ChiliWin.h"
#include "ChiliException.h"
#include "d3d11.h"



class Graphics
{
public:
	class Exception : public ChiliException
	{
		using ChiliException::ChiliException;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	};


public:
	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator = (const Graphics&) = delete;
	~Graphics();
	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;
private:
	ID3D11Device* pDevice = nullptr; // Represents the Direct3D device used to manage GPU resources.
	IDXGISwapChain* pSwap = nullptr; // Manages the swap chain for presenting frames to the screen.
	ID3D11DeviceContext* pContext = nullptr; // Executes rendering commands on the GPU.
	ID3D11RenderTargetView* pTarget = nullptr; // Represents the render target (back buffer) where the GPU draws.

};

