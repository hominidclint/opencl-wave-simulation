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

class GridGenerator
{
/*public:
    struct Vertex
    {
        Vertex(){}
        Vertex(const XMFLOAT3& position,
               const XMFLOAT3& normal,
               const XMFLOAT3& tangent,
               const XMFLOAT2& uv)
            : Position(p), Normal(n), TangentU(t), TexC(uv){}

        Vertex(float px, float py, float pz, 
               float nx, float ny, float nz,
               float tx, float ty, float tz,
               float u, float v)
            : Position(px,py,pz), Normal(nx,ny,nz),
              TangentU(tx, ty, tz), TexC(u,v){}

        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT3 TangentU;
        XMFLOAT2 TexC;
    };

    struct MeshData
    {
        std::vector<Vertex> Vertices;
        std::vector<int> Indices;
    };

    // mxn grid in the xz-plane with m rows and n columns, centered
    // at the origin with the specified width and depth.
    void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);*/
};

#endif // GRID_GENERATOR_H