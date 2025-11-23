#pragma once
#include "FlatConstraint.h"
namespace FlatPhysics {
    class PenetrationConstraintTwoPoint : public PenetrationConstraintBase
    {
    public:
        PenetrationConstraintTwoPoint(
            FlatFixture* a,
            FlatFixture* b,
            const Vector2& p0_world_a,
            const Vector2& p0_world_b,
            const Vector2& p1_world_a,
            const Vector2& p1_world_b,
            const Vector2& normal_world,
            float* n0_impulse_ptr,
            float* t0_impulse_ptr,
            float* n1_impulse_ptr,
            float* t1_impulse_ptr,
            bool   is_new_contact);

        virtual void PreSolve(float dt) override;
        virtual void Solve() override;
        virtual void PostSolve() override;

    private:
        Vector2 point_a1_local_;
        Vector2 point_b1_local_;

        Vector2 normal_local_;

        MatMN<2, 6> Jn_;

        VecN<2> normal_lambda_;
        VecN<2> normal_bias_;

        float friction_ = 0.0f;

        float* normal_impulse0_ = nullptr;
        float* tangent_impulse0_ = nullptr;
        float* normal_impulse1_ = nullptr;
        float* tangent_impulse1_ = nullptr;

        bool is_new_contact_ = false;

        Vector2 n_world_;
        Vector2 t_world_;

        Vector2 p0a_world_;
        Vector2 p0b_world_;
        Vector2 p1a_world_;
        Vector2 p1b_world_;

        Vector2 r0a_;
        Vector2 r0b_;
        Vector2 r1a_;
        Vector2 r1b_;

        float invMassA_ = 0.0f;
        float invMassB_ = 0.0f;
        float invIA_ = 0.0f;
        float invIB_ = 0.0f;

        MatMN<6, 2> JnT_cached_;
        MatMN<2, 2> K_cached_;
    };
}