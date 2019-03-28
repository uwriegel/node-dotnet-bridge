#include <nan.h>
#include <string>
#include <locale>
#include <codecvt>

#if WINDOWS
#include <Windows.h>
#elif LINUX
#include <dirent.h>
#include <dlfcn.h>
#include <limits.h>
#endif

// https://github.com/dotnet/coreclr/blob/master/src/coreclr/hosts/inc/coreclrhost.h
#include "coreclrhost.h"

using namespace std;

using namespace Nan;
using namespace v8;

// Function pointer types for the managed call and callback
typedef int (*report_callback_ptr)(int progress);
typedef char* (*doWork_ptr)(const char* jobName, int iterations, int dataSize, double* data, report_callback_ptr callbackFunction);
typedef wchar_t* (*executeSyncPtr)(const wchar_t* payload);
executeSyncPtr executeSyncDelegate;

string ws2utf8(const wstring &input) {
	wstring_convert<codecvt_utf8<wchar_t>> utf8conv;
	return utf8conv.to_bytes(input);
}

wstring utf82ws(const string &input) {
	wstring_convert<codecvt_utf8<wchar_t>> utf8conv;
	return utf8conv.from_bytes(input);
}

v8::Persistent<Function> loggingCallbackPersist;
coreclr_shutdown_ptr shutdownCoreClr = nullptr;
void* hostHandle;
unsigned int domainId;
#if WINDOWS
 	HMODULE coreClr;
#elif LINUX
	void* coreClr;
#endif

void log(Isolate* isolate, v8::Local<String> str) {
    Handle<Value> argv[] = { str };
    Local<Function>::New(isolate, loggingCallbackPersist)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
}

int ReportProgressCallback(int progress) {
	// Just print the progress parameter to the console and return -progress
	printf("Received status from managed code: %d\n", progress);
	return -progress;
}

void log(Isolate* isolate, const wchar_t* text) {
    auto str = String::NewFromTwoByte(isolate, (uint16_t*)text);
    log(isolate, str); 
}

