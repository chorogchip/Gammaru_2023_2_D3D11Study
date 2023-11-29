cbuffer VSConstBufferPerFrame : register(b0)
{
    float4x4 MatWorld;
    float4x4 MatView;
};

cbuffer VSConstBufferPerResize : register(b1)
{
    float4x4 MatProj;
};

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
    output.Pos = float4(input.Pos, 1.0f);
    output.Pos = mul(MatWorld, output.Pos);
    output.Pos = mul(MatView, output.Pos);
    output.Pos = mul(MatProj, output.Pos);
    output.UV = input.UV;
    
    return output;
}

float4 PS(VS_Output input) : SV_Target
{
    return float4(input.UV, 1.0f, 1.0f);
}
