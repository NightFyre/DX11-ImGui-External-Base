#ifndef PCH_H
#define PCH_H
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// STANDARD LIBRARIES
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <conio.h>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <dwmapi.h>
#include <functional>


// DIRECTX
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

//	GAMEPAD
#include <XInput.h>
#pragma comment(lib, "XInput.lib")

// DEAR IMGUI
#define IMGUI_DEFINE_MATH_OPERATORS 1
#include "../libs/ImGui/imgui.h"
#include "../libs/ImGui/imgui_internal.h"
#include "../libs/ImGui/imgui_Impl_dx11.h"
#include "../libs/ImGui/imgui_Impl_Win32.h"

using namespace std::chrono_literals;

namespace Utility
{
	struct Vector2
	{
		float x, y;
	};

	struct Vector3
	{
		float x, y, z;
	};

	struct Vector4
	{
		float x, y, z, w;
	};
}

#endif //PCH_H