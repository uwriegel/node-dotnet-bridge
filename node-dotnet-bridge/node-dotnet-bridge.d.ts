declare module 'node-dotnet-bridge' {
    interface Settings {
        logCallback: ((text: string)=>void) | undefined
        resolveCoreclr: (corePath: string, dllName: string)=>string 
    }

    function initialize(settings: Settings): void
    function unInitialize(): void
}