// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float iTime;
uniform vec2 windowResolution;
// NOTE: Add your custom variables here

float osSin(float x) {
    return 0.5 + 0.5 * sin(x);
}

float warp(float coordinate, float time, float scale, float speed, float amp) {
    return osSin((coordinate * scale) + time * speed) * amp;
}

float warpSum(float coordinate, float time, float scale, float speed, float amp, float scaleInc, float timeInc) {
    return warp(coordinate, time, scale, speed, amp)
        + warp(coordinate, time * timeInc, scale + scaleInc, speed, amp)
        - warp(coordinate, time * timeInc * 2.0, scale + scaleInc * 2.0, speed, amp);
}

void main()
{
    vec2 center = vec2(0.5);
    float d = distance(fragTexCoord, center);
    float d3 = d * 3.0;
    float ds = 0.01 + (d3 * d3 * d3);
    float ds2 = 0.07 * (d3 * d3);
    vec2 modTexCoord = vec2(
            fragTexCoord.x + warpSum(fragTexCoord.y, iTime, 2.0, 2.0, 0.003, 18.0, 1.35) * ds,
            fragTexCoord.y + warpSum(fragTexCoord.x, iTime, 48.0, 3.0, 0.003, 32.0, 1.5) * ds
        );
    // Texel color fetching from texture sampler
    vec4 texelColor = texture2D(texture0, modTexCoord);
    modTexCoord *= 0.5;
    texelColor.xyz -= ds2;
    // NOTE: Implement here your fragment shader code
    gl_FragColor = texelColor;
}
