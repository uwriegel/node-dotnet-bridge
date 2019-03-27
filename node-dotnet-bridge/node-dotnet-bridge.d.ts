// declare module 'node-dotnet-bridge' {
//     function initialize(callback: (text: string)=>void): void
//     function testLogging(text: string): void
//     function unInitialize(): void
//     var status: string
// }

declare module 'node-dotnet-bridge' {
    interface Settings {
        logCallback: (text: string)=>void
    }

    function initialize(settings: Settings): void
    function testLogging(text: string): void
    function unInitialize(): void
    var status: string
}