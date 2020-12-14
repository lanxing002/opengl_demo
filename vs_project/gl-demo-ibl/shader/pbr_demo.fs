#version 330 core
out vec4 FragColor;

in vec2 tex_coords;
in vec3 world_pos;
in vec3 normal;

uniform vec3 camera_pos;
//uniform samplerCube texture_env;

//material parametes
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

//lights 
uniform vec3 light_positions[4];
uniform vec3 light_colors[4];

const float PI = 3.14159265359;

vec3 fresnel_Schlick(float cos_theta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}

float distribution_GGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometry_SchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float geometry_Smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometry_SchlickGGX(NdotV, roughness);
    float ggx1  = geometry_SchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}  

void main()
{             
    vec3 I = normalize(world_pos - camera_pos);
    vec3 fle_dire = reflect(I, normalize(normal));

    float ratio = 1.00 / 1.52;
    vec3 fra_dire = refract(I, normalize(normal), ratio);
    //vec3 TexCoords2 = vec3(rotate * vec4(TexCoords, 1.0));

    vec3 N = normalize(normal);
    vec3 V = normalize(camera_pos - world_pos);
    vec3 R = reflect(-V, N); 

    vec3 F0 = vec3(0.04); //基础反射率
    F0 = mix(F0, albedo, metallic); //插值之后介于金属与非金属的反射率

    //pbr着色方程的实现
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        vec3 L = normalize(light_positions[i] - world_pos);
        vec3 H = normalize(V + L);
        float distance = length(light_positions[i] - world_pos); //光源与fragment的距离，用于计算光强衰减
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light_colors[i] * attenuation;

        //CooK-Toorrance
        float NDF = distribution_GGX(N, H, roughness);
        float G = geometry_Smith(N, V, L, roughness);
        vec3 F = fresnel_Schlick(clamp(dot(H, V), 0.0, 1.0), F0);

        //F = vec3(0.8, 0.8, 0.8);
        vec3 nominator    = NDF * G * F; //分子项
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0); //分母项
        vec3 specular = nominator / max(denominator, 0.001);

        vec3 kS = F; //镜面反射系数
        vec3 kD = vec3(1.0) - kS; //能量守恒
        kD *= 1.0 - metallic; //金属没有漫反射
        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL; //加法实现积分
        //Lo += (specular) * radiance * NdotL;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

     vec3 ambient = (kD * diffuse + specular) * ao;
     vec3 color = ambient + Lo;

    //hdr映射
    color = color / (color + vec3(1.0));
    //gamma矫正
    color = pow(color, vec3(1.0 / 2.2));
    vec3 tmp_color = texture(prefilterMap, R).rgb;
    FragColor = vec4(color, 1.0);
    //FragColor = vec4(color, 1.0);
}