//
// Copyright (c) 2016 Rokas Kupstys
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"

// Drag this file and any image on to the window of ShaderSketch.

varying vec4 vPos;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = vPos = GetClipPos(worldPos);
}

void PS()
{
	float d = distance(vPos.xy, vec2(sin(cElapsedTimePS * 0.5) / 2.5, sin(cElapsedTimePS)) / 2.5);
	float size = sin(cElapsedTimePS) / 10 + 0.5;
	vec4 color = vec4(0.2, 0.2, 0.2, 1);
	if (d < size)
		color = texture2D(sDiffMap, vec2(vPos.x, vPos.y * -1));
	gl_FragColor = color;
}
