#include <node.h>
#include <string>
using namespace std;

using namespace v8;

Persistent<Function> loggingCallbackPersist;

void log(Isolate* isolate, const uint16_t* text) {
    auto str = String::NewFromTwoByte(isolate, text);
    Handle<Value> argv[] = { str };
    Local<Function>::New(isolate, loggingCallbackPersist)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
}

void Initialize(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    auto callback = Local<Function>::Cast(args[0]);
    loggingCallbackPersist.Reset(isolate, callback);
}

void TestLogging(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    v8::String::Value s(args[0]);
    log(isolate, *s);
    log(isolate, (uint16_t*)L"Däs wäre schön");
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