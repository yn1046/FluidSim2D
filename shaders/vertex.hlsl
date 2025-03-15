struct VSInput
{
    float2 position : POSITION;
};

float4 vs_main(VSInput input) : SV_POSITION
{
    return float4(input.position, 0.0f, 1.0f);
}