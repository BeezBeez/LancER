#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// 
layout (location = 0) in vec4 fPosition;
layout (location = 1) in vec4 fTexcoord;
layout (location = 2) in vec4 fNormal;
layout (location = 3) in vec4 fTangent;
layout (location = 4) flat in uvec4 uData;

// 
layout (location = COLORING) out vec4 colored;
layout (location = POSITION) out vec4 samples;
layout (location = NORMALED) out vec4 normals;
layout (location = TANGENTS) out vec4 tangent;
layout (location = EMISSION) out vec4 emission;

// 
void main() {
    const MaterialUnit unit = materials[0u].data[meshInfo[drawInfo.data.x].materialID];
    vec4 diffuseColor = unit.diffuseTexture >= 0 && hasTexcoord(meshInfo[drawInfo.data.x]) ? texture(textures[nonuniformEXT(unit.diffuseTexture)],fTexcoord.xy,0) : unit.diffuse;
    vec4 normalsColor = unit.normalsTexture >= 0 && hasTexcoord(meshInfo[drawInfo.data.x]) ? texture(textures[nonuniformEXT(unit.normalsTexture)],fTexcoord.xy,0) : unit.normals;
    vec4 specularColor = unit.specularTexture >= 0 && hasTexcoord(meshInfo[drawInfo.data.x]) ? texture(textures[nonuniformEXT(unit.specularTexture)],fTexcoord.xy,0) : unit.specular;
    vec4 emissionColor = unit.emissionTexture >= 0 && hasTexcoord(meshInfo[drawInfo.data.x]) ? texture(textures[nonuniformEXT(unit.emissionTexture)],fTexcoord.xy,0) : unit.emission;

    if (diffuseColor.w > 0.001f) {
        colored = vec4(max(vec4(DIFFUSE_COLOR,diffuseColor.w) - vec4(emissionColor.xyz*emissionColor.w,0.f),0.f.xxxx).xyz,1.f);
        normals = vec4(fNormal.xyz,1.f);
        samples = vec4(fPosition.xyz,1.f);
        emission = vec4(emissionColor.xyz,1.f);
        //emission = vec4(emissionColor.xyz,emissionColor.w);
    } else {
        colored = 0.f.xxxx;
        normals = vec4(0.f.xx,0.f.xx);
        samples = vec4(0.f.xxx,0.f.x);
        emission = 0.f.xxxx;
    };
    //ivec2 txd = ivec2(gl_FragCoord.xy), txs = imageSize(writeImages[DIFFUSED]);
    //const vec4 dEmi = imageLoad(writeImages[DIFFUSED], ivec2(txd.x,txs.y-txd.y-1));
    //imageStore(writeImages[DIFFUSED], ivec2(txd.x,txs.y-txd.y-1), vec4(emissionColor.xyz*emissionColor.w,0.f)+dEmi);
};
