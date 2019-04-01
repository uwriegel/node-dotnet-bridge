using System;
using System.Collections.Generic;
using System.Text;

namespace NodeDotnet.Core
{
    public interface IJavascriptObject
    {
        string Execute(string method, string payload);
    }
}
