using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.Reflection;
using NodeDotnet.Core;

namespace NodeDotnet
{
    struct ObjectInfo
    {
        public ObjectInfo(Type t)
        {
            FullName = t.FullName;
            var methods = t.GetMethods().Where(m => m.GetCustomAttribute(typeof(JavascriptMethodAttribute)) != null);
            Methods = methods.ToDictionary(n => n.Name, n => new Method(n));
        }
        public string FullName { get; }
        public Dictionary<string, Method> Methods { get; }
    }
}
