import * as fs from 'fs'
import * as Path from 'path'
import { initialize, unInitialize } from 'node-dotnet-bridge'

const log = function(text: string) { console.log(text) }

const resolveCoreclr = function(basePath: string, dllName: string) {
    const version = fs.readdirSync(basePath).sort((a, b) => - a.localeCompare(b))[0]
    const path = basePath + version
    const dllPath = Path.join(path, dllName)
    console.log("Loading dotnet core from " + dllPath)


    console.log(__dirname)



    return {
        path: path,
        dll: dllPath,
        assemblies: [] as string[]
    }
}

// TODO: logCallback: kann auch null sein, dann kein Logging
initialize({
    logCallback: log,
    resolveCoreclr: resolveCoreclr
})

unInitialize()
console.log("Finished")
