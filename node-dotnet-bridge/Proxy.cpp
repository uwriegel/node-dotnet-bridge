#include <thread>
#include "Proxy.h"
#include "AsyncContext.h"
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

    const char* objResult = "{\"value1\":1,\"text\":\"Hallo\",\"values\":[1,2,3]}";
    auto cname = Nan::New(objResult).ToLocalChecked();
    info.GetReturnValue().Set(cname);
}

NAN_METHOD(ProxyObject::ExecuteAsync) {
    auto proxy = Nan::ObjectWrap::Unwrap<ProxyObject>(info.Holder());

    auto callback = v8::Local<v8::Function>::Cast(info[1]);
    auto asyncContext = new AsyncContext(callback);

    // const char *res = "This is the data for callbäck!!!😁😁😁👏👏";
    // asyncContext->data = new char[strlen(res)+1];
    // strcpy((char*)asyncContext->data, res);
    // asyncContext->ExecuteAction();

    auto t1 = thread([asyncContext]()-> void {
        //this_thread::sleep_for(10s);
        const char *res = "This is the data for callbäck!!!😁😁😁👏👏";
        asyncContext->data = new char[strlen(res)+1];
        strcpy((char*)asyncContext->data, res);
        asyncContext->ExecuteAction();
    });
    t1.detach();
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