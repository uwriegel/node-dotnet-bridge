declare module 'node-dotnet-bridge' {
    interface CorePathResult {
        path: string
        appPath: string
        dll: string
        tpaList: string
    }
    
    interface Settings {
        module: string, 
        logCallback: ((text: string)=>void) | undefined
        deserialize: (json: string)=>any
        resolveCoreclr: (coreBasePath: string, dllName: string)=>CorePathResult
    }

    function initialize(settings: Settings): void
    function unInitialize(): void
    function executeAsync(id: number, name: string, callback: (result: string)=>void, text: string, nm: number, date: Date): void
    function runEventLoop(callback: ()=>void, onEvent: (evt: string)=>void): void
    
    class ProxyObject {
        constructor(module: string)
        executeSync(payload: string): string
        execute2Sync(payload: string): string
    }
}

declare var testDate: Date
declare var welt: string
