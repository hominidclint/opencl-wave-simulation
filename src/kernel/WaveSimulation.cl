// Copyright (c) 2013, Hannes W�rfel <hannes.wuerfel@student.hpi.uni-potsdam.de>
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

__kernel void compute_vertex_displacement(__global float3* prevGrid,
                                        __global float3* currGrid,
                                        int width,
                                        float k1,
                                        float k2,
                                        float k3)
{
    unsigned int x = get_global_id(0);
    unsigned int y = get_global_id(1);

    //
    // will be swapped on the host side after the kernel call
    //

    prevGrid[y*width+x].y = k1 *  prevGrid[y*width+x].y     +
                            k2 *  currGrid[y*width+x].y     +
                            k3 * (currGrid[(y+1)*width+x].y +
                                  currGrid[(y-1)*width+x].y +
                                  currGrid[y*width+(x+1)].y +
                                  currGrid[y*width+(x-1)].y);
}

// #TODO float4 to float3
__kernel void compute_finite_difference_scheme(__global float3* currGrid,
                                               __global float3* normals,
                                               __global float3* tangents,
                                               int width,
                                               float spatialStep)
{
    unsigned int x = get_global_id(0);
    unsigned int y = get_global_id(1);

    float l = currGrid[y*width+(x-1)].y;
    float r = currGrid[y*width+(x+1)].y;
    float t = currGrid[(y-1)*width+x].y;
    float b = currGrid[(y+1)*width+x].y;

    float3 estimatedNormal  = (float3)(l-r, 2.0f*spatialStep, b-t);
    float3 estimatedTangent = (float3)(2.0f*spatialStep, r-l, 0.0f);

    normals[y*width+x]  = normalize(estimatedNormal);
    tangents[y*width+x] = normalize(estimatedTangent);
}

__kernel void disturb_grid(__global float3* currGrid,
                         unsigned int i,
                         unsigned int j,
                         int width, // assumes grid is quadratic
                         float magnitude)
{
    unsigned int x = get_global_id(0);
    unsigned int y = get_global_id(1);

    float halfMagnitude = 0.5f * magnitude;

    //
    // only this specific thread is disturbing the ijth vertex height and its neighbors
    //

    if((x == 1 && y == 1))
    {
        currGrid[i*width+j].y     += magnitude;
        currGrid[i*width+(j+1)].y += halfMagnitude;
        currGrid[i*width+(j-1)].y += halfMagnitude;
        currGrid[(i+1)*width+j].y += halfMagnitude;
        currGrid[(i-1)*width+j].y += halfMagnitude;
    }
}