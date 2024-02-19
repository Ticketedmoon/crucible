#include "transform_system.h"

TransformSystem::TransformSystem(EntityManager& entityManager, sf::Clock& gameClock)
    : m_entityManager(entityManager), m_gameClock(gameClock)
{
}

void TransformSystem::execute()
{
    std::vector<Entity> entities = m_entityManager.getEntities();
    for (const Entity& entity : entities)
    {
        auto& entityTransform = entity.getComponent<Component::CTransform>();

        // @Refactor: Should we do this elsewhere?
        if (entity.hasComponent<Component::CControllable>())
        {
            resolveControllerMovementForEntity(entity, entityTransform);
        }

        if (entity.hasComponent<Component::CPathFollower>())
        {
            auto& cPathFollower = entity.getComponent<Component::CPathFollower>();

            if (m_gameClock.getElapsedTime().asMilliseconds() < cPathFollower.gameTimeTicker.timeUntilUpdate)
            {
                continue;
            }

            moveToNextWaypoint(entityTransform, cPathFollower);
        }
    }
}

void TransformSystem::moveToNextWaypoint(
        Component::CTransform& entityTransform,
        Component::CPathFollower& cPathFollower) const
{
    Waypoint waypoint = cPathFollower.path[cPathFollower.destinationIndex];
    float distanceToWaypoint = distance(waypoint.position, *entityTransform.position);

    if (distanceToWaypoint < 1)
    {
        cPathFollower.gameTimeTicker.timeUntilUpdate = m_gameClock.getElapsedTime().asMilliseconds() + waypoint.waitPeriodMs;
        cPathFollower.destinationIndex++;
    }

    // If destinationIndex is == path.size(), then reset to 0.
    if (cPathFollower.destinationIndex == cPathFollower.path.size())
    {
        cPathFollower.destinationIndex = 0;
        waypoint = cPathFollower.path[cPathFollower.destinationIndex];
    }

    // move forward (into the waypoint, based on the current object rotation)
    float r = std::atan2(waypoint.position.y - entityTransform.position->y, waypoint.position.x - entityTransform.position->x);
    entityTransform.position->x += std::cos(r) * GUARD_SPEED;
    entityTransform.position->y += std::sin(r) * GUARD_SPEED;
}

float TransformSystem::distance(Crucible::Vec2 p1, Crucible::Vec2 p2) {
    float xb = p2.x - p1.x;
    float yb = p2.y - p1.y;
    return std::sqrt((xb * xb) + (yb * yb));
}

void TransformSystem::resolveControllerMovementForEntity(const Entity& e, Component::CTransform& cTransform)
{
    auto& controllable = e.getComponent<Component::CControllable>();
    auto& animation = e.getComponent<Component::CAnimation>();

    // TODO @Refactor
    animation.animationList = {};

    if (controllable.isMovingDown)
    {
        cTransform.position->y += PLAYER_SPEED;
        animation.animationList = {
                PlayerAnimation::PLAYER_WALK_DOWN_A, PlayerAnimation::PLAYER_WALK_DOWN_B, PlayerAnimation::PLAYER_WALK_DOWN_C,
                PlayerAnimation::PLAYER_WALK_DOWN_D, PlayerAnimation::PLAYER_WALK_DOWN_E, PlayerAnimation::PLAYER_WALK_DOWN_F,
        };
    }
    if (controllable.isMovingUp)
    {
        cTransform.position->y -= PLAYER_SPEED;
        animation.animationList = {
                PlayerAnimation::PLAYER_WALK_UP_A, PlayerAnimation::PLAYER_WALK_UP_B, PlayerAnimation::PLAYER_WALK_UP_C,
                PlayerAnimation::PLAYER_WALK_UP_D, PlayerAnimation::PLAYER_WALK_UP_E, PlayerAnimation::PLAYER_WALK_UP_F,
                };
    }
    if (controllable.isMovingLeft)
    {
        cTransform.position->x -= PLAYER_SPEED;
        // TODO add other anims
        animation.animationList = {
                PlayerAnimation::PLAYER_WALK_LEFT_A, PlayerAnimation::PLAYER_WALK_LEFT_B, PlayerAnimation::PLAYER_WALK_LEFT_C,
                PlayerAnimation::PLAYER_WALK_LEFT_D, PlayerAnimation::PLAYER_WALK_LEFT_E, PlayerAnimation::PLAYER_WALK_LEFT_F
        };
    }
    if (controllable.isMovingRight)
    {
        cTransform.position->x += PLAYER_SPEED;
        animation.animationList = {
                PlayerAnimation::PLAYER_WALK_RIGHT_A, PlayerAnimation::PLAYER_WALK_RIGHT_B, PlayerAnimation::PLAYER_WALK_RIGHT_C,
                PlayerAnimation::PLAYER_WALK_RIGHT_D, PlayerAnimation::PLAYER_WALK_RIGHT_E, PlayerAnimation::PLAYER_WALK_RIGHT_F,
        };
    }
}