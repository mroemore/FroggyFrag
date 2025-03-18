// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float iTime;
uniform vec2 windowResolution;

// NOTE: Add your custom variables here
const float renderWidth = 1280.0;
const float renderHeight = 960.0;
const float crtWidth = 320.0;
const float crtHeight = 240.0;

float lum(vec3 rgb) {
    float c_min = min(rgb.r, min(rgb.g, rgb.b));
    float c_max = max(rgb.r, max(rgb.g, rgb.b));
    return (c_min + c_max) * 0.5;
}

void main()
{
    vec2 crtPxSize = vec2(1.0 / crtWidth, 1.0 / crtHeight);
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    // NOTE: Implement here your fragment shader code
    gl_FragColor = texelColor;
}
