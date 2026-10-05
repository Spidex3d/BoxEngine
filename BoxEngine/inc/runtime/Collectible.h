#pragma once


class Entity;
// list foods Honey, Water, Meat, Fish, Fruit, Tomatos, Egg, Milk, Nuts, Seeds, Grains, Beans, Olives Sugar, Salt, Spices
enum class CollectibleType
{
	Water,
	Tomatos,
	Nuts,
    Honey
};

class Collectible
{
public:
    Collectible(
        Entity* entity,
        CollectibleType type,
        int value = 1
    );

    Entity* GetEntity() const
    {
        return m_entity;
    }

    CollectibleType GetType() const
    {
        return m_type;
    }

    int GetValue() const
    {
        return m_value;
    }

    bool IsCollected() const
    {
        return m_collected;
    }

    void Collect()
    {
        m_collected = true;
    }

private:
    Entity* m_entity = nullptr;

    CollectibleType m_type = CollectibleType::Water;

    int m_value = 1;

    bool m_collected = false;
};
