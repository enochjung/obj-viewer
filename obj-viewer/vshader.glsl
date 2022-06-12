#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

out vec2 UV;
out vec3 vertexPosition_world;
out vec3 vertexNormal_camera;
out vec3 cameraDirection_camera;
out vec3 lightDirection_camera;

uniform mat4 ModelView;
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 lightPosition;

void main() { 
	gl_Position = Projection * ModelView * vec4(vertexPosition, 1.0);

	vertexPosition_world = (Model * vec4(vertexPosition, 1.0)).xyz;

	vec3 vertexPosition_camera = (ModelView * vec4(vertexPosition, 1.0)).xyz;
	cameraDirection_camera = vec3(0.0, 0.0, 0.0) - vertexPosition_camera;

	vec3 lightPosition_camera = (View * vec4(lightPosition, 1.0)).xyz;
	lightDirection_camera = lightPosition_camera + cameraDirection_camera;
	
	vertexNormal_camera = (ModelView * vec4(vertexNormal, 0.0)).xyz;
	
	UV = vertexUV;
}
