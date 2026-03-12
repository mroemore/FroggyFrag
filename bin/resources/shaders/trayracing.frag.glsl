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

uniform vec2 u_mouse;

#define TWO_PI 6.28318530718

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

vec3 hsb2rgb( in vec3 c ){
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),
                             6.0)-3.0)-1.0,
                     0.0,
                     1.0 );
    rgb = rgb*rgb*(3.0-2.0*rgb);
    return c.z * mix( vec3(1.0), rgb, c.y);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitud = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitud * noise(st);
        st *= 2.;
        amplitud *= .5;
    }
    return value;
}


vec2 random2(vec2 p) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

float cellular(vec2 p, float offset) {
    vec2 i_st = floor(p);
    vec2 f_st = fract(p);
    float m_dist = 10.;
    //float c_dist = (1.-distance(p, vec2(5.))/10.)*2.;
    
    for (int j=-1; j<=1; j++ ) {
        for (int i=-1; i<=1; i++ ) {
            vec2 neighbor = vec2(float(i),float(j));
            vec2 point = random2(i_st + neighbor);
            //point *= c_dist;
            point = 0.5 + 0.5*sin(6.2831*point + offset);

            vec2 diff = neighbor + point - f_st;
            
            float dist = length(diff);
            if( dist < m_dist ) {
                m_dist = dist;
            }
        }
    }
    return m_dist;
}

float contrast_adjust(vec3 _colour, float _scale){
    return ((_colour.r + _colour.g + _colour.b)/3.  - 0.5) * _scale;

}

void main() {
    vec2 st = gl_FragCoord.xy / windowResolution.xy;
    st.x *= windowResolution.x / windowResolution.y;
    vec2 uv = st;//((st*.5)+1.5);

    vec2 toCenter = vec2(0.5)-st;
    float angle = atan(toCenter.y,toCenter.x);
    float radius = length(toCenter)*2.0;
    float c_dist = 1.-radius;// + (uv.x/50.);
    
    vec2 cell_vec = uv * (5.+(c_dist * 7.));
    float cell = cellular(cell_vec, iTime);
    //float fractal = fbm(uv+cell);
    vec3 c = vec3(cell);
    //angle = smoothstep(0.19, 0.84, cell);
    //float angle_fract = angle / 128.; 
    //float peturbation = sin(iTime/16. + c_dist * TWO_PI)*0.5 + 0.59;
    //float peturbation2 = sin(iTime/13. + (c_dist + angle) * TWO_PI)*0.25 + 0.5;
    //vec3 ac1 = hsb2rgb(vec3(TWO_PI*0.483 + 0.5 * (1.7* angle_fract * TWO_PI),peturbation+2.8*cell - 0.07,.8));
    //vec3 ac2 = hsb2rgb(vec3(TWO_PI*0.492 + peturbation2 * (3.* angle_fract * TWO_PI), .4+fractal,.9));
  
    //ac1 += contrast_adjust(ac1, 1.98);
    //ac2 += contrast_adjust(ac2, 1.3);
	  //c*= ac2 + ac1;
    
    //float fog_mod = exp(c_dist);
    //c.b += fog_mod * .175; 
    //c.g += fog_mod * .105;
    //c.r -= fog_mod * .12;
    //c *= (1.-fog_mod*.135);
    gl_FragColor = vec4(c,1.0);
}
