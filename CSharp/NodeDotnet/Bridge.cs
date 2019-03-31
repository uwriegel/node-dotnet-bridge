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
        [DllImport("user32.dll")]
        static extern int MessageBox(IntPtr hWnd, string text, string caption, int type);

        [return: MarshalAs(UnmanagedType.LPWStr)]
        public static string Initialize([MarshalAs(UnmanagedType.LPWStr)] string assemblyName)
        {
            // TODO: in managedBridge, read all assemblies in root (TestModule) and add those to tpaList
            // TODO: return MethodInfos as json to calling javascript. Create javascript classes in C++
            // TODO: At the moment creater d.ts file manually
            // TODO: Constructor of C++-Proxy -> Invoke new Object in TestModule

            //MessageBox(IntPtr.Zero, "Haha", "Huhu", 0);
            assembly = Assembly.Load(assemblyName);
            objectNames = assembly.GetExportedTypes().Select(n => (n.Name, n.FullName)).ToDictionary(m => m.Name, m => m.FullName);
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

        public static void ConstructObject(int objectId, [MarshalAs(UnmanagedType.LPWStr)] string name)
        {
            var fullname = objectNames[name];
            var t = assembly.GetType(fullname);
            var o = Activator.CreateInstance(t);
            objects[objectId] = o;
        }

        public static void DeleteObject(int objectId)
            => objects.Remove(objectId);

        [return: MarshalAs(UnmanagedType.LPWStr)]
        public static string ExecuteSync(int objectId, [MarshalAs(UnmanagedType.LPWStr)] string method, [MarshalAs(UnmanagedType.LPWStr)] string input)
            =>"Retörning from Mänaged Cöde: " + input;

        public static void Execute(int objectId, [MarshalAs(UnmanagedType.LPWStr)] string method,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)][In] byte[] payload, int size)
        {

        }

        static Dictionary<string, string> objectNames = new Dictionary<string, string>();
        static Dictionary<int, object> objects = new Dictionary<int, object>();
        static Assembly assembly;
    }
}
