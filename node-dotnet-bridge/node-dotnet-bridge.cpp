#include <nan.h>
#include <string>

#if WINDOWS
#include <Windows.h>
#elif LINUX
#include <dirent.h>
#include <dlfcn.h>
#include <limits.h>
#define FS_SEPARATOR "/"
#define PATH_DELIMITER ":"
#define MAX_PATH PATH_MAX
#endif

// https://github.com/dotnet/coreclr/blob/master/src/coreclr/hosts/inc/coreclrhost.h
#include "coreclrhost.h"

using namespace std;

using namespace Nan;
using namespace v8;

v8::Persistent<Function> loggingCallbackPersist;

void log(Isolate* isolate, v8::Local<String> str) {
    Handle<Value> argv[] = { str };
    Local<Function>::New(isolate, loggingCallbackPersist)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
}

void log(Isolate* isolate, const wchar_t* text) {
    auto str = String::NewFromTwoByte(isolate, (uint16_t*)text);
    log(isolate, str); 
}

NAN_METHOD(Initialize) {
    auto isolate = info.GetIsolate();

    auto settings = Handle<Object>::Cast(info[0]);
    auto loggingValue = settings->Get(New<String>("logCallback").ToLocalChecked());

    auto callback = Local<Function>::Cast(loggingValue);
    loggingCallbackPersist.Reset(isolate, callback);

#if WINDOWS
    auto clrBasePath = "C:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\";
    auto dllName = "coreclr.dll";
#else
    auto clrBasePath = "/usr/share/dotnet/shared/Microsoft.NETCore.App/";
    auto dllName = "libcoreclr.so";
#endif
    auto resolveCoreclr = Local<Function>::Cast(settings->Get(New<String>("resolveCoreclr").ToLocalChecked()));
    auto var = New<String>(clrBasePath).ToLocalChecked();
    auto var2 = New<String>(dllName).ToLocalChecked();
    Handle<Value> argv[] = { var, var2 };
    auto res = resolveCoreclr->Call(isolate->GetCurrentContext()->Global(), 2, argv);
    v8::String::Utf8Value s(res);
    const char* coreclrPath = *s;

#if WINDOWS
 	auto coreClr = LoadLibraryExA(coreclrPath, nullptr, 0);
#elif LINUX
	void* coreClr = dlopen(coreclrPath, RTLD_NOW | RTLD_LOCAL);
#endif
	if (!coreClr) {
		log(isolate, L"ERROR: Failed to load CoreCLR from");
        return;
    }

#if WINDOWS
	coreclr_initialize_ptr initializeCoreClr = (coreclr_initialize_ptr)GetProcAddress(coreClr, "coreclr_initialize");
	coreclr_create_delegate_ptr createManagedDelegate = (coreclr_create_delegate_ptr)GetProcAddress(coreClr, "coreclr_create_delegate");
	coreclr_shutdown_ptr shutdownCoreClr = (coreclr_shutdown_ptr)GetProcAddress(coreClr, "coreclr_shutdown");
#elif LINUX
	coreclr_initialize_ptr initializeCoreClr = (coreclr_initialize_ptr)dlsym(coreClr, "coreclr_initialize");
	coreclr_create_delegate_ptr createManagedDelegate = (coreclr_create_delegate_ptr)dlsym(coreClr, "coreclr_create_delegate");
	coreclr_shutdown_ptr shutdownCoreClr = (coreclr_shutdown_ptr)dlsym(coreClr, "coreclr_shutdown");
#endif

	if (!initializeCoreClr) {
		log(isolate, L"coreclr_initialize not found");
		return;
	}

	if (!createManagedDelegate) {
		log(isolate, L"coreclr_create_delegate not found");
		return;
	}

	if (!shutdownCoreClr) {
		log(isolate, L"coreclr_shutdown not found");
		return;
	}        

    log(isolate, L"Initialization finished");
}

NAN_METHOD(UnInitialize) {
    loggingCallbackPersist.Reset();
}

NAN_MODULE_INIT(init) {
    Nan::Set(target, New<String>("initialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(Initialize)).ToLocalChecked());
    Nan::Set(target, New<String>("unInitialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(UnInitialize)).ToLocalChecked());
}

NODE_MODULE(node_dotnet, init)