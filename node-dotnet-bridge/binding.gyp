{
    "targets": [
        {
            "target_name": "node-dotnet-bridge",
            "sources": [ 
                "addon.cpp",
                "proxy.cpp"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],            
            "cflags": ["-Wall", "-std=c++14"],
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


