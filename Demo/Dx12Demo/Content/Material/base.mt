
Property {
    [min = 1.0, max = 2.0]
    float Potato = 1.0;
    [instance_type, min = 2.0, max = 3.0]
    {
        float Data = 1.0
        [DefineType]
        float Data2 = 1.0;
    }
    SamplerState = "sadasd";
    Texture1D = "WTF";
}

Import "$Content:/Other" As Other;

Material "Type" as pp {
    WTF = 1.0;
    Segment VS [Random, GoGo.WTF, WTF] (float2 Position, out float4 Color, out float2 Texture) @{
        Color = PPP(WTF);
    }@;
    Segment PS [Random, GoGo.WTF] (float2 Position, out float3 Color, out float2 WTF) @{
        Color = float3(1.0f, 1.0f, 1.0f);
    }@;
};

Code Name [] @{
    float2 Function(float P)
    {
        return float2(P, P) * TYU(P) + WTF;
    }

    float3 PPP(float WTF)
    {
        return WTF;
    }
}@;

