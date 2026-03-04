#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(location = 0) out highp vec4 out_color;

void main()
{
    // using depth test and stencil test to achieve outline effect, so just output a solid color here
    out_color = vec4(1.0, 1.0, 0.0, 1.0);
}