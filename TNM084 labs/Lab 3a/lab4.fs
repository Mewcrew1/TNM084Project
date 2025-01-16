#version 410 core

out vec4 out_Color;
in vec2 gsTexCoord;
in vec3 gsNormal;
uniform sampler2D tex;

uniform int SphereMaterial;
uniform int TessLevelInner; // Sent from main program
uniform int TessLevelOuter1;
uniform int TessLevelOuter2;
uniform int TessLevelOuter3;

void main(void)
{
    //Stone
    if(SphereMaterial == 0){
	float shade = normalize(gsNormal).z * 0.8; // Fake light
	//out_Color = vec4(gsTexCoord.s, gsTexCoord.t, 0.0, 1.0);
	//out_Color = vec4(gsNormal.x, gsNormal.y, gsNormal.z, 1.0);
	//
	//out_Color = vec4(shade, shade, shade, 1.0);

	vec4 t = texture(tex,gsTexCoord);

	out_Color = t * (shade + 0.7);
	out_Color.a = 1.0;
	//out_Color = vec4(TessLevelInner / 15.0 , TessLevelOuter1 / 15.0, 0.0, 1.0);

	}
	//Cloud
	else{
	out_Color = vec4(1, 1, 1, 0.70); // Bright red
	}
}

