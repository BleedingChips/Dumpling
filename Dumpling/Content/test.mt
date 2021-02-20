property{
    [min = float2(1.0, 2.0), max = float2(2.0, 2.0)]
    {
        float2 Data;
        float P = 1.0;
        [InsOnly, dddd = 1.0]
        float K = 1.0;
    }

    Texture2D<float2> data;
    SamplerState SS = "dsadasd";
};

import "Context\As" as Mask;

code Text [Mask.asd]@{

float2 Random(float2 In) { return In; } 

}@;

[mateicd = "sdasdasdasd"]
material "sadasd" as WTF {
    float2 wyf = 2.0;
    snippet VS [Text, Mask.asd](in float2 WTF, out float2 WTF2, float3 WTF3) @{
    }@;
    float2 tyu = 2.0;
};

[mateicd = "sdasdasdasd"]
material "sadasd" as WTF {
    float2 wyf = 2.0;
    snippet VS [Text, Mask.asd](in float2 WTF, out float2 WTF2, float3 WTF3) @{
    }@;
    float2 tyu = 2.0;
};