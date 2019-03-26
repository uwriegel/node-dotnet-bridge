#include <node.h>
#include <string>
using namespace std;

using namespace v8;

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
    Local<String>  retval = String::NewFromTwoByte(isolate, (uint16_t*)wstr.c_str());
    args.GetReturnValue().Set(retval);
}

void init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "hello8", Method8);
    NODE_SET_METHOD(exports, "hello16", Method16);
}

NODE_MODULE(node_dotnet, init)