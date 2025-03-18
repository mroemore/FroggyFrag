varying vec2 fragTexCoord;
varying vec4 fragColor;

float Pi = 3.141592653589793238462643383;
float twoPi = 6.28318530718;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float iTime;

// Output fragment color
//out vec4 finalColor;

// NOTE: Add your custom variables here

// NOTE: Render size values should be passed from code
const float renderWidth = 640.0;
const float renderHeight = 480.0;

float radius = 250.0;
float angle = 0.8;

void main()
{
    vec2 original = fragTexCoord;
    vec2 adjusted = vec2(fragTexCoord.x, fragTexCoord.y);
    float piFactor = Pi;
    //vec2 newCoord = adjusted * piFactor;
    float threshold = 0.1;
    bool coordTest = false;

    float mulr = sin(original.x * piFactor) * sin(original.y * piFactor);
    vec2 mulxy = vec2(1.0 - sin(original.x * piFactor), 1.0 - sin(original.y * piFactor));

    adjusted += .5;
    //adjusted/=2.0;
    mulr *= 0.5;
    //mulxy *= 0.5;

    float mulClamp = mulr < threshold ? 0.0 : 1.0;

    //mulxy *= fragTexCoord;

    vec4 texelColor = texture2D(texture0, fragTexCoord * mulxy);

    // vec2 texSize = vec2(renderWidth, renderHeight);
    // vec2 tc = fragTexCoord*texSize;

    vec4 testColor = vec4(1.0 * mulr, 1.0 * mulr, 0.0, 1.0);
    vec4 screenEdgeMask = vec4(0.35 + (mulClamp * 0.65), 0.35 + (mulClamp * 0.65), 0.35 + (mulClamp * 0.65), 1.0);

    if (coordTest && fragTexCoord.x < 0.1) {
        gl_FragColor = texelColor * testColor;
    } else {
        gl_FragColor = texelColor * screenEdgeMask;
    }
    //gl_FragColor = testColor;
}
