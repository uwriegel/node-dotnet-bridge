#include "AsyncContext.h"
#include "Nan.h"
using namespace Nan;

AsyncContext::AsyncContext(const v8::Local<v8::Function>& function) : callback(function) {
    uv_async_init(uv_default_loop(), &uv_async, (uv_async_cb)ContinueOnV8Thread);
}

AsyncContext::~AsyncContext() {
    delete data;
}

void AsyncContext::ExecuteAction() {
    uv_async_send(&uv_async);
}

void AsyncContext::ContinueOnV8Thread(uv_async_t* handle, int status) {
    // This executes on V8 thread
    Nan::HandleScope scope;

    auto asyncContext = (AsyncContext*)handle;
    void* data = asyncContext->data;
    CancelAction(asyncContext);

    auto str = New<v8::String>((const char*)data).ToLocalChecked();
    v8::Handle<v8::Value> argv[] = { str };
    asyncContext->callback.Call(GetCurrentContext()->Global(), 1, argv);
}

void AsyncContext::CancelAction(AsyncContext* asyncContext)
{
    // This is a cancellation of an action registered on V8 thread.
    // Unref the handle to stop preventing the process from exiting.
    uv_unref((uv_handle_t*)&asyncContext->uv_async);
    uv_close((uv_handle_t*)&asyncContext->uv_async, CloseCallback);
}

void AsyncContext::CloseCallback(uv_handle_t* handle) {
    auto asyncContext = (AsyncContext*)handle;
    delete asyncContext;
}

