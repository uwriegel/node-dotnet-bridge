declare module 'node-dotnet-bridge' {
    interface CorePathResult {
        path: string
        dll: string
        // TODO: Path to assemblies
        assemblies: string[]
    }
    
    interface Settings {
        logCallback: ((text: string)=>void) | undefined
        resolveCoreclr: (coreBasePath: string, dllName: string)=>CorePathResult
    }

    function initialize(settings: Settings): void
    function unInitialize(): void
}