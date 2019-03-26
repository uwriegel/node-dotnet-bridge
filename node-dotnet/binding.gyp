{
    "targets": [
        {
            "target_name": "node-dotnet",
            "sources": [ "node-dotnet.cpp" ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],            
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


