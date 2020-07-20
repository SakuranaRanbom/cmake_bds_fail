// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

//----------------------------------
// 引用头文件
//----------------------------------
#define WIN32_LEAN_AND_MEAN
// Windows 头文件
#include <windows.h>
// C++标准库 头文件
#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <map>
#include <set>
#include <utility>
// 微软 Detours 库 头文件
#include "./Detours/include/detours.h"

//----------------------------------
// 基本类型定义
//----------------------------------
using VA = unsigned __int64;
using RVA = unsigned int;

template<typename Type>
using Ptr = Type*;

//----------------------------------
// Hook 机制
//----------------------------------
enum class HookErrorCode {
	ERR_SUCCESS,
	ERR_TRANSACTION_BEGIN,
	ERR_UPDATE_THREAD,
	ERR_ATTACH,
	ERR_DETACH,
	ERR_TRANSACTION_COMMIT
};
template<typename T = Ptr<void>>
auto Hook(Ptr<T> p, T f) {
	int error = DetourTransactionBegin();
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_TRANSACTION_BEGIN;
	}
	error = DetourUpdateThread(GetCurrentThread());
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_UPDATE_THREAD;
	}
	error = DetourAttach(
		reinterpret_cast<Ptr<PVOID>>(p),
		reinterpret_cast<PVOID>(f)
	);
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_ATTACH;
	}
	error = DetourTransactionCommit();
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_TRANSACTION_COMMIT;
	}
	return HookErrorCode::ERR_SUCCESS;
}
template<typename T = Ptr<void>>
auto UnHook(Ptr<T> p, T f) {
	int error = DetourTransactionBegin();
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_TRANSACTION_BEGIN;
	}
	error = DetourUpdateThread(GetCurrentThread());
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_UPDATE_THREAD;
	}
	error = DetourDetach(
		reinterpret_cast<Ptr<PVOID>>(p),
		reinterpret_cast<PVOID>(f)
	);
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_DETACH;
	}
	error = DetourTransactionCommit();
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_TRANSACTION_COMMIT;
	}
	return HookErrorCode::ERR_SUCCESS;
}

//----------------------------------
// THook 模板
//----------------------------------
template<typename T_VA, typename T_RVA>
struct RegisterStaticHook {
	RegisterStaticHook(T_RVA sym, void* hook, void** org) {
		auto base = reinterpret_cast<T_VA>(GetModuleHandle(NULL));
		*org = reinterpret_cast<void*>(base + sym);
		auto ret = Hook<void*>(org, hook);
		if (ret != HookErrorCode::ERR_SUCCESS) {
			std::cout << "[Error] ";
			switch (ret) {
			case HookErrorCode::ERR_TRANSACTION_BEGIN:
				std::cout << "DetourTransactionBegin";
				break;
			case HookErrorCode::ERR_UPDATE_THREAD:
				std::cout << "DetourUpdateThread";
				break;
			case HookErrorCode::ERR_ATTACH:
				std::cout << "DetourAttach";
				break;
			case HookErrorCode::ERR_DETACH:
				std::cout << "DetourDetach";
				break;
			case HookErrorCode::ERR_TRANSACTION_COMMIT:
				std::cout << "DetourTransactionCommit";
				break;
			}
			std::cout << "failed!" << std::endl;
		}
	}
	// workaround for a warning
	template <typename T>
	RegisterStaticHook(T_RVA sym, T hook, void** org) {
		union {
			T a;
			void* b;
		} hookUnion;
		hookUnion.a = hook;
		RegisterStaticHook(sym, hookUnion.b, org);
	}
};

//----------------------------------
// 引用符号定位文件
//----------------------------------

//----------------------------------

#endif //PCH_H

