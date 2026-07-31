cbuffer TransformBuffer : register(b0)
{
    float2 objectPosition;
    float2 objectSize;

    float rotation;
    float aspectRatio;
    float2 padding;
};

struct VertexInput
{
    float2 position : POSITION;
    float3 color : COLOR;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;

    // Quad vertices are initially between -0.5 and 0.5.
    float2 localPosition =
        input.position * objectSize;

    const float cosine = cos(rotation);
    const float sine = sin(rotation);

    float2 rotatedPosition;

    rotatedPosition.x =
        localPosition.x * cosine -
        localPosition.y * sine;

    rotatedPosition.y =
        localPosition.x * sine +
        localPosition.y * cosine;

    // Correct horizontal scale so a square remains square
    // on a widescreen window.
    float2 worldPosition =
    rotatedPosition + objectPosition;

worldPosition.x /= aspectRatio;

output.position = float4(
    worldPosition,
    0.0f,
    1.0f
);

    output.color = input.color;

    return output;
}

float4 PSMain(PixelInput input) : SV_TARGET
{
    return float4(
        input.color,
        1.0f
    );
}