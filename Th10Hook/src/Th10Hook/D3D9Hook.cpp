#include "Th10Hook/Common.h"
#include "Th10Hook/D3D9Hook.h"

#include <Base/ScopeGuard.h>

#include "Th10Hook/OldDxResult.h"
#include "Th10Hook/MyDetours.h"

namespace th
{
	D3D9Hook::D3D9Hook() :
		Singleton(this),
		m_enabled(false),
		m_present(nullptr),
		m_isPresentReadied(false)
	{
		WNDCLASSEXW wc = {};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &DefWindowProcW;
		wc.hInstance = GetModuleHandleW(nullptr);
		wc.lpszClassName = L"D3D9HookClass";
		if (RegisterClassExW(&wc) == 0)
			BASE_THROW(WindowsError());
		ON_SCOPE_EXIT([&]()
			{
				UnregisterClassW(wc.lpszClassName, wc.hInstance);
			});

		HWND_ptr window = HWND_ptr(CreateWindowExW(0, wc.lpszClassName, L"D3D9HookWindow",
			WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, nullptr, nullptr, wc.hInstance, nullptr),
			&DestroyWindow);
		if (window == nullptr)
			BASE_THROW(WindowsError());

		HMODULE d3d9Dll = GetModuleHandleW(L"d3d9.dll");
		if (d3d9Dll == nullptr)
			BASE_THROW(WindowsError());
		Direct3DCreate9_t* direct3DCreate9 = reinterpret_cast<Direct3DCreate9_t*>(
			GetProcAddress(d3d9Dll, "Direct3DCreate9"));
		if (direct3DCreate9 == nullptr)
			BASE_THROW(WindowsError());

		CComPtr<IDirect3D9> d3d9;
		d3d9.p = direct3DCreate9(D3D_SDK_VERSION);
		if (d3d9 == nullptr)
			BASE_THROW(Exception("Direct3DCreate9() failed."));

		D3DDISPLAYMODE d3ddm = {};
		HRESULT hr = d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
		if (FAILED(hr))
			BASE_THROW(OldDxResult(hr));

		D3DPRESENT_PARAMETERS d3dpp = {};
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = d3ddm.Format;

		CComPtr<IDirect3DDevice9> device;
		hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window.get(),
			D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
			&d3dpp, &device);
		if (FAILED(hr))
			BASE_THROW(OldDxResult(hr));

		uint_t* vTable = reinterpret_cast<uint_t*>(*reinterpret_cast<uint_t*>(device.p));
		m_present = reinterpret_cast<Present_t*>(vTable[17]);

		MyDetours detours;
		detours.attach(reinterpret_cast<PVOID*>(&m_present), &D3D9Hook::PresentHook);
	}

	D3D9Hook::~D3D9Hook()
	{
		MyDetours detours;
		detours.detach(reinterpret_cast<PVOID*>(&m_present), &D3D9Hook::PresentHook);
	}

	void D3D9Hook::enable(bool enabled)
	{
		m_enabled = enabled;
	}

	HRESULT D3D9Hook::PresentHook(IDirect3DDevice9* device, CONST RECT* sourceRect, CONST RECT* destRect,
		HWND destWindowOverride, CONST RGNDATA* dirtyRegion)
	{
		D3D9Hook& d3d9Hook = D3D9Hook::GetInstance();
		return d3d9Hook.presentHook(device, sourceRect, destRect, destWindowOverride, dirtyRegion);
	}

	std::chrono::steady_clock::time_point g_presentBeginTime;
	std::chrono::steady_clock::time_point g_presentEndTime;

	HRESULT D3D9Hook::presentHook(IDirect3DDevice9* device, CONST RECT* sourceRect, CONST RECT* destRect,
		HWND destWindowOverride, CONST RGNDATA* dirtyRegion)
	{
		if (!m_enabled)
			return m_present(device, sourceRect, destRect, destWindowOverride, dirtyRegion);

		g_presentBeginTime = std::chrono::steady_clock::now();

		notifyPresent();
		HRESULT hr = m_present(device, sourceRect, destRect, destWindowOverride, dirtyRegion);

		g_presentEndTime = std::chrono::steady_clock::now();

		return hr;
	}

	void D3D9Hook::notifyPresent()
	{
		unique_lock<mutex> lock(m_presentMutex);
		m_isPresentReadied = true;
		m_presentCond.notify_one();
	}

	bool D3D9Hook::waitPresent()
	{
		bool waited = false;
		unique_lock<mutex> lock(m_presentMutex);
		while (!m_isPresentReadied)
		{
			m_presentCond.wait(lock);
			waited = true;
		}
		m_isPresentReadied = false;
		return waited;
	}
}