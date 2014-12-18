#version 130

in vec3	position;
in vec3	colorIn;
in vec2	texCoordIn;	// incoming texcoord from the texcoord array
in vec3	normalIn;

out vec3 viewSpacePosition; 
out vec3 viewSpaceNormal; 
out vec3 viewSpaceLightPosition; 
out vec4 color;
out	vec2 texCoord;	// outgoing interpolated texcoord to fragshader

uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 normalMatrix;

uniform vec3 lightpos; 
uniform mat4 lightMatrix;
out vec4 shadowMapCoord;

void main() 
{
	color = vec4(colorIn,1); 
	texCoord = texCoordIn; 
	viewSpacePosition = vec3(modelViewMatrix * vec4(position, 1)); 
	viewSpaceNormal = vec3(normalize( (normalMatrix * vec4(normalIn,0.0)).xyz ));
	viewSpaceLightPosition = (modelViewMatrix * vec4(lightpos, 1)).xyz; 
	gl_Position = modelViewProjectionMatrix * vec4(position,1);

	shadowMapCoord = lightMatrix * vec4(viewSpacePosition, 1.0);
}
