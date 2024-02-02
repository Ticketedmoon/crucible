#include "level_manager.h"

LevelManager::LevelManager(EntityManager& entityManager) : m_entityManager(entityManager)
{
}

Level& LevelManager::loadLevel()
{
    // Level
    activeLevel = loadMapData();

    // Build Tile sheet
    buildTileSheet(tileSheetFilePath);

    tileSheetTexture = m_textureManager.getTexture(tileSheetFilePath);

    return activeLevel;
}

Tile& LevelManager::getTile(Level& level, uint32_t x, uint32_t y)
{
    uint32_t positionForTile = getPositionForTile(level, x, y);
    return level.tileLayers[0].data.at(positionForTile);
}

uint32_t LevelManager::getPositionForTile(const Level& level, uint32_t x, uint32_t y)
{
    uint32_t row = level.width * y;
    uint32_t positionForTile = row + x;
    return positionForTile;
}

Level LevelManager::loadMapData()
{
    std::ifstream f(mapFilePath);
    nlohmann::json data = nlohmann::json::parse(f);

    Level level;
    level.width = data["width"];
    level.height = data["height"];

    for (size_t tilesetIdx = 0; tilesetIdx < data[LEVEL_FILE_TILESETS_KEY].size(); tilesetIdx++)
    {
        size_t totalTiles = data[LEVEL_FILE_TILESETS_KEY][tilesetIdx][LEVEL_FILE_TILES_KEY].size();
        for (size_t tileIdx = 0; tileIdx < totalTiles; tileIdx++)
        {
            std::vector<std::unordered_map<std::string, std::string>> properties =
                    data[LEVEL_FILE_TILESETS_KEY][tilesetIdx][LEVEL_FILE_TILES_KEY][tileIdx]["properties"];
            uint32_t tileId =
                    data[LEVEL_FILE_TILESETS_KEY][tilesetIdx][LEVEL_FILE_TILES_KEY][tileIdx]["id"];

            for (std::unordered_map<std::string, std::string> p : properties)
            {
                if (p.contains("type"))
                {
                    TileType tileType = TILE_TYPE_LOOKUP_TABLE.at(p.at("value"));
                    level.tileTypeToId.insert({{tileType, tileId}});
                }
            }
        }
    }

    for (size_t layerIdx = 0; layerIdx < data[LEVEL_FILE_LAYERS_KEY].size(); layerIdx++)
    {
        std::string layerType = data[LEVEL_FILE_LAYERS_KEY][layerIdx]["type"];
        // TODO use enum class for this
        if (layerType == "tilelayer")
        {
            TileLayer layer;
            layer.name = data[LEVEL_FILE_LAYERS_KEY][layerIdx]["name"];
            layer.type = layerType;
            layer.data = createTilesForWorld(level, data, layerIdx);
            level.tileLayers.emplace_back(layer);
        }

        if (layerType == "objectgroup")
        {
            ObjectLayer layer;
            layer.name = data[LEVEL_FILE_LAYERS_KEY][layerIdx]["name"];
            layer.type = layerType;
            for (size_t i = 0; i < data[LEVEL_FILE_LAYERS_KEY][layerIdx][LEVEL_FILE_OBJECTS_KEY].size(); i++)
            {
                sf::FloatRect r;
                r.left = data[LEVEL_FILE_LAYERS_KEY][layerIdx][LEVEL_FILE_OBJECTS_KEY][i]["x"];
                r.top = data[LEVEL_FILE_LAYERS_KEY][layerIdx][LEVEL_FILE_OBJECTS_KEY][i]["y"];
                r.width = data[LEVEL_FILE_LAYERS_KEY][layerIdx][LEVEL_FILE_OBJECTS_KEY][i]["width"];
                r.height = data[LEVEL_FILE_LAYERS_KEY][layerIdx][LEVEL_FILE_OBJECTS_KEY][i]["height"];

                std::shared_ptr<sf::VertexArray> verts = std::make_shared<sf::VertexArray>();
                verts->append({{r.left, r.top}});
                verts->append({{r.left + r.width, r.top}});
                verts->append({{r.left + r.width, r.top + r.height}});
                verts->append({{r.left, r.top + r.height}});
                verts->append({{r.left, r.top}});
                layer.tileObjectVertices.emplace_back(verts);
            }

            level.objectLayers.emplace_back(layer);
        }
    }


    return level;
}

std::vector<Tile> LevelManager::createTilesForWorld(const Level& level, const nlohmann::json& data, const size_t layerIdx)
{
    std::vector<Tile> tiles;
    std::vector<long> tileValues = data[LEVEL_FILE_LAYERS_KEY][layerIdx][LEVEL_FILE_DATA_KEY].get<std::vector<long>>();
    for (unsigned int y = 0; y < level.height; y++)
    {
        for (unsigned int x = 0; x < level.width; x++)
        {
            uint32_t position = getPositionForTile(level, x, y);
            unsigned long tileValue = tileValues[position];
            TileRotation rotation{TileRotation::NONE};

            // Tile has been rotated
            if (tileValue > INT_MAX)
            {
                // Tile is rotated, set rotation flag and correct tileValue
                rotation = TileRotation::FLIPPED_LEFT;
                tileValue -= Crucible::FLIPPED_LEFT;
            }

            auto tileType = static_cast<TileType>(tileValue);
            sf::Vector2u pos{x, y};
            Tile tile(pos, tileType, rotation, {});
            tiles.emplace_back(tile);
        }
    }
    return tiles;
}

void LevelManager::buildTileSheet(const char* tileSheetFilePath)
{
    if (!m_textureManager.hasTexture(tileSheetFilePath))
    {
        m_textureManager.addTexture(tileSheetFilePath);
    }
}
