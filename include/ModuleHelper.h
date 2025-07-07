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

namespace ModuleHelper
{
	// Manage module handle
	class ModuleHandle
	{
	public:
		// Constructors
		ModuleHandle() = delete;
		explicit ModuleHandle(const wchar_t* moduleName)
			: hModule(LoadLibraryEx(moduleName, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)) {
		};

		// No copyable
		ModuleHandle(const ModuleHandle&) = delete;
		ModuleHandle& operator=(const ModuleHandle&) = delete;

		// No movable
		ModuleHandle(ModuleHandle&&) = delete;
		ModuleHandle& operator=(ModuleHandle&&) = delete;

		// Destructor
		~ModuleHandle() {
			if (hModule != nullptr)
				FreeLibrary(hModule);
		};

	public:
		[[nodiscard]] HMODULE Get() const noexcept {
			return hModule;
		};

		[[nodiscard]] bool IsLoaded() const noexcept {
			return hModule != nullptr;
		};

	private:
		HMODULE hModule = nullptr;
	};

	// Replace function pointer
	template <typename FuncPtr>
	static auto ReplaceFunction(IMAGE_THUNK_DATA* addr, const FuncPtr& newFunction) -> FuncPtr
	{
		DWORD oldProtect = 0;
		if (!VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &oldProtect))
			return nullptr;

		const uintptr_t oldFunction = addr->u1.Function;
		addr->u1.Function = reinterpret_cast<uintptr_t>(newFunction);
		VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), oldProtect, &oldProtect);
		return reinterpret_cast<FuncPtr>(oldFunction);
	};

	// Load function from module by name
	template <typename FuncPtr>
	static auto LoadFunc(HMODULE handle, FuncPtr& pointer, const char* name) -> bool
	{
		if (auto proc = ::GetProcAddress(handle, name); proc != nullptr) {
			pointer = reinterpret_cast<FuncPtr>(proc);
			return true;
		}

		return false;
	};

	// Load function from module by index
	template <typename FuncPtr>
	static auto LoadFunc(HMODULE handle, FuncPtr& pointer, WORD index) -> bool
	{
		return LoadFunc(handle, pointer, MAKEINTRESOURCEA(index));
	};
};
