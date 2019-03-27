import * as fs from 'fs'
import * as Path from 'path'
import { initialize, unInitialize } from 'node-dotnet-bridge'

const log = function(text: string) { console.log(text) }

const resolveCoreclr = function(basePath: string, dllName: string) {
    const version = fs.readdirSync(basePath).sort((a, b) => - a.localeCompare(b))[0]
    const result = Path.join(basePath + version, dllName)
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
