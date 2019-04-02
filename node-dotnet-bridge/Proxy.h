#pragma once
#include <nan.h>
#include <string>

class ProxyObject: public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);
private:
    static NAN_METHOD(New); 
    static NAN_METHOD(ExecuteSync);
    static NAN_METHOD(ExecuteAsync); 

    ProxyObject(wchar_t* name) 
    : name(name) 
    {}

    ~ProxyObject() {
    }
public:    
    std::wstring name;
};