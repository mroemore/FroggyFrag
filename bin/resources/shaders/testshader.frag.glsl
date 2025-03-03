varying vec2 fragTexCoord;
varying vec4 fragColor;


// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float gTime;

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
    vec2 newCoord = vec2(fragTexCoord.x, fragTexCoord.y + sin((gTime*4.0 + fragTexCoord.x*64.0))*0.005);
    
    vec4 texelColor = texture2D(texture0, newCoord)*fragColor;
    vec2 texSize = vec2(renderWidth, renderHeight);
    vec2 tc = fragTexCoord*texSize;
    float div = 4.0;
    vec4 finalColor = texelColor;
    if((mod(floor(tc.x)/div, 2.0) == 0.0 && mod(floor(tc.y)/div, 2.0) == 1.0)
        || (mod(floor(tc.y)/div, 2.0) == 0.0 && mod(floor(tc.x)/div, 2.0) == 1.0)){
        finalColor.x = finalColor.x * 0.5;
        finalColor.y = finalColor.y * 0.5;
        finalColor.z = finalColor.z * 0.5;
    }
    
    gl_FragColor = finalColor;
}