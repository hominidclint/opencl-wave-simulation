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

#ifndef GRID_GENERATOR_H
#define GRID_GENERATOR_H

#include <vector>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

class GridGenerator
{
public:
    struct Vertex
    {
        Vertex()
        {}

        Vertex(const glm::vec3& position,
               const glm::vec3& normal,
               const glm::vec3& tangent,
               const glm::vec2& uv)
            : Position(position), Normal(normal), TangentU(tangent), TexC(uv)
        {}

        Vertex(float px, float py, float pz, 
               float nx, float ny, float nz,
               float tx, float ty, float tz,
               float u, float v)
            : Position(px,py,pz), Normal(nx,ny,nz),
              TangentU(tx, ty, tz), TexC(u,v)
        {}

        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 TangentU;
        glm::vec2 TexC;
    };

    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    // mxn grid in the xz-plane with m rows and n columns, centered
    // at the origin with the specified width and depth.
    void CreateGrid(float width, float depth, unsigned int m, unsigned int n, MeshData& meshData);
};

#endif // GRID_GENERATOR_H