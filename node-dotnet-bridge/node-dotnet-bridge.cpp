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
typedef wchar_t* (*initializePtr)(const wchar_t* assemblyName);
initializePtr initializeDelegate;
typedef void (*constructObjectPtr)(int objectId, const wchar_t* objectName);
constructObjectPtr constructObjectDelegate;
typedef void (*deleteObjectPtr)(int objectId);
deleteObjectPtr deleteObjectDelegate;
typedef wchar_t* (*executeSyncPtr)(int objectId, const wchar_t* methodName, wchar_t* payload);
executeSyncPtr executeSyncDelegate;
typedef wchar_t* (*executePtr)(int objectId, const wchar_t* methodName, char* payload, int length);
executePtr executeDelegate;

string ws2utf8(const wstring &input) {
	wstring_convert<codecvt_utf8<wchar_t>> utf8conv;
	return utf8conv.to_bytes(input);
}

wstring utf82ws(const string &input) {
	wstring_convert<codecvt_utf8<wchar_t>> utf8conv;
	return utf8conv.from_bytes(input);
}

v8::Persistent<Function> loggingCallbackPersist;
v8::Persistent<Function> deserializeCallbackPersist;
coreclr_shutdown_ptr shutdownCoreClr = nullptr;
void* hostHandle;
unsigned int domainId;
#if WINDOWS
 	HMODULE coreClr;
#elif LINUX
	void* coreClr;
#endif

void log(Isolate* isolate, const char* text);

void log(Isolate* isolate, v8::Local<String> str) {
    Handle<Value> argv[] = { str };
    Local<Function>::New(isolate, loggingCallbackPersist)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
}

void deserialize(Isolate* isolate, const wchar_t* text) {
    auto str = String::NewFromTwoByte(isolate, (uint16_t*)text);
    Handle<Value> argv[] = { str };
    auto res = Local<Function>::New(isolate, deserializeCallbackPersist)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    if (res->IsArray()) {
        Local<Array> array = Local<Array>::Cast(res);

        for (auto i = 0; i < array->Length(); i++ ) {
            if (Nan::Has(array, i).FromJust()) {
                Local<Object> objectInfo = Nan::Get(array, i).ToLocalChecked()->ToObject();
                String::Utf8Value objectNameVal(objectInfo->Get(New<String>("name").ToLocalChecked()));
                auto objectName = *objectNameVal;
                log(isolate, objectName);
            }                
        }
    }
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
    auto deserializeValue = settings->Get(New<String>("deserialize").ToLocalChecked());

    auto callback = Local<Function>::Cast(loggingValue);
    loggingCallbackPersist.Reset(isolate, callback);

    callback = Local<Function>::Cast(deserializeValue);
    deserializeCallbackPersist.Reset(isolate, callback);
printf("Super");
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
		"Initialize",
		(void**)&initializeDelegate);

	if (hr >= 0)
		log(isolate, "initializeDelegate created\n");
	else
	{
		log(isolate, "initializeDelegate failed"); // - status: 0x%08x\n", hr);
		return;
	}

    hr = createManagedDelegate(
		hostHandle,
		domainId,
		"NodeDotnet",
		"NodeDotnet.Bridge",
		"Execute",
		(void**)&executeDelegate);

	if (hr >= 0)
		log(isolate, "executeDelegate created\n");
	else
	{
		log(isolate, "executeDelegate failed"); // - status: 0x%08x\n", hr);
		return;
	}

	hr = createManagedDelegate(
		hostHandle,
		domainId,
		"NodeDotnet",
		"NodeDotnet.Bridge",
		"ConstructObject",
		(void**)&constructObjectDelegate);

	if (hr >= 0)
		log(isolate, "constructObjectDelegate created\n");
	else
	{
		log(isolate, "constructObjectDelegate failed"); // - status: 0x%08x\n", hr);
		return;
	}

	hr = createManagedDelegate(
		hostHandle,
		domainId,
		"NodeDotnet",
		"NodeDotnet.Bridge",
		"DeleteObject",
		(void**)&deleteObjectDelegate);

	if (hr >= 0)
		log(isolate, "deleteObjectDelegate created\n");
	else
	{
		log(isolate, "deleteObjectDelegate failed"); // - status: 0x%08x\n", hr);
		return;
	}

	hr = createManagedDelegate(
		hostHandle,
		domainId,
		"NodeDotnet",
		"NodeDotnet.Bridge",
		"ExecuteSync",
		(void**)&executeSyncDelegate);

	if (hr >= 0)
		log(isolate, "executeSyncDelegate created\n");
	else
	{
		log(isolate, "executeSyncDelegate failed"); // - status: 0x%08x\n", hr);
		return;
	}

    printf("Super");
	auto ret = initializeDelegate(L"TestModule");
	log(isolate, ret);
    deserialize(isolate, ret);

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

