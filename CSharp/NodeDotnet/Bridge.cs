using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace NodeDotnet
{
    public class Bridge
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        public static string ExecuteSync([MarshalAs(UnmanagedType.LPWStr)] string input)
        {
            var test = true;
            while (test)
                Thread.Sleep(1000);


            var assembly = Assembly.Load("TestModule");
            
            return "Retörning from Mänaged Cöde: " + input;

        }
    }
}
