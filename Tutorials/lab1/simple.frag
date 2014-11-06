#version 130

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

out vec4 fragmentColor;

in vec3 outColor;

void main() 
{
	fragmentColor.rbg = outColor;
}