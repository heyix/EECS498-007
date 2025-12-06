#pragma once
#include "DistributedDomain.h"
#include "FlatBody.h"
#include "FlatShape.h"
#include "FlatDefs.h"
#include "Vector2.h"

#include <random>
#include <cmath>
namespace FlatPhysics {
    static FlatPhysics::FlatBody* CreateStaticBoxBody(
        FlatPhysics::DistributedDomain& domain,
        const Vector2& center,
        float half_w,
        float half_h,
        float friction = 0.3f,
        float restitution = 0.0f);
    static void BuildGridWalls(
        FlatPhysics::DistributedDomain& domain,
        const FlatPhysics::FlatAABB& world_bounds,
        int   grid_x,
        int   grid_y,
        float wall_thickness = 2.0f);
    static FlatPhysics::FlatBody* CreateDynamicBody(
        FlatPhysics::DistributedDomain& domain,
        const Vector2& pos,
        bool use_box,
        float half_w,
        float half_h,
        float radius,
        float density = 1.0f,
        float friction = 0.2f,
        float restitution = 0.3f);
    void GenerateHugeTestCase(
        FlatPhysics::DistributedDomain& domain,
        const FlatPhysics::FlatAABB& world_bounds,
        int   grid_x,
        int   grid_y,
        int   num_bodies,
        float p_polygon);

    void GenerateHugeTestCaseSkewedCorner(
        FlatPhysics::DistributedDomain& domain,
        const FlatPhysics::FlatAABB& world_bounds,
        int   grid_x,
        int   grid_y,
        int   num_bodies,
        float p_polygon);
}