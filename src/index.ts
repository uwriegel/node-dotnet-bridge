import { initialize, unInitialize, testLogging } from 'node-dotnet-bridge'

console.log("Start " + (new Date).toTimeString())

const log = function(text: string) { console.log(text)}

initialize(log)

console.log("hell16 " + (new Date).toTimeString())

testLogging("Das was äber schön 👌😁")

unInitialize()

console.log("Finished")
