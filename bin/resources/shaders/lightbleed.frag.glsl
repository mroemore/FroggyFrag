// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float iTime;
uniform vec2 windowResolution;

// NOTE: Add your custom variables here
const float renderWidth = 640.0;
const float renderHeight = 480.0;
int radius = 3;

float lum(vec3 rgb) {
    float c_min = min(rgb.r, min(rgb.g, rgb.b));
    float c_max = max(rgb.r, max(rgb.g, rgb.b));
    return (c_min + c_max) * 0.5;
}

vec4 get_adj_colour_sum(sampler2D tex, vec2 tc, vec2 pxSize, float lumthreshold, float time) {
    vec4 source = texture2D(tex, tc);
    vec4 outVec = source;
    vec2 tmp;
    float flicker = 0.5 + 0.5 * sin(time * 30.5);
    flicker *= 0.07;
    lumthreshold += flicker;
    float count = 0.0;
    for (int i = -radius; i < radius; i++) {
        for (int j = -radius; j < radius; j++) {
            tmp = vec2(tc.x + float(i) * pxSize.x, tc.y + float(j) * pxSize.y);
            vec4 adjacentPx = texture2D(tex, tmp);
            float l = lum(adjacentPx.xyz);
            l = smoothstep(0.3, 0.5, l);
            //count += 0.5 + l;
            outVec.xyz += adjacentPx.xyz * ((float(abs(i) + abs(j)) / float(radius * 2)) * (l)); //* (flicker + .75) ;
        }
    }

    outVec.xyz /= float(radius * radius);
    return outVec + source;
}

void main()
{
    // Texel color fetching from texture sampler
    vec2 pxSize = vec2(1.0 / windowResolution.x, 1.0 / windowResolution.y);
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    // NOTE: Implement here your fragment shader code
    gl_FragColor = get_adj_colour_sum(texture0, fragTexCoord, pxSize, 0.33, iTime);
}
