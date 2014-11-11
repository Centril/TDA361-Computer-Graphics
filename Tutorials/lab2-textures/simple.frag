#version 130

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

// texture stuff:
uniform sampler2D colortexture;
in vec2	texCoord;

in vec3 outColor;

out vec4 fragmentColor;

void main()
{
	fragmentColor = texture2D(colortexture, texCoord.xy);
}