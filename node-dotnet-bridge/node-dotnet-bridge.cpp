#include <nan.h>
#include <string>

#if WINDOWS
#include <Windows.h>
#endif

using namespace std;

using namespace Nan;
using namespace v8;

v8::Persistent<Function> loggingCallbackPersist;
Nan::Persistent<v8::String> statusPersist;

void log(Isolate* isolate, const wchar_t* text) {
    auto str = String::NewFromTwoByte(isolate, (uint16_t*)text);
    Handle<Value> argv[] = { str };
    Local<Function>::New(isolate, loggingCallbackPersist)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
}

NAN_METHOD(Initialize) {
    auto isolate = info.GetIsolate();

    auto settings = Handle<Object>::Cast(info[0]);
    auto loggingValue = settings->Get(New<String>("logCallback").ToLocalChecked());
    auto callback = Local<Function>::Cast(loggingValue);
    loggingCallbackPersist.Reset(isolate, callback);

    const char* coreclrPath = "C:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\2.2.2\\coreclr.dll";

#if WINDOWS
 	// <Snippet1>
 	auto coreClr = LoadLibraryExA(coreclrPath, nullptr, 0);
 	// </Snippet1>
#elif LINUX
	void* coreClr = dlopen(coreClrPath.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
	if (!coreClr)
		log(isolate, L"ERROR: Failed to load CoreCLR from");

    statusPersist.Reset(New<String>("ready").ToLocalChecked());
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
    Nan::Set(target, New<String>("testLogging").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(TestLogging)).ToLocalChecked());
    Nan::Set(target, New<String>("unInitialize").ToLocalChecked(), Nan::GetFunction(New<FunctionTemplate>(UnInitialize)).ToLocalChecked());

    auto status = New<String>("new").ToLocalChecked();
    statusPersist.Reset(status);
    Nan::Set(target, New<String>("status").ToLocalChecked(), status);
}

NODE_MODULE(node_dotnet, init)