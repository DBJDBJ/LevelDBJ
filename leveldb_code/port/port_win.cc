// LevelDB Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// See port_example.h for documentation for the following types/functions.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of the University of California, Berkeley nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "port/port_win.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <cassert>

namespace leveldb {
	namespace port {

		BOOL CALLBACK InvokeInitOnceFunction(PINIT_ONCE InitOnce, PVOID param, PVOID* lpContext)
		{
			InitOnceCallbackFunction func = (InitOnceCallbackFunction)param;
			func();
			return TRUE;
		}

		INIT_ONCE g_InitOnce = INIT_ONCE_STATIC_INIT;
		void InitOnce(OnceType* once, void(*initializer)())
		{
			InitOnceExecuteOnce(&g_InitOnce, InvokeInitOnceFunction, initializer, NULL);
		}

		std::wstring s2ws(const std::string& s)
		{
			const int slength = (int)s.size() + 1;
			int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), slength, 0, 0);
			std::vector<wchar_t> rez(len, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, s.data(), slength, rez.data(), len);
			return rez.data();
		}

		std::string ws2s(const std::wstring& s)
		{
			const int slength = (int)s.size() + 1;
			int len = WideCharToMultiByte(CP_UTF8, 0, s.data(), slength, 0, 0, 0, 0);
			std::vector<char> rez(len, '\0');
			WideCharToMultiByte(CP_UTF8, 0, s.data(), slength, rez.data(), len, 0, 0);
			return rez.data();
		}

		// DBJ
		namespace dbj {
			//  ErrorMessage support function.
	//  Retrieves the system error message for the GetLastError() code.
	//  Note: caller must use LocalFree() on the returned LPCTSTR buffer.
			inline LPCSTR ErrorMessage(DWORD error)
			{
				LPVOID lpMsgBuf;

				FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
					| FORMAT_MESSAGE_FROM_SYSTEM
					| FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					error,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPSTR)&lpMsgBuf,
					0,
					NULL);

				return((LPCSTR)lpMsgBuf);
			}

			//  PrintError support function.
			//  Simple wrapper function for error output.
			inline void PrintError(const char* errDesc)
			{
				LPCSTR errMsg = ErrorMessage(GetLastError());
				printf("\n** ERROR ** %s: %s\n", errDesc, errMsg);
				LocalFree((LPVOID)errMsg);
			}
		} // dbj

		std::string os_env_temp_path()
		{
			CHAR lpTempPathBuffer[MAX_PATH]{ 0 };
			//  Gets the temp path env string (no guarantee it's a valid path).
			DWORD dwRetVal = GetTempPathA(MAX_PATH, lpTempPathBuffer);
			if (dwRetVal > MAX_PATH || (dwRetVal == 0))
			{
				dbj::PrintError("GetTempPath failed");
				exit(0);
			}

			return lpTempPathBuffer;
		}

		Mutex::Mutex() :
			cs_(nullptr) {
			assert(!cs_);
			cs_ = static_cast<void*>(new CRITICAL_SECTION());
			::InitializeCriticalSectionEx(static_cast<CRITICAL_SECTION*>(cs_), 1, 0);
			assert(cs_);
		}

		Mutex::~Mutex() {
			assert(cs_);
			::DeleteCriticalSection(static_cast<CRITICAL_SECTION*>(cs_));
			delete static_cast<CRITICAL_SECTION*>(cs_);
			cs_ = nullptr;
			assert(!cs_);
		}

		void Mutex::Lock() {
			assert(cs_);
			::EnterCriticalSection(static_cast<CRITICAL_SECTION*>(cs_));
		}

		void Mutex::Unlock() {
			assert(cs_);
			::LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(cs_));
		}

		void Mutex::AssertHeld() {
			assert(cs_);
			assert(1);
		}

		CondVar::CondVar(Mutex* mu) :
			waiting_(0),
			mu_(mu),
			sem1_(::CreateSemaphoreEx(NULL, 0, 10000, NULL, 0, 0)),
			sem2_(::CreateSemaphoreEx(NULL, 0, 10000, NULL, 0, 0)) {
			assert(mu_);
		}

		CondVar::~CondVar() {
			::CloseHandle(sem1_);
			::CloseHandle(sem2_);
		}

		void CondVar::Wait() {
			mu_->AssertHeld();

			wait_mtx_.Lock();
			++waiting_;
			wait_mtx_.Unlock();

			mu_->Unlock();

			// initiate handshake
			::WaitForSingleObjectEx(sem1_, INFINITE, FALSE);
			::ReleaseSemaphore(sem2_, 1, NULL);
			mu_->Lock();
		}

		void CondVar::Signal() {
			wait_mtx_.Lock();
			if (waiting_ > 0) {
				--waiting_;

				// finalize handshake
				::ReleaseSemaphore(sem1_, 1, NULL);
				::WaitForSingleObjectEx(sem2_, INFINITE, FALSE);
			}
			wait_mtx_.Unlock();
		}

		void CondVar::SignalAll() {
			wait_mtx_.Lock();
			for (long i = 0; i < waiting_; ++i) {
				::ReleaseSemaphore(sem1_, 1, NULL);
				while (waiting_ > 0) {
					--waiting_;
					::WaitForSingleObjectEx(sem2_, INFINITE, FALSE);
				}
			}
			wait_mtx_.Unlock();
		}

		AtomicPointer::AtomicPointer(void* v) {
			Release_Store(v);
		}

		void* AtomicPointer::Acquire_Load() const {
			void* p = nullptr;
			InterlockedExchangePointer(&p, rep_);
			return p;
		}

		void AtomicPointer::Release_Store(void* v) {
			InterlockedExchangePointer(&rep_, v);
		}

		void* AtomicPointer::NoBarrier_Load() const {
			return rep_;
		}

		void AtomicPointer::NoBarrier_Store(void* v) {
			rep_ = v;
		}
	}
}
