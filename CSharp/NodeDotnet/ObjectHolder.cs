using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.Reflection;
using NodeDotnet.Core;

namespace NodeDotnet
{
    struct ObjectHolder
    {
        public ObjectHolder(object obj, ObjectInfo info)
        {
            Object = obj;
            Info = info;
        }
        public ObjectInfo Info { get; }
        public object Object { get; }
    }
}
