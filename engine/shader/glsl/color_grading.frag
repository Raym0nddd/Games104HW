#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;

layout(set = 0, binding = 1) uniform sampler2D color_grading_lut_texture_sampler;

layout(location = 0) out highp vec4 out_color;

void main()
{
    highp ivec2 lut_tex_size = textureSize(color_grading_lut_texture_sampler, 0);
    highp float _COLORS      = float(lut_tex_size.y);

    highp vec4 color       = subpassLoad(in_color).rgba;
    
    // Raymond: sample color in LUT Texture based on Pos derived from the input color
    highp float blue_color = color.b * (_COLORS - 1.0);
    
    highp vec2 uv1, uv2;
    uv1.x = floor(blue_color) / _COLORS + (color.g * (_COLORS - 1.0) + 0.5) / (_COLORS * _COLORS);
    uv1.y = (color.r * (_COLORS - 1.0) + 0.5) / _COLORS;
    uv2.x = ceil(blue_color) / _COLORS + (color.g * (_COLORS - 1.0) + 0.5) / (_COLORS * _COLORS);
    uv2.y = (color.r * (_COLORS - 1.0) + 0.5) / _COLORS;
    
    highp vec4 lut_color1 = texture(color_grading_lut_texture_sampler, uv1);
    highp vec4 lut_color2 = texture(color_grading_lut_texture_sampler, uv2);
    highp float frac = blue_color - floor(blue_color);
    
    out_color = mix(lut_color1, lut_color2, frac);
}