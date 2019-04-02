#include "Proxy.h"
using namespace std;
using namespace Nan;

NAN_METHOD(ProxyObject::New) {
    if (!info.IsConstructCall()) {
        // [NOTE] generic recursive call with `new`
        std::vector<v8::Local<v8::Value>> args(info.Length());
        for (std::size_t i = 0; i < args.size(); ++i) 
            args[i] = info[i];
        return;
    }
    
    v8::String::Value s(info[0]);
    auto proxy = new ProxyObject((wchar_t*)*s);

    // [NOTE] break vm if run the under code in no-`new` function call 
    proxy->Wrap(info.This()); // `Wrap` bind C++ object to JS object 
}

NAN_METHOD(ProxyObject::ExecuteSync) {
    // `Unwrap` refer C++ object from JS Object
    auto proxy = Nan::ObjectWrap::Unwrap<ProxyObject>(info.Holder());
}

NAN_METHOD(ProxyObject::ExecuteAsync) {
    auto proxy = Nan::ObjectWrap::Unwrap<ProxyObject>(info.Holder());
}

NAN_MODULE_INIT(ProxyObject::Init) {
    auto cname = Nan::New("ProxyObject").ToLocalChecked();
    auto ctor = Nan::New<v8::FunctionTemplate>(New);
    auto ctorInst = ctor->InstanceTemplate(); // target for member functions
    ctor->SetClassName(cname); // as `ctor.name` in JS
    ctorInst->SetInternalFieldCount(1); // for ObjectWrap, it should set 1

    // add member functions and accessors
    Nan::SetPrototypeMethod(ctor, "executeSync", ExecuteSync);
    Nan::SetPrototypeMethod(ctor, "executeAsync", ExecuteAsync);
  
    Nan::Set(target, cname, Nan::GetFunction(ctor).ToLocalChecked());
}