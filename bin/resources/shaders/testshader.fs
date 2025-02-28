in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here

// NOTE: Render size values should be passed from code
const float renderWidth = 640;
const float renderHeight = 480;

float radius = 250.0;
float angle = 0.8;

uniform vec2 center = vec2(200.0, 200.0);

void main()
{
    vec2 texSize = vec2(renderWidth, renderHeight);
    vec2 tc = fragTexCoord*texSize;

    fragColor.z *= 0.5;
    finalColor = fragColor;
}