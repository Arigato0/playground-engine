
all the code for game logic stuff is stored here

## Entity compontent system

```c++
PGE_COMPONENT(HealthComp)
{
public:
    void update(double _) override
    {
        if (m_amount <= 0)
        {
            on_death();
        }
    }

    void set_health(int amount)
    {
        m_amount = amount;
    }

    int get_health()
    {
        return m_amount;
    }
    
    pge::Signal<void()> on_death;
private:
    int m_amount = 100;
};

PGE_COMPONENT(PlayerInputComp)
{
public:
    void update(double _) override
    {
        if (pge::key_pressed(pge::Key::Space))
        {
            jump()
        }
    }

};

void main()
{
    // init engine
       
       // create entity
       pge:Engine::entity_manager.create<HealthComp, PlayerInputComp>("Player");
       
    // run engine
}
```
`PGE_COMPONENT` is a macro that expands to `class Name : public Component<Name>` pge components use The Curiously Recurring Template Pattern
as a form of reflection to register the component as a prototype so the editor can view and clone it.
for storing components use `IComponent*` to avoid dealing with templates.

