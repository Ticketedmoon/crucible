#pragma once

#ifndef CRUCIBLE_ENTITY_SPAWNER_H
#define CRUCIBLE_ENTITY_SPAWNER_H

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>

#include "level_manager.h"
#include "system.h"
#include "entity_manager.h"
#include "common_constants.h"
#include "vertex.h"
#include "ray.h"
#include "tile.h"

class EntitySpawner
{
    public:
        explicit EntitySpawner(EntityManager& entityManager);

        void createPlayer();
        void createGuard(const std::string& lightingObjectLayerName, const std::string& pathingObjectLayerName);
        void createTile(Tile& t, bool isCollidable, bool immovable);

    private:
        static std::vector<Crucible::Ray> createRays(Component::CTransform& playerTransform, const std::string& layerName);
        static void updateTileTexture(Tile& tile);

    private:
        EntityManager& m_entityManager;
};


#endif //CRUCIBLE_ENTITY_SPAWNER_H
