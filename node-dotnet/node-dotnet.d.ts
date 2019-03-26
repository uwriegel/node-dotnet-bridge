declare module 'node-dotnet' {
    function initialize(callback: (text: string)=>void): void
    function testLogging(text: string): void
    function unInitialize(): void
    function hello8(text: string): string
    function hello16(text: string): string
}