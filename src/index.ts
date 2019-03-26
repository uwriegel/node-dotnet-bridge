import { hello8, hello16 } from 'node-dotnet'

console.log("Start " + (new Date).toTimeString())

for (let i = 0; i < 1000000; i++) {
    let r = hello8("A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string")
}
console.log("hello8 " + (new Date).toTimeString())
for (let i = 0; i < 1000000; i++) {
    let rr = hello16("A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string A long string")
}
console.log("hell16 " + (new Date).toTimeString())
console.log("Finished")
