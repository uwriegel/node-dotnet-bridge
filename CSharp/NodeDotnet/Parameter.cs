using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.Serialization;
using System.Text;

namespace NodeDotnet
{
    [DataContract]
    public class Parameter
    {
        public Parameter() { }
        public Parameter(ParameterInfo info)
        {
            Name = info.Name;
            var t = info.ParameterType.ToString();
            switch (t)
            {
                case "System.String":
                    Type = ParameterType.String;
                    InternalType = InternalParameterType.String;
                    break;
                case "System.Int32":
                case "System.UInt32":
                    InternalType = InternalParameterType.Int;
                    Type = ParameterType.Number;
                    break;
                case "System.Int64":
                case "System.Float":
                case "System.Double":
                case "System.UInt64":
                    Type = ParameterType.Number;
                    InternalType = InternalParameterType.Double;
                    break;
                case "System.DateTime":
                    Type = ParameterType.Date;
                    InternalType = InternalParameterType.Date;
                    break;
            }
        }

        [DataMember(Name = "name")]
        public string Name;
        [DataMember(Name = "type")]
        public ParameterType Type;

        internal InternalParameterType InternalType { get; }
    }
}
