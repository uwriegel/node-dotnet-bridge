{
    "targets": [
        {
            "target_name": "node-dotnet-bridge",
            "sources": [ 
                "node-dotnet-bridge.cpp",
                "loader.cpp"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],            
            "conditions": [
                ['OS=="win"', {
                     'defines': ['WINDOWS']   
                    }
                ],
                ['OS=="linux"', {        
                        'defines': ['LINUX'] 
                    }
                ]
            ]
        }
    ]
}


