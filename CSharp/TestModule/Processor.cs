using NodeDotnet.Core;
using System;
using System.Collections.Generic;
using System.Text;

namespace TestModule
{
    [JavascriptObject]
    public class Processor
    {
        [JavascriptMethod]
        public string GetTest(string text, double number, DateTime dateTime)
            => $"Returned from CSharp: {text}, {number}, {dateTime}";
        [JavascriptMethod]
        public int Add(int a, int b) => a + b;

        public int Noop() => 9;
    }
}
