using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.Serialization;

namespace NodeDotnet
{
    [DataContract]
    public class JSClass
    {
        public JSClass() { }

        public JSClass(string name, IEnumerable<Method> methods)
        {
            Name = name;
            Methods = methods.ToArray();
        }

        [DataMember(Name="name")]
        public string Name { get; set; }
        [DataMember(Name = "methods")]
        public Method[] Methods { get; set; }
    }
}