void log(Isolate* isolate, const char* text) {
    auto str = New<String>(text).ToLocalChecked();
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
    auto pathResult = Handle<Object>::Cast(res);
    String::Utf8Value coreclrPathValue(pathResult->Get(New<String>("path").ToLocalChecked()));
    auto coreclrPath = *coreclrPathValue;
    String::Utf8Value coreclrDllPathValue(pathResult->Get(New<String>("dll").ToLocalChecked()));
    auto coreclrDllPath = *coreclrDllPathValue;
    String::Utf8Value tpaListValue(pathResult->Get(New<String>("tpaList").ToLocalChecked()));
    auto tpaList = *tpaListValue;
    String::Utf8Value appPathValue(pathResult->Get(New<String>("appPath").ToLocalChecked()));
    auto appPath = *appPathValue;

#if WINDOWS
 	coreClr = LoadLibraryExA(coreclrDllPath, nullptr, 0);
#elif LINUX
	coreClr = dlopen(coreclrDllPath, RTLD_NOW | RTLD_LOCAL);
#endif
	if (!coreClr) {
		log(isolate, "ERROR: Failed to load CoreCLR from");
        return;
    }

#if WINDOWS
	coreclr_initialize_ptr initializeCoreClr = (coreclr_initialize_ptr)GetProcAddress(coreClr, "coreclr_initialize");
	coreclr_create_delegate_ptr createManagedDelegate = (coreclr_create_delegate_ptr)GetProcAddress(coreClr, "coreclr_create_delegate");
	shutdownCoreClr = (coreclr_shutdown_ptr)GetProcAddress(coreClr, "coreclr_shutdown");
#elif LINUX
	coreclr_initialize_ptr initializeCoreClr = (coreclr_initialize_ptr)dlsym(coreClr, "coreclr_initialize");
	coreclr_create_delegate_ptr createManagedDelegate = (coreclr_create_delegate_ptr)dlsym(coreClr, "coreclr_create_delegate");
	coreclr_shutdown_ptr shutdownCoreClr = (coreclr_shutdown_ptr)dlsym(coreClr, "coreclr_shutdown");
#endif

	if (!initializeCoreClr) {
		log(isolate, "coreclr_initialize not found");
		return;
	}

	if (!createManagedDelegate) {
		log(isolate, "coreclr_create_delegate not found");
		return;
	}

	if (!shutdownCoreClr) {
		log(isolate, "coreclr_shutdown not found");
		return;
	}        

	// Define CoreCLR properties
	// Other properties related to assembly loading are common here,
	// but for this simple sample, TRUSTED_PLATFORM_ASSEMBLIES is all
	// that is needed. Check hosting documentation for other common properties.
	const char* propertyKeys[] = {
		"TRUSTED_PLATFORM_ASSEMBLIES"      // Trusted assemblies
	};

	const char* propertyValues[] = {
		tpaList
	};

	// This function both starts the .NET Core runtime and creates
	// the default (and only) AppDomain
	int hr = initializeCoreClr(
		appPath,
		"NodeDotnetBridge", // AppDomain friendly name
		sizeof(propertyKeys) / sizeof(char*),   // Property count
		propertyKeys,       // Property names
		propertyValues,     // Property values
		&hostHandle,        // Host handle
		&domainId);         // AppDomain ID

	if (hr >= 0)
        log(isolate, "CoreCLR started\n");
	else {
        log(isolate, "coreclr_initialize failed\n"); //printf("coreclr_initialize failed - status: 0x%08x\n", hr);
		return;
	}

	// The assembly name passed in the third parameter is a managed assembly name
	// as described at https://docs.microsoft.com/dotnet/framework/app-domains/assembly-names
	hr = createManagedDelegate(
		hostHandle,
		domainId,
		//"NodeDotnet, Version=1.0.0.0",
		"NodeDotnet",
		"NodeDotnet.Bridge",
		"ExecuteSync",
		(void**)& executeSyncDelegate);

	if (hr >= 0)
		log(isolate, "Managed delegate created\n");
	else
	{
		log(isolate, "coreclr_create_delegate failed"); // - status: 0x%08x\n", hr);
		return;
	}

	auto ret = executeSyncDelegate(L"Das kömmt äüs däm ßchönen Äddon");
	log(isolate, ret);

#if WINDOWS
	CoTaskMemFree(ret);
#elif LINUX
	free(ret);
#endif
        
    log(isolate, "Initialization finished");
}

NAN_METHOD(UnInitialize) {
    auto isolate = info.GetIsolate();

    if (shutdownCoreClr) {
        auto hr = shutdownCoreClr(hostHandle, domainId);
        if (hr >= 0)
            log(isolate, "CoreCLR successfully shutdown\n");
        else
            log(isolate, "coreclr_shutdown failed"); //- status: 0x%08x\n", hr);
    }
	// Unload CoreCLR
#if WINDOWS
	if (!FreeLibrary(coreClr))
		log(isolate, "Failed to free coreclr.dll");
#elif LINUX
	if (dlclose(coreClr))
		log(isolate, "Failed to free libcoreclr.so\n");
#endif

    loggingCallbackPersist.Reset();
}

NAN_METHOD(Test) {
    auto isolate = info.GetIsolate();

    v8::String::Value s(info[0]);
	auto ret = executeSyncDelegate((wchar_t*)*s);
	log(isolate, ret);
#if WINDOWS
	CoTaskMemFree(ret);
#elif LINUX
	free(ret);
#endif
}

NAN_MODULE_INIT(init) {
    Nan::Set(target, New<String>("initialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(Initialize)).ToLocalChecked());
    Nan::Set(target, New<String>("unInitialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(UnInitialize)).ToLocalChecked());

    Nan::Set(target, New<String>("test").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(Test)).ToLocalChecked());
}

NODE_MODULE(node_dotnet, init)