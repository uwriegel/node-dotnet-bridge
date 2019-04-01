using NodeDotnet.Core;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace TestModule
{
    [JavascriptObject]
    public class Processor : IJavascriptObject
    {
        [JavascriptMethod]
        public string GetTest(string text, double number, DateTime dateTime)
            => $"Returned from CSharp: {text}, {number}, {dateTime}";
        [JavascriptMethod]
        public int Add(int a, int b) => a + b;

        [JavascriptMethod]
        public async Task<int> AddAsync(int a, int b)
        {
            await Task.Delay(1000);
            return a + b;
        }

        public int Noop() => 9;

        public string Execute(string method, string payload)
            => "returned from C#: " + payload; 
    }
}
