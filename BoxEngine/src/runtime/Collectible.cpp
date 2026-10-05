#include "runtime/Collectible.h"



Collectible::Collectible(Entity* entity, CollectibleType type, int value)
    : m_entity(entity), m_type(type), m_value(value)
{

}