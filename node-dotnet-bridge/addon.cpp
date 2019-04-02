#include <nan.h>
#include <uv.h>
#include "Proxy.h"

NAN_MODULE_INIT(init) {
    ProxyObject::Init(target);
}

NODE_MODULE(node_dotnet, init)