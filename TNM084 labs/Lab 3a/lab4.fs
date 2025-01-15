#version 410 core

out vec4 out_Color;
in vec2 gsTexCoord;
in vec3 gsNormal;
uniform sampler2D tex;

void main(void)
{	

	vec4 t = texture(tex,gsTexCoord);
	float shade = normalize(gsNormal).z; // Fake light
	out_Color = t * (shade+0.7);
	out_Color.a = 1.0; 
	//out_Color = vec4(gsTexCoord.s, gsTexCoord.t, 0.0, 1.0);
	//out_Color = vec4(gsNormal.x, gsNormal.y, gsNormal.z, 1.0);
	//out_Color = vec4(shade, shade, shade, 1.0);

	//fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Bright red

	


}

