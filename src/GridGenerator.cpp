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

#include "GridGenerator.h"

void GridGenerator::CreateGrid(float width, float depth, unsigned int m, unsigned int n, MeshData& meshData)
{
    unsigned int vertexCount = m*n;
    unsigned int faceCount   = (m-1)*(n-1)*2;

    //////////////////////////////////////////////////////////////////////////
    // create vertices
    //

    float halfWidth = 0.5f*width;
    float halfDepth = 0.5f*depth;

    float dx = width / (n-1);
    float dz = depth / (m-1);

    float du = 1.0f / (n-1);
    float dv = 1.0f / (m-1);

    meshData.vertices.resize(vertexCount);
    for(unsigned int i = 0; i < m; ++i)
    {
        float z = halfDepth - i*dz;
        for(unsigned int j = 0; j < n; ++j)
        {
            float x = -halfWidth + j*dx;

            meshData.vertices[i*n+j].Position = glm::vec3(x, 0.0f, z);
            meshData.vertices[i*n+j].Normal   = glm::vec3(0.0f, 1.0f, 0.0f);
            meshData.vertices[i*n+j].TangentU = glm::vec3(1.0f, 0.0f, 0.0f);

            // stretch texture over grid.
            meshData.vertices[i*n+j].TexC.x = j*du;
            meshData.vertices[i*n+j].TexC.y = i*dv;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Create the indices.
    //

    meshData.indices.resize(faceCount*3); // 3 indices per face

    // compute indices
    unsigned int k = 0;
    for(unsigned int i = 0; i < m-1; ++i)
    {
        for(unsigned int j = 0; j < n-1; ++j)
        {
            meshData.indices[k]   = i*n+j;
            meshData.indices[k+1] = i*n+j+1;
            meshData.indices[k+2] = (i+1)*n+j;

            meshData.indices[k+3] = (i+1)*n+j;
            meshData.indices[k+4] = i*n+j+1;
            meshData.indices[k+5] = (i+1)*n+j+1;

            k += 6; // next quad
        }
    }
}