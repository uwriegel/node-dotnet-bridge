using NodeDotnet.Core;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace Tester
{
    class Program
    {
        static void Main(string[] args)
        {
            var assembly = Assembly.Load("TestModule");
            var type = assembly.GetType("TestModule.Processor");
            var obj = Activator.CreateInstance(type);
            var jsObject = obj as IJavascriptObject;
            var result = jsObject.Execute("Run", "Das kömmt auüs dem Teßter 😅😅😅🤣🤣🤣");

            objs["123"] = jsObject;
            objs["1233"] = jsObject;
            objs["1233"] = jsObject;
            objs["1234"] = jsObject;
            objs["1234"] = jsObject;
            objs["12322"] = jsObject;
            objs["1243"] = jsObject;
            objs["1283"] = jsObject;
            objs["1923"] = jsObject;
            objs["Run"] = jsObject;

            string Execute(string method, string test)
            {
                return objs[method].Execute(method, test);
            }


            for (var i = 0; i < 100_000_000; i++)
                Execute("Run", "Das kömmt auüs dem Teßter 😅😅😅🤣🤣🤣");


            Console.WriteLine("Hello World!");

        }
        static Dictionary<string, IJavascriptObject> objs = new Dictionary<string, IJavascriptObject>();
    }
}
