using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Linq;
using System.Threading;
using NodeDotnet.Core;
using System.Runtime.Serialization.Json;
using System.IO;

namespace NodeDotnet
{
    public class Bridge
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        public static string Initialize([MarshalAs(UnmanagedType.LPWStr)] string assemblyName)
        {
            // TODO: in managedBridge, read all assemblies in root (TestModule) and add those to tpaList
            // TODO: return MethodInfos as json to calling javascript. Create javascript classes in C++
            // TODO: At the moment creater d.ts file manually
            // TODO: Constructor of C++-Proxy -> Invoke new Object in TestModule

            var assembly = Assembly.Load(assemblyName);
            var objects = 
                from n in assembly.GetExportedTypes()
                where n.GetCustomAttribute(typeof(JavascriptObjectAttribute)) != null
                select new JSClass(n.Name,
                    from m in n.GetMethods()
                    where m.GetCustomAttribute(typeof(JavascriptMethodAttribute)) != null
                    select new Method(m.Name, m.GetParameters(), m.ReturnParameter));
            var seri = new DataContractJsonSerializer(typeof(JSClass[]));
            var ms = new MemoryStream();
            seri.WriteObject(ms, objects.ToArray());
            ms.Capacity = (int)ms.Length;
            var buff = ms.GetBuffer();
            var result = Encoding.UTF8.GetString(buff);
            return result;
        }

        [return: MarshalAs(UnmanagedType.LPWStr)]
        public static string ExecuteSync([MarshalAs(UnmanagedType.LPWStr)] string input)
            =>"Retörning from Mänaged Cöde: " + input;
    }
}
