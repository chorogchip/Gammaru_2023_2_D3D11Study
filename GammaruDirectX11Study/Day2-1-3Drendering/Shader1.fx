
struct VS_Input
{
    float3 Pos : POSITION;
    float2 UV : TEXCOORD;
};

struct VS_Output
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
};

VS_Output VS(VS_Input input)
{
    VS_Output output;
    output.Pos = float4(input.Pos * 0.25f + 0.5f, 1.0f);
    output.UV = input.UV;
    return output;
}

float4 PS(VS_Output input) : SV_Target
{
    return float4(input.UV, 1.0f, 1.0f);
}
