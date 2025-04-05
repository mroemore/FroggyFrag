// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float iTime;
uniform vec2 windowResolution;
uniform vec2 mousePosition;
// NOTE: Add your custom variables here

void main()
{
    // Texel color fetching from texture sampler
    vec2 scaledMousePos = vec2(mousePosition.x / windowResolution.x, mousePosition.y / windowResolution.y);
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    if (distance(scaledMousePos, fragTexCoord) < 0.01) {
        texelColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    // NOTE: Implement here your fragment shader code
    gl_FragColor = texelColor;
}
