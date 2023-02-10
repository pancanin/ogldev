/*
    Copyright 2023 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#define SLI_DEBUG_PRINTS

#include "slope_lighter.h"

void SlopeLighter::InitLighter(const Vector3f& LightDir, int TerrainSize, float MinHeight, float MaxHeight, float Softness)
{
    /* Slope lighting works by comparing the height of the current vertex with the
   height of the vertex which is "before" it on the way to the light source. This
   means that we need to reverse the light vector and in order to improve the accuracy
   intepolate between the two vertices that are closest to the reversed light vector (since
   in most cases the light vector will go between two vertices).

   The algorithm works by doing a dot product of the reversed light vector with two vectors:
   DirX (1, 0, 0) and DirZ (0, 0, 1).

   In order to understand the calculations below we need the following table that shows the
   two dot products for the eight vertices around the current one (degrees in parenthesis)

   |-------------------------------------------------------|
   | dpx: -0.707 (135) | dpx: 0 (90)   | dpx: 0.707 (45)   |
   | dpz: 0.707 (45)   | dpz: 1, (0)   | dpz: 0.707 (45)   |
   |-------------------|---------------|-------------------|
   | dpx: -1 (180)     |               | dpx: 1 (0)        |
   | dpz: 0 (90)       |               | dpz: 0 (90)       |
   |-------------------|---------------|-------------------|
   | dpx: -0.707 (135) | dpx: 0 (90)   | dpx: 0.707 (45)   |
   | dpz: -0.707 (135) | dpz: -1 (180) | dpz: -0.707 (135) |
   |-------------------------------------------------------|
*/
    m_terrainSize = TerrainSize;
    m_minHeight = MinHeight;
    m_maxHeight = MaxHeight;
    m_softness = Softness;

    Vector3f ReversedLightDir = LightDir * -1.0f;

    Vector3f DirX(1.0f, 0.0f, 0.0f);
    float dpx = DirX.Dot(ReversedLightDir);
    float cosx = ToDegree(acos(dpx));
    
    Vector3f DirZ(0.0f, 0.0f, 1.0f);
    float dpz = DirZ.Dot(ReversedLightDir);
    float cosz = ToDegree(acos(dpz));

#ifdef SLI_DEBUG_PRINTS
    printf("DirX %f %f\n", dpx, cosx);
    printf("DirZ %f %f\n", dpz, cosz);
#endif

    float RadianOf45Degrees = cosf(ToRadian(45.0f));

    bool InterpolateOnX = false;

    if (dpz >= RadianOf45Degrees) {
        //  printf("foo1\n");
        m_dz0 = m_dz1 = 1;
        InterpolateOnX = true;
    }
    else if (dpz <= -RadianOf45Degrees) {
        //printf("foo2\n");
        m_dz0 = m_dz1 = -1;
        InterpolateOnX = true;
    }
    else {
        if (dpz >= 0.0f) {
            //  printf("foo3\n");
            m_dz0 = 0;
            m_dz1 = 1;
            m_factor = dpz;
        }
        else {
            // printf("foo4\n");
            m_dz0 = 0;
            m_dz1 = -1;
            m_factor = -dpz;
        }

        if (dpx >= 0.0f) {
            // printf("foo5\n");
            m_dx0 = m_dx1 = 1;
        }
        else {
            // printf("foo6\n");
            m_dx0 = m_dx1 = -1;
        }
    }

    if (InterpolateOnX) {
        if (dpx >= 0.0f) {
            // printf("foo7\n");
            m_dx0 = 0;
            m_dx1 = 1;
            m_factor = dpx;
        }
        else {
            // printf("foo8\n");
            m_dx0 = 0;
            m_dx1 = -1;
            m_factor = -dpx;
        }
    }

    m_factor = 1.0f - m_factor / RadianOf45Degrees;

 #ifdef SLI_DEBUG_PRINTS
    printf("0: dx %d dz %d\n", dx0, dz0);
    printf("1: dx %d dz %d\n", dx1, dz1);
    printf("Factor %f\n", Factor);
#endif
}

float SlopeLighter::GetLighting(int x, int z) const
{
    float Height = m_pHeightmap->Get(x, z);

    float f = 0.0f;

    int XBefore0 = x + m_dx0 * 5;
    int ZBefore0 = z + m_dz0 * 5;

    int XBefore1 = x + m_dx1 * 5;
    int ZBefore1 = z + m_dz1 * 5;

    if ((XBefore0 >= 0) && (XBefore0 < m_terrainSize) && (ZBefore0 >= 0) && (ZBefore0 < m_terrainSize) &&
        (XBefore1 >= 0) && (XBefore1 < m_terrainSize) && (ZBefore1 >= 0) && (ZBefore1 < m_terrainSize)) {

        float HeightF32 = Height;
        float HeightBefore0 = m_pHeightmap->Get(XBefore0, ZBefore0);
        float HeightBefore1 = m_pHeightmap->Get(XBefore1, ZBefore1);

        // Interpolate between the height of the two vertices
        float HeightBefore = HeightBefore0 * m_factor + (1.0f - m_factor) * HeightBefore1;

        f = 1.0f - (HeightBefore - HeightF32) / m_softness;
        float min_brightness = 0.2f;
        f = std::max(std::min(f, 1.0f), min_brightness);
        //        printf("%f\n", f);
    }
    else {
        float Delta = Height - m_minHeight;
        float MaxDelta = m_maxHeight - m_minHeight;

        f = (float)Delta / (float)MaxDelta;
    }

    return f;
}
