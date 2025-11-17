#include "PenetrationConstraintTwoPoint.h"
#include "FlatBody.h"
#include <cmath>
#include <algorithm>
namespace FlatPhysics {

    PenetrationConstraintTwoPoint::PenetrationConstraintTwoPoint(
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
        bool   is_new_contact)
        : PenetrationConstraintBase(
            a,
            b,
            a->GetBody()->WorldToLocal(p0_world_a),
            b->GetBody()->WorldToLocal(p0_world_b))
        , point_a1_local_(a->GetBody()->WorldToLocal(p1_world_a))
        , point_b1_local_(b->GetBody()->WorldToLocal(p1_world_b))
        , normal_local_(a->GetBody()->WorldToLocal(normal_world))
        , friction_(std::max(a->GetFriction(), b->GetFriction()))
        , normal_impulse0_(n0_impulse_ptr)
        , tangent_impulse0_(t0_impulse_ptr)
        , normal_impulse1_(n1_impulse_ptr)
        , tangent_impulse1_(t1_impulse_ptr)
        , is_new_contact_(is_new_contact)
    {
        Jn_.Zero();
        normal_lambda_.Zero();
        normal_bias_.Zero();
    }

    void PenetrationConstraintTwoPoint::PreSolve(float dt)
    {
        FlatBody* bodyA = a->GetBody();
        FlatBody* bodyB = b->GetBody();

        n_world_ = bodyA->LocalToWorld(normal_local_).Normalized();

        p0a_world_ = bodyA->LocalToWorld(point_a);
        p0b_world_ = bodyB->LocalToWorld(point_b);
        r0a_ = p0a_world_ - bodyA->GetMassCenterWorld();
        r0b_ = p0b_world_ - bodyB->GetMassCenterWorld();

        p1a_world_ = bodyA->LocalToWorld(point_a1_local_);
        p1b_world_ = bodyB->LocalToWorld(point_b1_local_);
        r1a_ = p1a_world_ - bodyA->GetMassCenterWorld();
        r1b_ = p1b_world_ - bodyB->GetMassCenterWorld();

        const Vector2& n = n_world_;

        Jn_(0, 0) = -n.x();
        Jn_(0, 1) = -n.y();
        Jn_(0, 2) = Vector2::Cross(-r0a_, n);
        Jn_(0, 3) = n.x();
        Jn_(0, 4) = n.y();
        Jn_(0, 5) = Vector2::Cross(r0b_, n);

        Jn_(1, 0) = -n.x();
        Jn_(1, 1) = -n.y();
        Jn_(1, 2) = Vector2::Cross(-r1a_, n);
        Jn_(1, 3) = n.x();
        Jn_(1, 4) = n.y();
        Jn_(1, 5) = Vector2::Cross(r1b_, n);

        invMassA_ = bodyA->GetInverseMass();
        invMassB_ = bodyB->GetInverseMass();
        invIA_ = bodyA->GetInverseInertia();
        invIB_ = bodyB->GetInverseInertia();

        MatMN<6, 6> inv_m = GetInverseM(); 
        JnT_cached_ = Jn_.Transpose();
        K_cached_ = Jn_ * inv_m * JnT_cached_;

        const float beta = 0.1f;
        const float restitution_threshold = 1.0f;
        const float e = std::min(a->GetRestitution(), b->GetRestitution());

        {
            float C = Vector2::Dot(p0b_world_ - p0a_world_, -n);
            C = std::min(0.0f, C + 0.005f);

            Vector2 v0a = bodyA->GetLinearVelocity()
                + Vector2(-bodyA->GetAngularVelocity() * r0a_.y(),
                    bodyA->GetAngularVelocity() * r0a_.x());
            Vector2 v0b = bodyB->GetLinearVelocity()
                + Vector2(-bodyB->GetAngularVelocity() * r0b_.y(),
                    bodyB->GetAngularVelocity() * r0b_.x());

            float v_rel_dot_n0 = Vector2::Dot((v0b - v0a), n);

            float bias0 = (beta / dt) * C;
            if (is_new_contact_ && v_rel_dot_n0 < -restitution_threshold)
            {
                bias0 += (e * v_rel_dot_n0);
            }

            normal_bias_(0) = bias0;
        }

        {
            float C = Vector2::Dot(p1b_world_ - p1a_world_, -n);
            C = std::min(0.0f, C + 0.005f);

            Vector2 v1a = bodyA->GetLinearVelocity()
                + Vector2(-bodyA->GetAngularVelocity() * r1a_.y(),
                    bodyA->GetAngularVelocity() * r1a_.x());
            Vector2 v1b = bodyB->GetLinearVelocity()
                + Vector2(-bodyB->GetAngularVelocity() * r1b_.y(),
                    bodyB->GetAngularVelocity() * r1b_.x());

            float v_rel_dot_n1 = Vector2::Dot((v1b - v1a), n);

            float bias1 = (beta / dt) * C;
            if (is_new_contact_ && v_rel_dot_n1 < -restitution_threshold)
            {
                bias1 += (e * v_rel_dot_n1);
            }

            normal_bias_(1) = bias1;
        }

        float oldN0 = normal_impulse0_ ? *normal_impulse0_ : 0.0f;
        float oldT0 = tangent_impulse0_ ? *tangent_impulse0_ : 0.0f;
        float oldN1 = normal_impulse1_ ? *normal_impulse1_ : 0.0f;
        float oldT1 = tangent_impulse1_ ? *tangent_impulse1_ : 0.0f;

        normal_lambda_(0) = oldN0;
        normal_lambda_(1) = oldN1;

        if (oldN0 != 0.0f || oldN1 != 0.0f)
        {
            VecN<6> impulses = JnT_cached_ * normal_lambda_;

            bodyA->ApplyImpulseLinear({ impulses(0), impulses(1) }, false);
            bodyA->ApplyImpulseAngular(impulses(2), false);

            bodyB->ApplyImpulseLinear({ impulses(3), impulses(4) }, false);
            bodyB->ApplyImpulseAngular(impulses(5), false);
        }

        if (friction_ > 0.0f && (oldT0 != 0.0f || oldT1 != 0.0f))
        {
            t_world_ = n_world_.NormalDirection().Normalized();

            if (oldT0 != 0.0f)
            {
                Vector2 P = oldT0 * t_world_;

                bodyA->ApplyImpulseLinear(-P, false);
                bodyA->ApplyImpulseAngular(-Vector2::Cross(r0a_, P), false);

                bodyB->ApplyImpulseLinear(P, false);
                bodyB->ApplyImpulseAngular(Vector2::Cross(r0b_, P), false);
            }

            if (oldT1 != 0.0f)
            {
                Vector2 P = oldT1 * t_world_;

                bodyA->ApplyImpulseLinear(-P, false);
                bodyA->ApplyImpulseAngular(-Vector2::Cross(r1a_, P), false);

                bodyB->ApplyImpulseLinear(P, false);
                bodyB->ApplyImpulseAngular(Vector2::Cross(r1b_, P), false);
            }
        }
    }


