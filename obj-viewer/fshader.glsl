#version 330 core

in vec2 UV;
in vec3 vertexPosition_world;
in vec3 vertexNormal_camera;
in vec3 cameraDirection_camera;
in vec3 lightDirection_camera;

uniform mat4 ModelView;
uniform vec3 lightPosition;

uniform vec3 diffuse;
uniform vec3 specular;
uniform vec3 ambient;
uniform float shininess;

void main() {
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	float lightPower = 50.0f;
	
	//vec3 MaterialDiffuseColor = texture( myTextureSampler, UV ).rgb;

	float distance = length(lightPosition - vertexPosition_world);

	vec3 n = normalize(vertexNormal_camera);
	vec3 l = normalize(lightDirection_camera);
	float cosTheta = clamp(dot(n, l), 0, 1);
	
	vec3 E = normalize(cameraDirection_camera);
	vec3 R = reflect(-l, n);
	float cosAlpha = clamp(dot(E, R), 0, 1);
	
	gl_FragColor = vec4((ambient+vec3(0.1,0.1,0.1))*diffuse + diffuse*lightColor*lightPower*cosTheta/(distance*distance) + specular*lightColor*lightPower*pow(cosAlpha,3)/(distance*distance), 1.0);
}