NAN_METHOD(Execute) {
    auto isolate = info.GetIsolate();
    Nan::Maybe<double> value = Nan::To<double>(info[0]); 
    double id = value.FromJust();
    v8::String::Value methodName(info[1]);

    int position = 0;
    char* buffer = (char*)malloc(1000);
    for (auto i = 2; i < info.Length(); i++) {
        if (info[i]->IsString()) {
            v8::String::Value strval(info[i]);
            auto str = (wchar_t*)*strval;
            int len = wcslen(str) * 2;
            memcpy(buffer + position, &len, 4);
            position+=4;
            memcpy(buffer + position, str, len);
            position += 2*len;
        }
        else if (info[i]->IsInt32() ||info[i]->IsUint32()) {
            Nan::Maybe<uint32_t> value = Nan::To<uint32_t>(info[i]); 
            uint32_t val = value.FromJust();   
            int len = 4;
            memcpy(buffer + position, &len, 4);         
            position+=4;
            memcpy(buffer + position, &val, 4);
            position+=4;
        }
    }

    auto ret = executeDelegate(id, (wchar_t*)*methodName, buffer, position);
    auto str = String::NewFromTwoByte(isolate, (uint16_t*)ret);
    info.GetReturnValue().Set(str);
    free(buffer);
#if WINDOWS
	CoTaskMemFree(ret);
#elif LINUX
	free(ret);
#endif
}

int proxyIdFactory = 0;

class ProxyObject: public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);
private:
    // Native JS Functions for accessing the custom object properties
    static NAN_METHOD(New); // constructor
    static NAN_METHOD(ExecuteSync); // method
    static NAN_GETTER(IdGet); // (specific) property getter
    // static NAN_SETTER(NameSet); // (specific) property setter

    // the native object properties
    ProxyObject() : id(++proxyIdFactory) {
        name = L"Processor";
        constructObjectDelegate(id, name.c_str());
    }

    ~ProxyObject() {
        deleteObjectDelegate(id);
    }
public:    
    int id;
    wstring name;
};

NAN_METHOD(ProxyObject::New) {
    if (!info.IsConstructCall()) {
        // [NOTE] generic recursive call with `new`
        std::vector<v8::Local<v8::Value>> args(info.Length());
        for (std::size_t i = 0; i < args.size(); ++i) 
            args[i] = info[i];
        // auto inst = Nan::NewInstance(info.Callee(), args.size(), args.data());
        // if (!inst.IsEmpty()) 
        //     info.GetReturnValue().Set(inst.ToLocalChecked());
        return;
    }
    
    //auto name = Nan::To<v8::String>(info[0]).ToLocalChecked();
    //auto proxy = new ProxyObject(*Nan::Utf8String(name));
    auto proxy = new ProxyObject();

    // [NOTE] break vm if run the under code in no-`new` function call 
    proxy->Wrap(info.This()); // `Wrap` bind C++ object to JS object 
}

NAN_METHOD(ProxyObject::ExecuteSync) {
    // `Unwrap` refer C++ object from JS Object
    auto proxy = Nan::ObjectWrap::Unwrap<ProxyObject>(info.Holder());

    auto isolate = info.GetIsolate();
    v8::String::Value s(info[0]);
 	auto ret = executeSyncDelegate(proxy->id, proxy->name.c_str(), (wchar_t*)*s);
    auto str = String::NewFromTwoByte(isolate, (uint16_t*)ret);
     info.GetReturnValue().Set(str);
#if WINDOWS
	CoTaskMemFree(ret);
#elif LINUX
	free(ret);
#endif
 }

NAN_GETTER(ProxyObject::IdGet) {
  auto proxy = Nan::ObjectWrap::Unwrap<ProxyObject>(info.Holder());
  auto id = Nan::New(proxy->id);
  info.GetReturnValue().Set(id);
}

// NAN_SETTER(NanPerson::NameSet) {
//   auto person = Nan::ObjectWrap::Unwrap<NanPerson>(info.Holder());
//   // [NOTE] `value` is defined argument in `NAN_SETTER`
//   auto name = Nan::To<v8::String>(value).ToLocalChecked();
//   person->name = *Nan::Utf8String(name);
// }


NAN_MODULE_INIT(ProxyObject::Init) {
    auto cname = Nan::New("ProxyObject").ToLocalChecked();
    auto ctor = Nan::New<v8::FunctionTemplate>(New);
    auto ctorInst = ctor->InstanceTemplate(); // target for member functions
    ctor->SetClassName(cname); // as `ctor.name` in JS
    ctorInst->SetInternalFieldCount(1); // for ObjectWrap, it should set 1

    // add member functions and accessors
    Nan::SetPrototypeMethod(ctor, "executeSync", ExecuteSync);
    auto pid = Nan::New("id").ToLocalChecked();
    Nan::SetAccessor(ctorInst, pid, IdGet);
  
    Nan::Set(target, cname, Nan::GetFunction(ctor).ToLocalChecked());
}

NAN_MODULE_INIT(init) {
    Nan::Set(target, New<String>("initialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(Initialize)).ToLocalChecked());
    Nan::Set(target, New<String>("unInitialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(UnInitialize)).ToLocalChecked());
    Nan::Set(target, New<String>("execute").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(Execute)).ToLocalChecked());
    ProxyObject::Init(target);
}

NODE_MODULE(node_dotnet, init)