struct Particle
{
    float2 position;
    float2 velocity;
    float density;
    float pressure;
};

RWStructuredBuffer<Particle> particles : register(u0);

[numthreads(256, 1, 1)]
void cs_main(uint3 id : SV_DispatchThreadID)
{
    // “вой код Compute Shader
}