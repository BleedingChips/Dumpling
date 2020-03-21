Import "Context\As" As Mask;

Define Text []@{

}@;

Pass Defualt {
    Result = true;
    Define RTX []@{
        cbuffer WTF{
            float Data21;
        };
        Texture2D Da;
    }@;

    VS [RTX] (float2 Postion, float2 GoGo) -> (float4 SV_POSITION) @{
        SV_POSITION = flaot4(Position, 0.0, 0.0);
    }@;

    PS [RTX] (float4 SV_POSITION) -> (float3 SV_POSITION) @{
        SV_POSITION = SV_POSITION.xyz;
    }@;
};