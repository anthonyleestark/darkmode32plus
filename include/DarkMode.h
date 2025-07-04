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

extern bool g_darkModeSupported;
extern bool g_darkModeEnabled;


[[nodiscard]] bool ShouldAppsUseDarkMode();
bool AllowDarkModeForWindow(HWND hWnd, bool allow);
[[nodiscard]] bool IsHighContrast();
#if defined(_DARKMODELIB_ALLOW_OLD_OS)
void RefreshTitleBarThemeColor(HWND hWnd);
void SetTitleBarThemeColor(HWND hWnd, BOOL dark);
#endif
[[nodiscard]] bool IsColorSchemeChangeMessage(LPARAM lParam);
[[nodiscard]] bool IsColorSchemeChangeMessage(UINT uMsg, LPARAM lParam);
void AllowDarkModeForApp(bool allow);
void EnableDarkScrollBarForWindowAndChildren(HWND hWnd);
void InitDarkMode();
void SetDarkMode(bool useDark, bool fixDarkScrollbar);
[[nodiscard]] bool IsWindows10();
[[nodiscard]] bool IsWindows11();
[[nodiscard]] DWORD GetWindowsBuildNumber();

void SetMySysColor(int nIndex, COLORREF clr);
bool HookSysColor();
void UnhookSysColor();
