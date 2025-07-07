// SPDX-License-Identifier: BSD-3-Clause license

/*
 * Copyright (c) 2025 Anthony Lee Stark. All rights reserved.
 *
 * This project is based on and includes modified code from:
 * project 'win32-darkmode' by ysc3839 (MIT License),
 * available at: https://github.com/ysc3839/win32-darkmode
 * and project 'darkmodelib' by ozone10 (MPL-2.0 License),
 * available at: https://github.com/ozone10/darkmodelib
 *
 * The respective original licenses apply to portions of this code.
 * See the `licenses/` folder for more information.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *	  list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *	  this list of conditions and the following disclaimer in the documentation
 *	  and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Anthony Lee Stark (@anthonyleestark) nor the names of
 *	  its contributors may be used to endorse or promote products derived from
 *	  this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif


#include "DarkMode.h"
using namespace DarkModeHelper;


#include "WinVerHelper.h"

// Supported Windows version
static unsigned long g_buildNumber = 0;
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
static constexpr unsigned short MinSupportVersion = WinVerHelper::WinVer::WIN10_VER_1809;
#else
static constexpr unsigned short MinSupportVersion = WinVerHelper::WinVer::WIN10_VER_22H2;
#endif


// For module management
#include "ModuleHelper.h"
using namespace ModuleHelper;

#include <mutex>
#include <unordered_set>


// Dark mode external IatHook
#if !defined(_DARKMODE_EXTERNAL_IATHOOK)
	#include "IatHook.h"
#else
	extern PIMAGE_THUNK_DATA FindAddressByName(void* moduleBase, PIMAGE_THUNK_DATA impName, PIMAGE_THUNK_DATA impAddr, const char* funcName);
	extern PIMAGE_THUNK_DATA FindAddressByOrdinal(void* moduleBase, PIMAGE_THUNK_DATA impName, PIMAGE_THUNK_DATA impAddr, uint16_t ordinal);
	extern PIMAGE_THUNK_DATA FindIatThunkInModule(void* moduleBase, const char* dllName, const char* funcName);
	extern PIMAGE_THUNK_DATA FindDelayLoadThunkInModule(void* moduleBase, const char* dllName, const char* funcName);
	extern PIMAGE_THUNK_DATA FindDelayLoadThunkInModule(void* moduleBase, const char* dllName, uint16_t ordinal);
#endif


#if defined(_MSC_VER) && _MSC_VER >= 1800
	#pragma warning(disable : 4191)
#elif defined(__GNUC__)
	#include <cwchar>
#endif


// Function pointers
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
static fnSetWindowCompositionAttribute pfSetWindowCompositionAttribute = nullptr;
#endif
static fnShouldAppsUseDarkMode pfShouldAppsUseDarkMode = nullptr;
static fnAllowDarkModeForWindow pfAllowDarkModeForWindow = nullptr;
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
static fnAllowDarkModeForApp _AllowDarkModeForApp = nullptr;
#endif
static fnFlushMenuThemes pfFlushMenuThemes = nullptr;
static fnRefreshImmersiveColorPolicyState pfRefreshImmersiveColorPolicyState = nullptr;
static fnIsDarkModeAllowedForWindow pfIsDarkModeAllowedForWindow = nullptr;
static fnGetIsImmersiveColorUsingHighContrast pfGetIsImmersiveColorUsingHighContrast = nullptr;
static fnOpenNcThemeData pfOpenNcThemeData = nullptr;
// 1903 18362
//static fnShouldSystemUseDarkMode _ShouldSystemUseDarkMode = nullptr;
static fnSetPreferredAppMode pfSetPreferredAppMode = nullptr;


// Global variables initialization
bool DarkModeHelper::g_darkModeSupported = false;
bool DarkModeHelper::g_darkModeEnabled = false;


// Should application use Dark Mode or not
bool DarkModeHelper::ShouldAppsUseDarkMode() noexcept
{
	if (pfShouldAppsUseDarkMode)
		return pfShouldAppsUseDarkMode();

	return false;
}

// Allow dark mode for the application or not
void DarkModeHelper::AllowDarkModeForApp(bool allow) noexcept
{
	if (pfSetPreferredAppMode != nullptr)
		pfSetPreferredAppMode(allow ? PreferredAppMode::ForceDark : PreferredAppMode::Default);

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
	else if (_AllowDarkModeForApp != nullptr)
		_AllowDarkModeForApp(allow);
#endif
}

// Allow Dark mode for a specific window or not
bool DarkModeHelper::AllowDarkModeForWindow(HWND hWnd, bool allow) noexcept
{
	if (g_darkModeSupported && (pfAllowDarkModeForWindow != nullptr))
		return pfAllowDarkModeForWindow(hWnd, allow);

	return false;
}

// Is system currently in high contrast mode
bool DarkModeHelper::IsHighContrast()
{
	HIGHCONTRASTW highContrast{};
	highContrast.cbSize = sizeof(HIGHCONTRASTW);
	if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRASTW), &highContrast, FALSE) == TRUE)
		return (highContrast.dwFlags & HCF_HIGHCONTRASTON) == HCF_HIGHCONTRASTON;

	return false;
}

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
// Set title bar theme color
void DarkModeHelper::SetTitleBarThemeColor(HWND hWnd, BOOL dark)
{
	using namespace WinVerHelper;
	if (g_buildNumber < WinVer::WIN10_VER_1903)
		SetPropW(hWnd, L"UseImmersiveDarkModeColors", reinterpret_cast<HANDLE>(static_cast<intptr_t>(dark)));
	else if (pfSetWindowCompositionAttribute != nullptr) {
		WINDOWCOMPOSITIONATTRIBDATA data{ WCA_USEDARKMODECOLORS, &dark, sizeof(dark) };
		pfSetWindowCompositionAttribute(hWnd, &data);
	}
}

// Refresh title bar theme color
void DarkModeHelper::RefreshTitleBarThemeColor(HWND hWnd)
{
	BOOL dark = FALSE;
	if (pfIsDarkModeAllowedForWindow != nullptr && pfShouldAppsUseDarkMode != nullptr) {
		if (pfIsDarkModeAllowedForWindow(hWnd) && pfShouldAppsUseDarkMode() && !IsHighContrast())
			dark = TRUE;
	}

	SetTitleBarThemeColor(hWnd, dark);
}
#endif

bool DarkModeHelper::IsColorSchemeChangeMessage(LPARAM lParam)
{
	bool result = false;
	if ((lParam != 0) // NULL
		&& (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)
		&& pfRefreshImmersiveColorPolicyState != nullptr) {
		pfRefreshImmersiveColorPolicyState();
		result = true;
	}

	if (pfGetIsImmersiveColorUsingHighContrast)
		pfGetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);

	return result;
}

bool DarkModeHelper::IsColorSchemeChangeMessage(UINT message, LPARAM lParam)
{
	if (message == WM_SETTINGCHANGE)
		return IsColorSchemeChangeMessage(lParam);

	return false;
}

static void FlushMenuThemes() noexcept
{
	if (pfFlushMenuThemes)
		pfFlushMenuThemes();
}

// limit dark scroll bar to specific windows and their children

static std::unordered_set<HWND> g_darkScrollBarWindows;
static std::mutex g_darkScrollBarMutex;

// Enable dark scroll bars for a specific window and its children
void DarkModeHelper::EnableDarkScrollBarForWindowAndChildren(HWND hWnd)
{
	const std::lock_guard<std::mutex> lock(g_darkScrollBarMutex);
	g_darkScrollBarWindows.insert(hWnd);
}

// Is a specific window or its parent using dark scroll bars or not
static bool IsWindowOrParentUsingDarkScrollBar(HWND hWnd)
{
	HWND hRoot = GetAncestor(hWnd, GA_ROOT);

	const std::lock_guard<std::mutex> lock(g_darkScrollBarMutex);
	auto hasElement = [](const auto& container, HWND hWndToCheck) -> bool {
#if (defined(_MSC_VER) && (_MSVC_LANG >= 202002L)) || (__cplusplus >= 202002L)
		return container.contains(hWndToCheck);
#else
		return container.count(hWndToCheck) != 0;
#endif
	};

	if (hasElement(g_darkScrollBarWindows, hWnd))
		return true;

	return (hWnd != hRoot && hasElement(g_darkScrollBarWindows, hRoot));
}

static HTHEME WINAPI MyOpenNcThemeData(HWND hWnd, LPCWSTR pszClassList)
{
	if (std::wcscmp(pszClassList, WC_SCROLLBAR) == 0) {
		if (IsWindowOrParentUsingDarkScrollBar(hWnd)) {
			hWnd = nullptr;
			pszClassList = L"Explorer::ScrollBar";
		}
	}

	return pfOpenNcThemeData(hWnd, pszClassList);
}

// Fix dark scroll bar
static void FixDarkScrollBar()
{
	const ModuleHandle moduleComctl(L"comctl32.dll");
	if (moduleComctl.IsLoaded())
	{
		auto* addr = FindDelayLoadThunkInModule(moduleComctl.Get(), "uxtheme.dll", 49); // OpenNcThemeData
		if (addr != nullptr) // && pfOpenNcThemeData != nullptr) // checked in InitDarkMode
			ReplaceFunction<fnOpenNcThemeData>(addr, MyOpenNcThemeData);
	}
}

// Initialize Dark Mode
void DarkModeHelper::InitDarkMode()
{
	static bool isInit = false;
	if (isInit)
		return;
	using namespace WinVerHelper;
	fnRtlGetNtVersionNumbers RtlGetNtVersionNumbers = nullptr;
	HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
	if (hNtdll && LoadFunc(hNtdll, RtlGetNtVersionNumbers, "RtlGetNtVersionNumbers") && RtlGetNtVersionNumbers)
	{
		DWORD major = 0;
		DWORD minor = 0;
		RtlGetNtVersionNumbers(&major, &minor, &g_buildNumber);
		g_buildNumber &= ~0xF0000000;
		if (major == 10 && minor == 0 && g_buildNumber >= MinSupportVersion)
		{
			const ModuleHandle moduleUxtheme(L"uxtheme.dll");
			if (moduleUxtheme.IsLoaded())
			{
				const HMODULE& hUxtheme = moduleUxtheme.Get();

				bool ptrFnOrd135NotNullptr = false;
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
				if (g_buildNumber < WinVer::WIN10_VER_1903)
					ptrFnOrd135NotNullptr = LoadFunc(hUxtheme, _AllowDarkModeForApp, 135);
				else
#endif
					ptrFnOrd135NotNullptr = LoadFunc(hUxtheme, pfSetPreferredAppMode, 135);

				if (ptrFnOrd135NotNullptr
					&& LoadFunc(hUxtheme, pfOpenNcThemeData, 49)
					&& LoadFunc(hUxtheme, pfRefreshImmersiveColorPolicyState, 104)
					&& LoadFunc(hUxtheme, pfShouldAppsUseDarkMode, 132)
					&& LoadFunc(hUxtheme, pfAllowDarkModeForWindow, 133)
					&& LoadFunc(hUxtheme, pfFlushMenuThemes, 136)
					&& LoadFunc(hUxtheme, pfIsDarkModeAllowedForWindow, 137))
				{
					g_darkModeSupported = true;
				}

				LoadFunc(hUxtheme, pfGetIsImmersiveColorUsingHighContrast, 106);

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
				if (g_buildNumber < WinVer::WIN10_VER_2004)
				{
					HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
					if (hUser32 != nullptr)
						LoadFunc(hUser32, pfSetWindowCompositionAttribute, "SetWindowCompositionAttribute");
				}
#endif
				isInit = true;
			}
		}
	}
}

// Set Dark Mode
void DarkModeHelper::SetDarkMode(bool useDarkMode, bool fixDarkScrollbar)
{
	if (g_darkModeSupported)
	{
		AllowDarkModeForApp(useDarkMode);
		FlushMenuThemes();
		if (fixDarkScrollbar)
			FixDarkScrollBar();

		g_darkModeEnabled = useDarkMode && ShouldAppsUseDarkMode() && !IsHighContrast();
	}
}

// Hooking GetSysColor for comboboxex' list box and list view's gridlines

using fnGetSysColor = auto (WINAPI*)(int nIndex) -> DWORD;

static fnGetSysColor pfGetSysColor = nullptr;

static COLORREF g_clrWindow = RGB(32, 32, 32);
static COLORREF g_clrText = RGB(224, 224, 224);
static COLORREF g_clrTGridlines = RGB(100, 100, 100);

static bool g_isGetSysColorHooked = false;
static int g_hookRef = 0;

// Override system color
void DarkModeHelper::SetMySysColor(int nIndex, COLORREF clr) noexcept
{
	switch (nIndex)
	{
		case COLOR_WINDOW:
			g_clrWindow = clr;
			break;

		case COLOR_WINDOWTEXT:
			g_clrText = clr;
			break;

		case COLOR_BTNFACE:
			g_clrTGridlines = clr;
			break;

		default:
			break;
	}
}

// Custom GetSysColor replacement
static DWORD WINAPI MyGetSysColor(int nIndex)
{
	if (!g_darkModeEnabled)
		return GetSysColor(nIndex);

	switch (nIndex)
	{
		case COLOR_WINDOW:
			return g_clrWindow;

		case COLOR_WINDOWTEXT:
			return g_clrText;

		case COLOR_BTNFACE:
			return g_clrTGridlines;

		default:
			return GetSysColor(nIndex);
	}
}

// Install system color hook
bool DarkModeHelper::HookSysColor()
{
	const ModuleHandle moduleComctl(L"comctl32.dll");
	if (moduleComctl.IsLoaded())
	{
		if (pfGetSysColor == nullptr || !g_isGetSysColorHooked)
		{
			auto* addr = FindIatThunkInModule(moduleComctl.Get(), "user32.dll", "GetSysColor");
			if (addr != nullptr)
			{
				pfGetSysColor = ReplaceFunction<fnGetSysColor>(addr, MyGetSysColor);
				g_isGetSysColorHooked = true;
			}
			else
				return false;
		}

		if (g_isGetSysColorHooked)
			++g_hookRef;

		return true;
	}

	return false;
}

// Uninstall system color hook
void DarkModeHelper::UnhookSysColor()
{
	const ModuleHandle moduleComctl(L"comctl32.dll");
	if (moduleComctl.IsLoaded())
	{
		if (g_isGetSysColorHooked)
		{
			if (g_hookRef > 0)
				--g_hookRef;

			if (g_hookRef == 0)
			{
				auto* addr = FindIatThunkInModule(moduleComctl.Get(), "user32.dll", "GetSysColor");
				if (addr != nullptr)
				{
					ReplaceFunction<fnGetSysColor>(addr, pfGetSysColor);
					g_isGetSysColorHooked = false;
				}
			}
		}
	}
}
