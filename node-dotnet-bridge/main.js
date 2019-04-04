// console.log(__dirname)

// class User {
//     run(text) { console.log(text)}
// }

// var path = require('path');
// var pa = path.resolve('./build/' + process.platform + '/node-dotnet-bridge')
// console.log(pa)
//var addon = require('./build/Debug/node-dotnet-bridge');
var addon = require('./build/Release/node-dotnet-bridge');

const processor = eval(`
(class Processor extends addon.ProxyObject
    {
        constructor() { super("Processor")} 
        getObject(text) {
            return JSON.parse(super.executeSync(text))
        }
    })`)
    
// var proxyObject = addon.ProxyObject

// console.log(__dirname)

// module.exports.User = User
module.exports = addon
// TODO: to C++
module.exports.Processor = processor
