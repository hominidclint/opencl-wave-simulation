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

#ifndef CPU_WAVES_H
#define CPU_WAVES_H

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

class CPUWaves
{
public:
    CPUWaves();
    ~CPUWaves();

    unsigned int rowCount() const;
    unsigned int columnCount() const;
    unsigned int vertexCount() const;
    unsigned int triangleCount() const;
    float width() const;
    float depth() const;
    const float* k1() const;
    const float* k2() const;
    const float* k3() const;

    inline glm::vec4* getCurrentWaves() const {return m_currSolution;}
    inline glm::vec4* getCurrentNormals() const {return m_normals;}

    // returns the solution at the ith grid point
    inline const glm::vec4& operator[](int i) const {return m_currSolution[i];}

    // returns the solution normal at the ith grid point.
    inline const glm::vec4& normal(int i) const { return m_normals[i]; }

    // returns the unit tangent vector at the ith grid point in the local x-axis direction.
    inline const glm::vec4& tangentX(int i) const { return m_tangentX[i]; }

    void init(unsigned int m, unsigned int n, float dx, float dt, float speed, float damping);
    void update(double dt);
    void disturb(unsigned int i, unsigned int j, float magnitude);

private:
    unsigned int m_nRows;
    unsigned int m_nCols;

    unsigned int m_nVertices;
    unsigned int m_nTriangles;

    // precomputed simulation constants
    float m_k1;
    float m_k2;
    float m_k3;

    float m_timeStep;
    float m_spatialStep;

    glm::vec4* m_prevSolution;
    glm::vec4* m_currSolution;
    glm::vec4* m_normals;
    glm::vec4* m_tangentX;
};

#endif // CPU_WAVES_H