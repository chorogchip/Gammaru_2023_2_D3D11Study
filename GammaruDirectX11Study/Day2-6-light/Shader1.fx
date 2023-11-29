
cbuffer VSConstBufferPerObject : register(b2)
{
    float4x4 MatWorld;
};

cbuffer VSConstBufferPerFrame : register(b0)
{
    float4x4 MatView;
    float3 CamPos;
};

cbuffer VSConstBufferPerResize : register(b1)
{
    float4x4 MatProj;
};

cbuffer VSConstBufferPerScene : register(b3)
{
    float3 LightPos;
    float3 LightColor;
}

struct VS_Input
{
    float3 Pos : POSITION;
    float2 UV : TEXCOORD;
    float3 Norm : NORMAL;
};

struct VS_Output
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
    float3 Norm : NORMAL;
    float3 Diffuse : DIFFUSE;
    float3 WorldPos : POSITION;
};

VS_Output VS(VS_Input input)
{
    VS_Output output;
    output.Pos = float4(input.Pos, 1.0f);
    output.Pos = mul(MatWorld, output.Pos);
    output.WorldPos = output.Pos.xyz;
    
    float3 lightVec = LightPos - output.WorldPos;
    float lightDist = length(lightVec);
    float lightIntensity = saturate((5.0f - lightDist) / (5.0f - 2.0f));
    
    output.Pos = mul(MatView, output.Pos);
    output.Pos = mul(MatProj, output.Pos);
    output.UV = input.UV;
    output.Norm = input.Norm;
    output.Diffuse = LightColor * lightIntensity * max(0, dot(normalize(lightVec), input.Norm));
    
    return output;
}

Texture2D texture1 : register(t0);
SamplerState samplerPoint : register(s0);

float4 PS(VS_Output input) : SV_Target
{
    float3 lightVec = input.WorldPos - LightPos;
    float lightDist = length(lightVec);
    float lightIntensity = saturate((10.0f - lightDist) / (10.0f - 2.0f));
    float3 reflected = reflect(lightVec, normalize(input.Norm));
    
    float3 specular = LightColor * lightIntensity * pow(max(0,
        dot(normalize(reflected), normalize(CamPos - input.WorldPos))), 16.0f);
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    
    float4 sampledColor = texture1.Sample(samplerPoint, input.UV);
    float3 color = (ambient + input.Diffuse) * sampledColor.xyz + specular;
    
    return float4(color, 1.0f);
}
