var addon = require('./build/Debug/node-dotnet-bridge');
//var addon = require('./build/Release/node-dotnet-bridge');

console.log("Dynamically creating classes")
const className = "Processor"
const classData = `
(class Processor extends addon.ProxyObject
    {
        constructor() { super("Processor")} 
        getObject(text) {
            return JSON.parse(super.executeSync(text))
        }
    })`

module.exports[className] = eval(classData)
