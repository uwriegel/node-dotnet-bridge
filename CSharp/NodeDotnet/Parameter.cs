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
            var t = info.ParameterType.Name;
            switch (t)
            {
                case "String":
                    Type = ParameterType.String;
                    InternalType = InternalParameterType.String;
                    break;
                case "Int32":
                case "UInt32":
                    InternalType = InternalParameterType.Int;
                    Type = ParameterType.Number;
                    break;
                case "Int64":
                case "Float":
                case "Double":
                case "UInt64":
                    Type = ParameterType.Number;
                    InternalType = InternalParameterType.Double;
                    break;
                case "DateTime":
                    Type = ParameterType.Date;
                    InternalType = InternalParameterType.Date;
                    break;
                case "Task`1":
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
