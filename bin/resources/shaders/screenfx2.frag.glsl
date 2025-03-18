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
float warp1(float x) {
    return sin(x * Pi);
}

float warp12(float x) {
    return 0.5 + 0.5 * sin(x * twoPi);
}

float warp2(float x) {
    return (0.5 + 0.5 * sin(x * Pi));
}

float warp3(float x, bool s, float scale) {
    if (s) {
        return (0.5 * sin(x * twoPi)) * scale;
    } else {
        return (0.5 * cos(x * twoPi)) * scale
    }
}
// NOTE: Render size values should be passed from code
// const float renderWidth = 640.0;
// const float renderHeight = 480.0;
const float renderWidth = 1280.0;
const float renderHeight = 960.0;
const vec2 middle = vec2(0.5);
void main()
{
    float scaleFactor = (95.0 / 100.0);

    vec2 centered = (fragTexCoord - 0.5);
    centered *= 2.0;

    float wx = warp1(fragTexCoord.x);
    float wy = warp1(fragTexCoord.y);
    float warpMult = (wx * wy);
    vec2 w_xy = vec2(wx, wy);

    float dst = distance(fragTexCoord, middle);
    vec2 dir = normalize(fragTexCoord - middle);

    vec2 scaledCoord = centered / scaleFactor;

    vec2 offset = vec2(scaleFactor - 1.0, scaleFactor - 1.0);
    vec2 warpCoord = centered; //(offset + scaledCoord) - (1.0 / (warpMult * 100.0));

    vec2 bounds = offset / 2.0;
    vec4 texelColor = texture2D(texture0, fragTexCoord + dir * 0.015);
    float threshold = .15;
    float mulClamp = warpMult < threshold ? 0.0 : 1.0;
    float invMulClamp = warpMult > threshold ? warpMult : 1.0;
    vec4 screenEdgeMask = vec4(0.35 + (mulClamp * 0.65), 0.35 + (mulClamp * 0.65), 0.35 + (mulClamp * 0.65), 1.0);
    vec4 screenMiddleMask = vec4(
            (invMulClamp * 1.0),
            0.6,
            0.6, 1.0
        );

    bool ct = false;
    bool middle = true;
    // if (ct && centered.x > 1.5) {
    //     gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    // } else if (middle) {
    //     gl_FragColor = texelColor * screenMiddleMask;
    // } else {
    //     gl_FragColor = texelColor * screenEdgeMask;
    // }
    //
    if (ct && centered.x > 1.5) {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (middle) {
        gl_FragColor = texelColor * screenMiddleMask;
    } else {
        gl_FragColor = texelColor * screenEdgeMask;
    }
}
