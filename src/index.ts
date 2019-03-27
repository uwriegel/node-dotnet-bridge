import { initialize, unInitialize, testLogging, status } from 'node-dotnet-bridge'

console.log("Start " + (new Date).toTimeString())

const log = function(text: string) { console.log(text)}

let aff = status
console.log(status)

// TODO: logCallback: kann auch null sein, dann kein Logging
initialize({
    logCallback: log
})

console.log(status)

console.log("hell16 " + (new Date).toTimeString())

testLogging("Das was äber schön 👌😁")

unInitialize()

console.log("Finished")
