
all the code for game logic stuff is stored here

## Entity compontent system

```c++
class Player : public Entity
{}

class HealthComp : public IComponent
{
public:
    void update(double _) override
    {
        if (m_amount <= 0)
        {
            // handle death logic
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
private:
    int m_amount = 100;
};

class PlayerInputComp : public IComponent
{
    // handle input 
};

void main()
{
    // init engine
       
       // create entity
       Engine::entity_manager.create<Player, HealthComp, PlayerInputComp>("Player");
       
    // run engine
}
```
`PGE_CREATE_PROP_TABLE` creates a table of properties along with a utility get function and `PGE_MAKE_SERIALIZABLE` generates functions to serialize the property table.


