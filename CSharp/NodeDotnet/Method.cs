using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using System.Linq;
using System.Runtime.Serialization;

namespace NodeDotnet
{
    [DataContract]
    public class Method
    {
        [DataMember(Name = "name")]
        public string Name { get; set; }
        [DataMember(Name = "parameters")]
        public Parameter[] Parameters { get; set; }
        [DataMember(Name = "returnValue")]
        public Parameter ReturnValue { get; set; }

        public MethodInfo Info { get; }
        
        public ParameterInfo ReturnInfo { get; set; }

        public Method() { }

        public Method(MethodInfo methodInfo)
        {
            Name = methodInfo.Name;
            Info = methodInfo;
            Parameters = methodInfo.GetParameters().Select(n => new Parameter(n)).ToArray();

            ReturnInfo = methodInfo.ReturnParameter;
            ReturnValue = new Parameter(ReturnInfo);
        }
    }
}
