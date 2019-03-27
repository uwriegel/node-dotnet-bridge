import * as fs from 'fs'
import { initialize, unInitialize } from 'node-dotnet-bridge'

const log = function(text: string) { console.log(text) }

const resolveCoreclr = function(basePath: string) {
    const version = fs.readdirSync(basePath).sort((a, b) => - a.localeCompare(b))[0]
    const result = basePath + version + "\\coreclr.dll"
    console.log("Loading dotnet core from " + result)
    return result
}

// TODO: logCallback: kann auch null sein, dann kein Logging
initialize({
    logCallback: log,
    resolveCoreclr: resolveCoreclr
})

unInitialize()
console.log("Finished")
