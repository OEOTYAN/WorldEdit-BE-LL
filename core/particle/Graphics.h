//
// Created by OEOTYAN on 2022/5/15.
//
#pragma once
#ifndef WORLDEDIT_GRAPHICS_H
#define WORLDEDIT_GRAPHICS_H

#include "Particle.h"
#include "MC/AABB.hpp"
#include "MC/Vec3.hpp"
#include "string/StringTool.h"
#include <vector>
#include <map>

namespace worldedit {
    enum class GRAPHIC_COLOR;

    enum class DIRECTION {
        NEG_Y = 0,
        POS_Y = 1,
        NEG_Z = 2,
        POS_Z = 3,
        NEG_X = 4,
        POS_X = 5,
    };
    enum class FACING {
        NEG_Y = 0,
        POS_Y = 1,
        NEG_Z = 2,
        POS_Z = 3,
        NEG_X = 4,
        POS_X = 5,
    };

    bool facingIsPos(FACING facing);

    bool facingIsNeg(FACING facing);

    bool facingIsX(FACING facing);

    bool facingIsY(FACING facing);

    bool facingIsZ(FACING facing);

    FACING invFacing(FACING facing);

    std::string facingToString(FACING facing);

    std::string facingToDirString(FACING facing);

    void drawLine(const Vec3& originPoint,
                  FACING direction,
                  float length,
                  GRAPHIC_COLOR color,
                  int dimType = 0);
    void drawOrientedLine(Vec3 start, Vec3 end, int dimType);
}  // namespace worldedit

#endif  // WORLDEDIT_GRAPHICS_H