    void PenetrationConstraintTwoPoint::Solve()
    {
        FlatBody* bodyA = a->GetBody();
        FlatBody* bodyB = b->GetBody();

        VecN<6> v = GetVelocities();

        VecN<2> rhs = Jn_ * v;
        rhs *= -1.0f;
        rhs(0) -= normal_bias_(0);
        rhs(1) -= normal_bias_(1);

        VecN<2> delta_lambda = MatMN<2, 2>::SolveGS(K_cached_, rhs);

        VecN<2> old_lambda = normal_lambda_;
        normal_lambda_ += delta_lambda;

        if (normal_lambda_(0) < 0.0f) normal_lambda_(0) = 0.0f;
        if (normal_lambda_(1) < 0.0f) normal_lambda_(1) = 0.0f;

        delta_lambda = normal_lambda_ - old_lambda;

        VecN<6> impulses = JnT_cached_ * delta_lambda;

        bodyA->ApplyImpulseLinear({ impulses(0), impulses(1) }, false);
        bodyA->ApplyImpulseAngular(impulses(2), false);

        bodyB->ApplyImpulseLinear({ impulses(3), impulses(4) }, false);
        bodyB->ApplyImpulseAngular(impulses(5), false);

        if (friction_ > 0.0f)
        {
            const Vector2& n = n_world_;
            if (t_world_.LengthSquared() == 0.0f)
                t_world_ = n.NormalDirection().Normalized();
            const Vector2& t = t_world_;

            auto solveFrictionForPoint =
                [&](int normalIndex,
                    float& tangentImpulse,
                    const Vector2& ra,
                    const Vector2& rb)
                {
                    Vector2 vA = bodyA->GetLinearVelocity()
                        + Vector2(-bodyA->GetAngularVelocity() * ra.y(),
                            bodyA->GetAngularVelocity() * ra.x());
                    Vector2 vB = bodyB->GetLinearVelocity()
                        + Vector2(-bodyB->GetAngularVelocity() * rb.y(),
                            bodyB->GetAngularVelocity() * rb.x());
                    Vector2 vRel = vB - vA;

                    float vt = Vector2::Dot(vRel, t);

                    float rtA = Vector2::Cross(ra, t);
                    float rtB = Vector2::Cross(rb, t);

                    float kT = invMassA_ + invMassB_ + invIA_ * rtA * rtA + invIB_ * rtB * rtB;
                    if (kT <= 0.0f)
                        return;

                    float lambda = -vt / kT;

                    float oldT = tangentImpulse;
                    float maxFriction = friction_ * normal_lambda_(normalIndex);
                    float newT = std::clamp(oldT + lambda, -maxFriction, maxFriction);
                    float dT = newT - oldT;

                    Vector2 P = dT * t;

                    bodyA->ApplyImpulseLinear(-P, false);
                    bodyA->ApplyImpulseAngular(-Vector2::Cross(ra, P), false);

                    bodyB->ApplyImpulseLinear(P, false);
                    bodyB->ApplyImpulseAngular(Vector2::Cross(rb, P), false);

                    tangentImpulse = newT;
                };

            float t0 = tangent_impulse0_ ? *tangent_impulse0_ : 0.0f;
            solveFrictionForPoint(0, t0, r0a_, r0b_);
            if (tangent_impulse0_) *tangent_impulse0_ = t0;

            float t1 = tangent_impulse1_ ? *tangent_impulse1_ : 0.0f;
            solveFrictionForPoint(1, t1, r1a_, r1b_);
            if (tangent_impulse1_) *tangent_impulse1_ = t1;
        }

        if (normal_impulse0_) *normal_impulse0_ = normal_lambda_(0);
        if (normal_impulse1_) *normal_impulse1_ = normal_lambda_(1);
    }

