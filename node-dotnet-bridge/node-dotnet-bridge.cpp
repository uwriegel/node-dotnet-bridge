#include <nan.h>
#include <string>

#if WINDOWS
#include <Windows.h>
#endif

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
#else
    auto clrBasePath = "C:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\";
#endif
    auto resolveCoreclr = Local<Function>::Cast(settings->Get(New<String>("resolveCoreclr").ToLocalChecked()));
    auto var = New<String>(clrBasePath).ToLocalChecked();
    Handle<Value> argv[] = { var };
    auto res = resolveCoreclr->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    v8::String::Utf8Value s(res);
    const char* coreclrPath = *s;

#if WINDOWS
 	auto coreClr = LoadLibraryExA(coreclrPath, nullptr, 0);
#elif LINUX
	void* coreClr = dlopen(coreClrPath.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
	if (!coreClr)
		log(isolate, L"ERROR: Failed to load CoreCLR from");
}

NAN_METHOD(TestLogging) {
    auto isolate = info.GetIsolate();

    v8::String::Value s(info[0]);
    log(isolate, (wchar_t*)*s);
    log(isolate, L"Däs wäre schön mit Nan 😢😢");
}

NAN_METHOD(UnInitialize) {
    loggingCallbackPersist.Reset();
}

NAN_MODULE_INIT(init) {
    Nan::Set(target, New<String>("initialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(Initialize)).ToLocalChecked());
    Nan::Set(target, New<String>("unInitialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(UnInitialize)).ToLocalChecked());
}

NODE_MODULE(node_dotnet, init)