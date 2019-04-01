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
            (Type, NumberType, SubType) = GetParameter(t, info);
        }

        (ParameterType, NumberType, ParameterType) GetParameter(string name, ParameterInfo info)
        {
            switch (name)
            {
                case "String":
                    return (ParameterType.String, NumberType.Nan, ParameterType.None);
                case "Int32":
                case "UInt32":
                    return (ParameterType.Number, NumberType.Int, ParameterType.None);
                case "Int64":
                case "Float":
                case "Double":
                case "UInt64":
                    return (ParameterType.Number, NumberType.Double, ParameterType.None);
                case "DateTime":
                    return (ParameterType.Date, NumberType.Nan, ParameterType.None);
                case "Task`1":
                    var fullName = info.ParameterType.FullName;
                    var pos1 = fullName.IndexOf("[[") + 2;
                    var subTypeStr = fullName.Substring(pos1);
                    pos1 = subTypeStr.IndexOf(".") + 1;
                    var pos2 = subTypeStr.IndexOf(",");
                    subTypeStr = subTypeStr.Substring(pos1, pos2 - pos1);
                    var (subType, subNumberType, _) = GetParameter(subTypeStr, info);
                    return (ParameterType.Task, subNumberType, subType);
                default:
                    return (ParameterType.None, NumberType.Nan, ParameterType.None);
            }
        }

        [DataMember(Name = "name")]
        public string Name;
        [DataMember(Name = "type")]
        public ParameterType Type;
        [DataMember(Name = "subType", EmitDefaultValue=false)]
        public ParameterType SubType;

        internal NumberType NumberType { get; }
    }
}
