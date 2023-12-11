#pragma once
#include "ecs.hpp"
#include "application/engine.hpp"
#include "graphics/CameraData.hpp"

namespace pge
{
    class CameraComp : public IComponent
    {
    public:
        void on_start() override
        {
            set_active();
        }

        void set_active()
        {
            Engine::renderer->set_camera(&data);
        }

        void update(double delta_time) override
        {
            data.process();
        }

        void move_forward(float mod = 1)
        {
            data.position += speed * data.front * mod;
        }

        void move_backward(float mod = 1)
        {
            data.position -= speed * data.front * mod;
        }

        void move_right(float mod = 1)
        {
            data.position += data.right * speed * mod;
        }

        void move_left(float mod = 1)
        {
            data.position -= data.right * speed * mod;
        }

        void move_up(float mod = 1)
        {
            data.position += data.up * speed * mod;
        }

        void move_down(float mod = 1)
        {
            data.position -= data.up * speed * mod;
        }

        float speed = 2.5f;
        CameraData data;
    };
}
