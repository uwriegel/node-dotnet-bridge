// console.log("bin bereit")
// console.log(__dirname)

// //"main": "./build/Release/node-dotnet-bridge",

// class User {
//     run(text) { console.log(text)}
// }

// var path = require('path');
// var pa = path.resolve('./build/' + process.platform + '/node-dotnet-bridge')
// console.log(pa)
var addon = require('./build/Debug/node-dotnet-bridge');

// var proxyObject = addon.ProxyObject

// console.log(__dirname)

// module.exports.User = User
module.exports = addon