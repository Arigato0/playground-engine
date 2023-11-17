#include <any>

#include "application/engine.hpp"
#include "game/ecs.hpp"

struct HealthComp : public pge::Component
{
    int amount = 0;
};
class TestComp : public pge::Component
{
public:
    void on_start() override
    {
        m_health = m_parent->find<HealthComp>();

        if (m_health == nullptr)
        {
            Logger::fatal("could not get health comp");
        }
    }
    void update(double delta_time) override
    {
        m_health->amount += 1;
        Logger::info("health = {}", m_health->amount);
    }
private:
    HealthComp *m_health;
};

class Player : public pge::Entity
{
public:

    PGE_MAKE_SERIALIZABLE();
};

int main()
{
    pge::Engine app
    (
        {
            .title = "playground engine",
            .window_size = {720, 460},
            .graphics_api = pge::GraphicsApi::OpenGl,
        }
    );

    ASSERT_ERR(app.init());

    Player player;

    player.register_component<TestComp>();
    player.register_component<HealthComp>();

    app.register_entity("player", &player);

    ASSERT_ERR(app.run());
}