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


#pragma once
#include <windows.h>
#include <uxtheme.h>
#include <vsstyle.h>

#define _DARKMODE_SUPPORT_OLDER_OS

// DarkMode helper namespace
namespace DarkModeHelper
{
	enum IMMERSIVE_HC_CACHE_MODE
	{
		IHCM_USE_CACHED_VALUE,
		IHCM_REFRESH
	};

	// Windows 10 version 1903 build number 18362
	enum class PreferredAppMode
	{
		Default,
		AllowDark,
		ForceDark,
		ForceLight,
		Max
	};

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
	enum WINDOWCOMPOSITIONATTRIB
	{
		WCA_UNDEFINED = 0,
		WCA_NCRENDERING_ENABLED = 1,
		WCA_NCRENDERING_POLICY = 2,
		WCA_TRANSITIONS_FORCEDISABLED = 3,
		WCA_ALLOW_NCPAINT = 4,
		WCA_CAPTION_BUTTON_BOUNDS = 5,
		WCA_NONCLIENT_RTL_LAYOUT = 6,
		WCA_FORCE_ICONIC_REPRESENTATION = 7,
		WCA_EXTENDED_FRAME_BOUNDS = 8,
		WCA_HAS_ICONIC_BITMAP = 9,
		WCA_THEME_ATTRIBUTES = 10,
		WCA_NCRENDERING_EXILED = 11,
		WCA_NCADORNMENTINFO = 12,
		WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
		WCA_VIDEO_OVERLAY_ACTIVE = 14,
		WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
		WCA_DISALLOW_PEEK = 16,
		WCA_CLOAK = 17,
		WCA_CLOAKED = 18,
		WCA_ACCENT_POLICY = 19,
		WCA_FREEZE_REPRESENTATION = 20,
		WCA_EVER_UNCLOAKED = 21,
		WCA_VISUAL_OWNER = 22,
		WCA_HOLOGRAPHIC = 23,
		WCA_EXCLUDED_FROM_DDA = 24,
		WCA_PASSIVEUPDATEMODE = 25,
		WCA_USEDARKMODECOLORS = 26,
		WCA_LAST = 27
	};

	struct WINDOWCOMPOSITIONATTRIBDATA
	{
		WINDOWCOMPOSITIONATTRIB Attrib;
		PVOID pvData;
		SIZE_T cbData;
	};
#endif

// Function typename declarations
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
	using fnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA*);
#endif
	// Windows 10 version 1809 build number 17763
	using fnShouldAppsUseDarkMode = auto (WINAPI*)() -> bool; // ordinal 132
	using fnAllowDarkModeForWindow = auto (WINAPI*)(HWND hWnd, bool allow) -> bool; // ordinal 133
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
	using fnAllowDarkModeForApp = auto (WINAPI*)(bool allow) -> bool; // ordinal 135, in 1809
#endif
	using fnFlushMenuThemes = void (WINAPI*)(); // ordinal 136
	using fnRefreshImmersiveColorPolicyState = void (WINAPI*)(); // ordinal 104
	using fnIsDarkModeAllowedForWindow = auto (WINAPI*)(HWND hWnd) -> bool; // ordinal 137
	using fnGetIsImmersiveColorUsingHighContrast = auto (WINAPI*)(IMMERSIVE_HC_CACHE_MODE mode) -> bool; // ordinal 106
	using fnOpenNcThemeData = auto (WINAPI*)(HWND hWnd, LPCWSTR pszClassList) -> HTHEME; // ordinal 49
	// Windows 10 version 1903 build number 18362
	using fnShouldSystemUseDarkMode = auto (WINAPI*)() -> bool; // ordinal 138
	using fnSetPreferredAppMode = auto (WINAPI*)(PreferredAppMode appMode) -> PreferredAppMode; // ordinal 135, in 1903
	using fnIsDarkModeAllowedForApp = auto (WINAPI*)() -> bool; // ordinal 139

// Global variables
	extern bool g_darkModeSupported;
	extern bool g_darkModeEnabled;

// DarkMode helpers
	[[nodiscard]] bool ShouldAppsUseDarkMode(void) noexcept;
	bool AllowDarkModeForWindow(HWND hWnd, bool isAllowed) noexcept;
	[[nodiscard]] bool IsHighContrast(void);

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
	void RefreshTitleBarThemeColor(HWND hWnd);
	void SetTitleBarThemeColor(HWND hWnd, BOOL isDark);
#endif

	[[nodiscard]] bool IsColorSchemeChangeMessage(LPARAM lParam);
	[[nodiscard]] bool IsColorSchemeChangeMessage(UINT message, LPARAM lParam);

	void AllowDarkModeForApp(bool isAllowed) noexcept;
	void EnableDarkScrollBarForWindowAndChildren(HWND hWnd);

	void InitDarkMode(void);
	void SetDarkMode(bool useDark, bool doFixDarkScrollbar);
};
