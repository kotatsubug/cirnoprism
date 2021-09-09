#version 440

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	sampler2D diffuseTex;
	sampler2D specularTex;
	sampler2D emissiveTex;
};

struct PointLight
{
	vec3 position;
	vec3 color;
	float intensity;

	float constant, linear, quadratic;
};

in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;

in vec4 fragPosLight;

out vec4 fs_color;

uniform Material material;
uniform PointLight pointLight;
uniform vec3 camPosition;

uniform sampler2D shadowMap;

// Functions
vec3 CalculateAmbient(Material material)
{
	return material.ambient;
}

vec3 CalculateDiffuse(Material material, vec3 vs_position, vec3 vs_normal, vec3 lightPos)
{
	vec3 posToLightDirVec = normalize(lightPos - vs_position);
	float diffuse = clamp(dot(posToLightDirVec, normalize(vs_normal)), 0, 1);

	return (material.diffuse * diffuse);
}

vec3 CalculateSpecular(Material material, vec3 vs_position, vec3 vs_normal, vec3 lightPos, vec3 camPosition)
{
	vec3 lightToPosDirVec = normalize(vs_position - lightPos);
	vec3 reflectDirVec = normalize(reflect(lightToPosDirVec, normalize(vs_normal)));
	vec3 posToViewDirVec = normalize(camPosition - vs_position);
	float specularConstant = pow(max(dot(posToViewDirVec, reflectDirVec), 0), 30);

	return (material.specular * specularConstant * texture(material.specularTex, vs_texcoord).rgb);
}

float CalculateShadow(vec4 fragPosLight)
{
	vec3 projectionCoords = fragPosLight.xyz / fragPosLight.w;

	// Map to [0,1]
	projectionCoords = projectionCoords * 0.5 + 0.5;

	// Get the closest depth value from the light's perspective in [0,1] fragPosLight
	float closestDepth = texture(shadowMap, projectionCoords.xy).r;
	// and get depth of current fragment from the light's perspective
	float currentDepth = projectionCoords.z;

	// Bias calculation to reduce shadow acne
	float bias = max(0.05 * (1.0 - dot(normalize(vs_normal), normalize(pointLight.position - vs_position))), 0.005);

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projectionCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0; 
		}
	}
	shadow /= 9.0;
	if (projectionCoords.z > 1.0)
	{
		shadow = 0.0;
	}

	return shadow;
}

// Scatters light (uniformly) similar to car headlights on a foggy day but without raymarching,
// instead by summing total light that hits the camera using inverse square distance as magnitude in an antiderivative
float FastScatter(in vec3 lightPos)
{
	// Method by https://ijdykeman.github.io/graphics/simple_fog_shader

	const vec3 w = vs_position;
	const vec3 c = camPosition;
	const vec3 L = lightPos;

	// Project to a 2D plane where the x-axis is the line from the camera to the world fragment position,
	// and the y-axis is the direction from the light source to the x-axis

	const vec3 xDir = ((w - c) / distance(w, c));
		vec3 P = c + (xDir * dot(xDir, L - c)); // P is the point on line CW closest to L
		// Now, make sure P lies on segment CW. If it doesn't, choose nearest point W or C.
		// If this were a point-to-an-infinite-line calculation, this part below could have been removed
		const float D_wP = distance(w, P);
		const float D_cP = distance(c, P);
		const float D_cw = distance(c, w);
		if (D_wP > D_cw || D_cP > D_cw)
		{
			if (D_cP > D_wP)
			{
				P = w;
			}
			else
			{
				P = c;
			}
		}
	const vec3 yDir = ((P - L) / distance(P, L));

	// Get transformed 2D coordinates from the projection of 3D coordinates onto the new plane
	// Treat camera position as the origin of this new plane, X and Y directions were inferred above
	// TODO: shouldn't this be dot(xDir, c-c) and dot(xDir,w)?????
	const float c_tx = dot(xDir, c); // y's not needed here
	const float w_tx = dot(xDir, w);
	const float L_ty = dot(yDir, L - c); // x's not needed here
	const float P_ty = dot(yDir, P - c);

	// Finally, integrate the distance sqrt(x^2 + h^2) as ./'c->w 1/(h^2 + x^2)dx on the new x-axis,
	// summing up all the "fragments" of light as 1/d^2 for an illuminated fog effect
	const float h = abs(P_ty - L_ty);
	const float integral = (1.0/h)*(atan(w_tx/h) - atan(c_tx/h));

	// TODO: Only perform these calculations if w is within bounds of a "light sphere" radius to save performance
	return integral;
}

void main()
{
	vec3 ambientFinal = CalculateAmbient(material);
	vec3 diffuseFinal = CalculateDiffuse(material, vs_position, vs_normal, pointLight.position);
//	vec3 specularFinal = CalculateSpecular(material, vs_position, vs_normal, pointLight.position, camPosition);

	// Attenuation
	float distance = length(pointLight.position - vs_position);
	float attenuation = pointLight.constant / (1.0 + pointLight.linear * distance + pointLight.quadratic * (distance * distance)); // Optimize this

	ambientFinal *= attenuation;
//	diffuseFinal *= attenuation;
//	specularFinal *= attenuation;

//	float shadow = CalculateShadow(fragPosLight);
	float shadow = 0.0;
	
	vec4 difTexColor = texture(material.diffuseTex, vs_texcoord);
	if (difTexColor.a < 0.1)
	{
		discard;
	}

	vec4 finalTexColor = difTexColor * (vec4(ambientFinal, 1.0) + ((vec4((1.0 - shadow) * diffuseFinal, 1.0)))) * vec4(vs_color, 1.0);
	vec4 finalEmissiveTexColor = 1.3 * (texture(material.diffuseTex, vs_texcoord) * texture(material.emissiveTex, vs_texcoord));

	fs_color = vec4(vec3(FastScatter(pointLight.position)), 1.0) + mix(finalTexColor, finalEmissiveTexColor, 0.5); 
//	fs_color = texture(material.diffuseTex, vs_texcoord) * (vec4(ambientFinal, 1.0) + ((vec4((1.0 - shadow) * diffuseFinal, 1.0) + vec4((1.0 - shadow) * specularFinal, 1.0)))) * vec4(vs_color, 1.0);
//	fs_color = vec4((1.0 - shadow) * material.diffuse, 1.0) * vec4(vs_color, 1.0);
	
}

