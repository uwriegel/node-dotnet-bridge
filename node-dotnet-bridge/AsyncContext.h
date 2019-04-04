#pragma once
#include <nan.h>
#include <uv.h>

class AsyncContext {
public:
    AsyncContext(const v8::Local<v8::Function>& function);
    ~AsyncContext();
    void ExecuteAction();
private:

    static void ContinueOnV8Thread(uv_async_t* handle, int status);
    static void CancelAction(AsyncContext* asyncContext);
    static void CloseCallback(uv_handle_t* handle); 
    static void Unref();

    uv_async_t uv_async;
public:    
    void* data{nullptr};
    Nan::Callback callback;       
};

