declare module 'node-dotnet-bridge' {
    interface CorePathResult {
        path: string
        appPath: string
        dll: string
        tpaList: string
    }
    
    interface Settings {
        logCallback: ((text: string)=>void) | undefined
        deserialize: (json: string)=>any
        resolveCoreclr: (coreBasePath: string, dllName: string)=>CorePathResult
    }

    function initialize(settings: Settings): void
    function unInitialize(): void
    function test(text: string): void

    class ProxyObject {

    }
}

declare var testDate: Date