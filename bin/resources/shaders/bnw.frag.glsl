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
vec4 outColour = vec4(0,0,0,1);
void gs_avg(vec4 c){
    float a = (c.r + c.b + c.g)/3.0;
    outColour = vec4(a,a,a,c.a);
}

void gs_hardclip(vec4 c, float bw){
    float a = (c.r + c.b + c.g)/3.0;
    float d = (1.0-bw)/2.0;
    if(a < d){
        outColour = vec4(0,0,0,1);
    } else if(a > 1.0 - d){
        outColour = vec4(1,1,1,1);
    } else {
    a = (a - (d*2.0)) * (1.0 / bw);
    outColour = vec4(a,a,a,c.a);
    }
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 tc = texture2D(texture0, fragTexCoord);

    // NOTE: Implement here your fragment shader code
    //gs_avg(tc);
    gs_hardclip(tc, 0.9);
    gl_FragColor = outColour;
}
