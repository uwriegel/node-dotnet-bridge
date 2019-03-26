{
    "targets": [
        {
            "target_name": "node-dotnet",
            "sources": [ "node-dotnet.cpp" ],
            "conditions": [
                ['OS=="win"', {
                     'defines': ['WINDOWS']   
                    }
                ],
                ['OS=="linux"', {        
                    }
                ]
            ]
        }
    ]
}


