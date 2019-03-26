declare module 'node-dotnet' {
    function initialize(callback: (text: string)=>void): void
    function testLogging(text: string): void
    function unInitialize(): void
}