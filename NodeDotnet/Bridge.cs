using System;
using System.Collections.Generic;
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
            return "Retörning from Mänaged Cöde: " + input;
        }
    }
}
