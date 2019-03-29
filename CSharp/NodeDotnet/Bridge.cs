using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Linq;
using System.Threading;
using NodeDotnet.Core;

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
            var allTypes = assembly.GetExportedTypes();

            var objects =
                from n in allTypes
                where n.GetCustomAttribute(typeof(JavascriptObjectAttribute)) != null
                select n;

            var methods =
                from n in
                    from t in allTypes
                    where t.GetCustomAttribute(typeof(JavascriptObjectAttribute)) != null
                    select t
                from m in n.GetMethods()
                where m.GetCustomAttribute(typeof(JavascriptMethodAttribute)) != null
                select new Method(n.Name, m.Name, m.GetParameters(), m.ReturnParameter);
            return null;
        }

        [return: MarshalAs(UnmanagedType.LPWStr)]
        public static string ExecuteSync([MarshalAs(UnmanagedType.LPWStr)] string input)
            =>"Retörning from Mänaged Cöde: " + input;
    }
}
