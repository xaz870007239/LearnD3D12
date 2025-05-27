#include <windows.h>
#include "BattleFireDirect.h"
#include "Scene.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"winmm.lib")

LPCTSTR gWindowClassName = L"BattleFire";

LRESULT CALLBACK WindowProc(HWND inHWND, UINT inMSG, WPARAM inWParam, LPARAM inLParam) {
	switch (inMSG) {
	case WM_CLOSE:
		PostQuitMessage(0);//enqueue WM_QUIT
		break;
	}
	return DefWindowProc(inHWND, inMSG, inWParam, inLParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int inShowCmd) {
	//register
	WNDCLASSEX wndClassEx;
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.cbClsExtra = NULL;//class
	wndClassEx.cbWndExtra = NULL;//instance
	wndClassEx.hInstance = hInstance;
	wndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = NULL;
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = gWindowClassName;
	wndClassEx.lpfnWndProc = WindowProc;
	if (!RegisterClassEx(&wndClassEx)) {
		MessageBox(NULL, L"Register Class Failed!", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	//create
	int viewportWidth = 1280;
	int viewportHeight = 720;
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = viewportWidth;
	rect.bottom = viewportHeight;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;
	HWND hwnd = CreateWindowEx(NULL,
		gWindowClassName,
		L"My Render Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidth, windowHeight,
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!hwnd) {
		MessageBox(NULL, L"Create Window Failed!", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	//show
	InitD3D12(hwnd, 1280, 720);
	InitScene(1280, 720);
	ShowWindow(hwnd, inShowCmd);
	UpdateWindow(hwnd);
	
	MSG msg;
	DWORD last_time = timeGetTime();
	DWORD appStartTime = last_time;
	while (true){
		ZeroMemory(&msg, sizeof(MSG));
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			//rendering
			WaitForCompletionOfCommandList();
			DWORD current_time = timeGetTime();//ms
			DWORD frameTime = current_time - last_time;
			DWORD timeSinceAppStartInMS = current_time - appStartTime;
			last_time = current_time;
			float frameTimeInSecond = float(frameTime) / 1000.0f;//second
			float timeSinceAppStartInSecond = float(timeSinceAppStartInMS) / 1000.0f;
			RenderOneFrame(frameTimeInSecond, timeSinceAppStartInSecond);
			SwapD3D12Buffers();
		}
	}
	return 0;
}