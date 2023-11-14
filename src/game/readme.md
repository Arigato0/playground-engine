
all the code for game logic stuff is stored here

## Entity compontent system

```c++
class Player : public Entity
{
public:

   Player()
    {
        auto &health = m_props[PROP_HEALTH];

        health = 100;

        m_props[PROP_NAME] = std::string {"john"};
    }

    void update(int delta_time) override
    {
        fmt::println("health = {}", get<int&>(PROP_HEALTH));

        fmt::println("name = {}", get<std::string&>(PROP_NAME));
    }

    PGE_MAKE_SERIALIZABLE();

private:
    PGE_CREATE_PROP_TABLE(
        PROP_HEALTH,
        PROP_NAME,
        PROP_ID);
};
```
`PGE_CREATE_PROP_TABLE` creates a table of properties along with a utility get function and `PGE_MAKE_SERIALIZABLE` generates functions to serialize the property table.


