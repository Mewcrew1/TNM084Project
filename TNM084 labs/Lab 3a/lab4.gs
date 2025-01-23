#version 410 core

layout(triangles) in;
// Use line_strip for visualization and triangle_strip for solids
layout(triangle_strip, max_vertices = 3) out;
//layout(line_strip, max_vertices = 3) out;
in vec2 teTexCoord[3];
in vec3 teNormal[3];
out vec2 gsTexCoord;
out vec3 gsNormal;
uniform sampler2D tex;

uniform mat4 projMatrix;
uniform mat4 mdlMatrix;
uniform mat4 camMatrix;

uniform float disp;
uniform int texon;
uniform int SphereMaterial;
uniform float stoneID;
uniform float cloudID;

vec2 random2(vec2 st)
{
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
float noise(vec2 st)
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

vec3 random3(vec3 st)
{
    st = vec3( dot(st,vec3(127.1,311.7, 543.21)),
              dot(st,vec3(269.5,183.3, 355.23)),
              dot(st,vec3(846.34,364.45, 123.65)) ); // Haphazard additional numbers by IR
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
// Trivially extended to 3D by Ingemar
float noise(vec3 st)
{
    vec3 i = floor(st);
    vec3 f = fract(st);

    vec3 u = f*f*(3.0-2.0*f);

    return mix(
    			mix( mix( dot( random3(i + vec3(0.0,0.0,0.0) ), f - vec3(0.0,0.0,0.0) ),
                     dot( random3(i + vec3(1.0,0.0,0.0) ), f - vec3(1.0,0.0,0.0) ), u.x),
                mix( dot( random3(i + vec3(0.0,1.0,0.0) ), f - vec3(0.0,1.0,0.0) ),
                     dot( random3(i + vec3(1.0,1.0,0.0) ), f - vec3(1.0,1.0,0.0) ), u.x), u.y),

    			mix( mix( dot( random3(i + vec3(0.0,0.0,1.0) ), f - vec3(0.0,0.0,1.0) ),
                     dot( random3(i + vec3(1.0,0.0,1.0) ), f - vec3(1.0,0.0,1.0) ), u.x),
                mix( dot( random3(i + vec3(0.0,1.0,1.0) ), f - vec3(0.0,1.0,1.0) ),
                     dot( random3(i + vec3(1.0,1.0,1.0) ), f - vec3(1.0,1.0,1.0) ), u.x), u.y), u.z

          	);
}

float smoothVoronoi(vec3 x) {
    vec3 p = floor(x);   // Cell origin
    vec3 f = fract(x);   // Fractional part

    float res = 0.0;
    for (int k = -1; k <= 1; k++) {
        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                vec3 b = vec3(i, j, k);           // Neighbor offset
                vec3 r = (b - f) + random3(p + b); // Relative position
                float d = length(r);             // Distance
                res += exp2(-32.0 * d);          // Exponential smoothing
            }
        }
    }
    return -(1.0/32.0)*log2( res ); // Normalize result
}

float fbm(vec3 pos, float scale, float stoneID){
    float t = 0.0;
    float G = exp(-scale);
    float f = 1.0;
    float a = 1.0;
    int round = int(floor(4.0 * stoneID));

    int numOctaves = 4 + round;
    for(int i = 0; i < numOctaves; i++){
        t += a * smoothVoronoi(pos * f);
        f *= 1.5;
        a*= G;
    }
    return t;
}

float fbmNoise(vec3 pos, float scale, float cloudID){
    float t = 0.0;
    float G = 0.5;
    float f = 1.0;
    float a = 1.0;
    int round = int(floor(3.0 * cloudID));
    int numOctaves = 3 + round;
    for(int i = 0; i < numOctaves; i++){
        t += a * noise(pos * f);
        f *= 2.0;
        a*= G;
    }
    return t;
}
void computeVertex(int nr)
{
	vec3 p, v1, v2, v3, p1, p2, p3, s1, s2, n;

	p = vec3(gl_in[nr].gl_Position);
	// Add interesting code here

	vec3 dir = p - vec3(0,0,0);
	vec3 normvec = normalize(dir) * 0.8;

	vec3 ortvec1 = cross(normvec, vec3(1,0,0));
	vec3 ortvec2 = cross(normvec, ortvec1);
	//Kanske fel
	vec3 ortvec3 = 0.5 * (-ortvec1 - ortvec2);
	float stepsize = 0.25;
	ortvec1 *= stepsize;
	ortvec2 *= stepsize;
	ortvec3 *= stepsize;
	vec3 ortpunkt1 = p + ortvec1;
    vec3 ortpunkt2 = p + ortvec2;
    vec3 ortpunkt3 = p + ortvec3;
    ortpunkt1 = normalize(ortpunkt1);
    ortpunkt2 = normalize(ortpunkt2);
    ortpunkt3 = normalize(ortpunkt3);

	p = normvec;

	//p = p + noise(p*4)/9;


    if(SphereMaterial == 0){
        float scale = 0.2 + 0.8 * stoneID;
        p = p + p * fbm(p ,scale, stoneID) / 1.5;
        //p = p + p * fbm(p, 1) * 3;
        int round = int(floor(4.0 * stoneID));
        int numOctaves = 4 + round;
        ortpunkt1 = ortpunkt1 + ortpunkt1 * fbm(ortpunkt1,scale, stoneID)/numOctaves;
        ortpunkt2 = ortpunkt2 + ortpunkt2 * fbm(ortpunkt2,scale, stoneID)/numOctaves;
        ortpunkt3 = ortpunkt3 + ortpunkt3 * fbm(ortpunkt3,scale, stoneID)/numOctaves;


	}

	else{
        float scale = 0.3 + 0.6*cloudID;
        p = p + p * fbmNoise(normvec , scale, cloudID);
        int round = int(floor(3.0 * cloudID));
        int numOctaves = 3 + round;
        ortpunkt1 = ortpunkt1 + ortpunkt1 * fbm(ortpunkt1,scale, cloudID)/numOctaves;
        ortpunkt2 = ortpunkt2 + ortpunkt2 * fbm(ortpunkt2,scale, cloudID)/numOctaves;
        ortpunkt3 = ortpunkt3 + ortpunkt3 * fbm(ortpunkt3,scale, cloudID)/numOctaves;
	}


    vec3 ortpunktvec1 = ortpunkt2 - ortpunkt1;
	vec3 ortpunktvec2 = ortpunkt3 - ortpunkt2;
	vec3 normal = cross(ortpunktvec1,ortpunktvec2);
    normal = normalize(normal);

	gl_Position = projMatrix * camMatrix * mdlMatrix * vec4(p, 1.0);
	//gl_Position = projMatrix * mdlMatrix * vec4(p, 1.0);

    gsTexCoord = teTexCoord[0];

	n = teNormal[nr]; // This is not the normal you are looking for. Move along!
    gsNormal = mat3(mdlMatrix) * normal;

    //gsNormal = normal;

    EmitVertex();
}

void main()
{
	computeVertex(0);
	computeVertex(1);
	computeVertex(2);
}

