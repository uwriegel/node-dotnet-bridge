using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;

namespace NodeDotnet
{
    class Method
    {
        public string ContainingClass { get; }

        public Method(string containingClass, string name, IEnumerable<ParameterInfo> parameters, ParameterInfo returnValue)
        {
            ContainingClass = containingClass;
            this.name = name;
            this.parameters = parameters;
            this.returnValue = returnValue;
        }

        string name;
        IEnumerable<ParameterInfo> parameters;
        ParameterInfo returnValue;
    }
}
