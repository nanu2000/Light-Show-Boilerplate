#version 330 core
out vec4 color;


//**********************************************
//These macros are modified on runtime. 
//the /*TAG*/ before the value tells the 
//compiler what value needs to be set and where.
//***********************************************

#define AMOUNT_OF_POINT_LIGHTS	/*MAL*/4//
#define SHADOW_INTENSITY		/*SI*/5//
#define SHADOW_FILTERING		/*SF*/5//
#define SHADOW_FILTER_DISTANCE  /*SFD*/1.2//

const float shadowFade = .5;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
    float shininess;
}; 
struct DirectionalLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

  

uniform PointLight pointLights[AMOUNT_OF_POINT_LIGHTS];

uniform DirectionalLight directionalLight;



uniform Material	material;
uniform samplerCube	pointShadowMap;  
uniform sampler2D	directionalShadowMap;  
uniform float		farPlane;
uniform vec3		viewPosition;
uniform vec3		lightPosition;

in vec4 fragmentPositionLightSpace_o;
in vec2 textureCoords_o;
in vec3 normal_o;
in vec3 fragPosition_o; 

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1),
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1)
);


float fadeShadowOverDistance(vec2 shadowCoord)
{
	vec4 coord = vec4(shadowCoord, 1. - shadowCoord);
	vec4 mu = clamp(coord / shadowFade, 0., 1.);
	vec2 mu2 = min(mu.xy, mu.zw);
	return min(mu2.x, mu2.y);
}


float directionalShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords		= (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5f + 0.5f;
    float bias			= max(0.002f * (1.0f - dot(normalize(normal_o), normalize(directionalLight.direction))), 0.005f);  
    float shadow		= 0.0f;
	vec2 textureSize	= textureSize(directionalShadowMap, 0);

	vec2 filterAmount = ((1.0 / textureSize) / 1 * (SHADOW_FILTER_DISTANCE * (textureSize / 2048)));

    for(int i = 0; i < SHADOW_FILTERING; ++i)
    {
            float pcfDepth = 
			texture(directionalShadowMap, projCoords.xy + sampleOffsetDirections[i].xy * filterAmount).r; 
            
			shadow += projCoords.z - bias > pcfDepth  ? 1.0f : 0.0f;        
    }    
  
	shadow *= fadeShadowOverDistance(projCoords.xy);
	
    shadow /= float(SHADOW_FILTERING + SHADOW_INTENSITY * 2);
    if(projCoords.z > 1.0f)
	{
        shadow = 0.0f;
    }
	                
    return shadow;
	
}

float pointLightShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight	= fragPos - lightPosition;
    float currentDepth	= length(fragToLight);
    float shadow		= 0.0;
    float bias			= 0.15;
    float viewDistance	= length(viewPosition - fragPos);
    float diskRadius	= (1.0 + (viewDistance / farPlane)) / 25.0;

    for(int i = 0; i < SHADOW_FILTERING; ++i)
    {
        float closestDepth = texture(pointShadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= farPlane;

        if(currentDepth - bias > closestDepth)
		{
            shadow += 1.0;
		}
	}
    shadow /= float(SHADOW_FILTERING + SHADOW_INTENSITY);
                
    return shadow;
} 

void main()
{
    // Properties
    vec3 norm		= normalize(normal_o);
    vec3 viewDir	= normalize(viewPosition - fragPosition_o);

    // Phase 1: Directional lighting
    vec3 result = CalcDirLight(directionalLight, norm, viewDir);

    // Phase 2: Point lights
    for(int i = 0; i < AMOUNT_OF_POINT_LIGHTS; i++)
	{
       result += CalcPointLight(pointLights[i], norm, fragPosition_o, viewDir);    
	}


    color = vec4(result, 1.0);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, textureCoords_o));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, textureCoords_o));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, textureCoords_o));
			

    return (ambient +(1.0f - directionalShadowCalculation(fragmentPositionLightSpace_o)) * (diffuse + specular));

}  

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, textureCoords_o));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, textureCoords_o));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, textureCoords_o));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
	
    return (ambient +(1.0f - pointLightShadowCalculation(fragPosition_o)) * (diffuse + specular));

} 
