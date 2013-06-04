// Copyright (c) 2013, Hannes Würfel <hannes.wuerfel@student.hpi.uni-potsdam.de>
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// wave propagation over grid
__kernel void compute_vertex_displacement(__global float4* prevGrid,
                                          __global float4* currGrid,
                                          __global float4* glBuffer,
                                          int width,
                                          float k1,
                                          float k2,
                                          float k3)
{
    unsigned int x = get_global_id(0);
    unsigned int y = get_global_id(1);

    if(x > 0 && x < get_global_size(0)-1 && y > 0 && y < get_global_size(1)-1)
    {
        prevGrid[y*width+x].y = k1 *  prevGrid[y*width+x].y     +
                                k2 *  currGrid[y*width+x].y     +
                                k3 * (currGrid[(y+1)*width+x].y +
                                      currGrid[(y-1)*width+x].y +
                                      currGrid[y*width+(x+1)].y +
                                      currGrid[y*width+(x-1)].y);

        glBuffer[y*width+x] = prevGrid[y*width+x];
    }
}

__kernel void compute_finite_difference_scheme(__global float4* currGrid,
                                               __global float4* glNormalBuffer,
                                               __global float4* glTangentBuffer,
                                               int width,
                                               float spatialStep)
{
    unsigned int x = get_global_id(0);
    unsigned int y = get_global_id(1);

    if(x > 0 && x < get_global_size(0)-1 && y > 0 && y < get_global_size(1)-1)
    {
        float l = currGrid[y*width+(x-1)].y;
        float r = currGrid[y*width+(x+1)].y;
        float t = currGrid[(y-1)*width+x].y;
        float b = currGrid[(y+1)*width+x].y;

        float4 estimatedNormal  = (float4)(l-r, 2.0f*spatialStep, b-t, 1.0f);
        float4 estimatedTangent = (float4)(2.0f*spatialStep, r-l, 0.0f, 1.0f);

        glNormalBuffer[y*width+x]  = normalize(estimatedNormal);
        glTangentBuffer[y*width+x] = normalize(estimatedTangent);
    }
}

// initialization kernel
__kernel void initialize_gl_grid(__global float4* glPositionBuffer,
                                 __global float4* glNormalBuffer,
                                 __global float4* glTangentBuffer,
                                 __global float4* clPositionBuffer,
                                 int width)
{
    unsigned int x = get_global_id(0);
    unsigned int y = get_global_id(1);

    glPositionBuffer[y*width+x] = clPositionBuffer[y*width+x];
    glNormalBuffer[y*width+x]   = (float4)(0.0f, 1.0f, 0.0f, 1.0f);
    glTangentBuffer[y*width+x]  = (float4)(1.0f, 0.0f, 0.0f, 1.0f);

}

// create water drop
__kernel void disturb_grid(__global float3* currGrid,
                           unsigned int i,
                           unsigned int j,
                           int width,
                           float magnitude)
{
    float halfMagnitude = 0.5f * magnitude;

    currGrid[i*width+j].y     += magnitude;
    currGrid[i*width+(j+1)].y += halfMagnitude;
    currGrid[i*width+(j-1)].y += halfMagnitude;
    currGrid[(i+1)*width+j].y += halfMagnitude;
    currGrid[(i-1)*width+j].y += halfMagnitude;
}