    void PenetrationConstraintTwoPoint::PostSolve()
    {

        FlatBody* bodyA = a->GetBody();
        FlatBody* bodyB = b->GetBody();

        const float linearSlop = 0.01f;
        const float percent = 0.2f;
        const float maxCorr = 0.02f;

        const float invMassA = invMassA_;
        const float invMassB = invMassB_;
        const float invIA = invIA_;
        const float invIB = invIB_;

        const Vector2 n = bodyA->LocalToWorld(normal_local_).Normalized();

        auto solvePositionForContact =
            [&](const Vector2& pa_local, const Vector2& pb_local)
            {
                const Vector2 pa = bodyA->LocalToWorld(pa_local);
                const Vector2 pb = bodyB->LocalToWorld(pb_local);

                const Vector2 ra = pa - bodyA->GetMassCenterWorld();
                const Vector2 rb = pb - bodyB->GetMassCenterWorld();

                const float rnA = Vector2::Cross(ra, n);
                const float rnB = Vector2::Cross(rb, n);

                float K = invMassA + invMassB + rnA * rnA * invIA + rnB * rnB * invIB;
                if (K <= 0.0f)
                    return;

                float C = Vector2::Dot(pb - pa, -n);
                float error = std::max(-C - linearSlop, 0.0f);
                float correction = std::min(percent * error, maxCorr);
                float impulseN = correction / std::max(K, 1e-8f);

                Vector2 P = impulseN * n;

                bodyA->Move(-invMassA * P, false);
                bodyA->Rotate(-invIA * rnA * impulseN, false);

                bodyB->Move(+invMassB * P, false);
                bodyB->Rotate(+invIB * rnB * impulseN, false);
            };

        solvePositionForContact(point_a, point_b);

        solvePositionForContact(point_a1_local_, point_b1_local_);
    }

} 
