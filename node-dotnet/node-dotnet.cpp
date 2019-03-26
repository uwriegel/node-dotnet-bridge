#include <node.h>
#include <string>

#if WINDOWS
#include <Windows.h>
#endif

using namespace std;

using namespace v8;

Persistent<Function> loggingCallbackPersist;

void log(Isolate* isolate, const wchar_t* text) {
    auto str = String::NewFromTwoByte(isolate, (uint16_t*)text);
    Handle<Value> argv[] = { str };
    Local<Function>::New(isolate, loggingCallbackPersist)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
}

void Initialize(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    auto callback = Local<Function>::Cast(args[0]);
    loggingCallbackPersist.Reset(isolate, callback);

    const char* coreclrPath = "C:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\2.2.3\\coreclr.dll";

#if WINDOWS
	// <Snippet1>
	auto coreClr = LoadLibraryExA(coreclrPath, nullptr, 0);
	// </Snippet1>
#elif LINUX
	void* coreClr = dlopen(coreClrPath.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
	if (!coreClr)
		log(isolate, L"ERROR: Failed to load CoreCLR from");
}

void TestLogging(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    v8::String::Value s(args[0]);
    log(isolate, (wchar_t*)*s);
    log(isolate, L"Däs wäre schön😢😢");
}

void UnInitialize(const FunctionCallbackInfo<Value>& args) {
    loggingCallbackPersist.Reset();
}

void Method8(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::String::Utf8Value s(args[0]);
    string str(*s);
    Local<String>  retval = String::NewFromUtf8(isolate, str.c_str());
    args.GetReturnValue().Set(retval);
}

void Method16(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::String::Value s(args[0]);
    wstring wstr((wchar_t*)*s);
    Local<String> retval = String::NewFromTwoByte(isolate, (uint16_t*)wstr.c_str());
    args.GetReturnValue().Set(retval);
}

void init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "initialize", Initialize);
    NODE_SET_METHOD(exports, "testLogging", TestLogging);
    NODE_SET_METHOD(exports, "unInitialize", UnInitialize);

    NODE_SET_METHOD(exports, "hello8", Method8);
    NODE_SET_METHOD(exports, "hello16", Method16);
}

NODE_MODULE(node_dotnet, init)