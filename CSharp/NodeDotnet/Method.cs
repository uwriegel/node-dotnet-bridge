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
        [DataMember]
        public string Name { get; set; }
        [DataMember]
        public Parameter[] Parameters { get; set; }
        [DataMember]
        public Parameter ReturnValue { get; set; }

        public ParameterInfo[] ParameterInfos { get; set; }
        
        public ParameterInfo ReturnInfo { get; set; }

        public Method() { }

        public Method(string name, IEnumerable<ParameterInfo> parameters, ParameterInfo returnValue)
        {
            Name = name;
            ParameterInfos = parameters.ToArray();
            Parameters = ParameterInfos.Select(n => new Parameter(n)).ToArray();

            ReturnInfo = returnValue;
            ReturnValue = new Parameter(ReturnInfo);
        }
    }
}
