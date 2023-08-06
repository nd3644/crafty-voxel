#ifndef MAP_H
#define MAP_H

#include "ebmp.h"
#include <array>
#include "mesh.h"
#include "texture_array.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <thread>
#include <stack>
#include <queue>
#include <glm/glm.hpp>

#include <SDL2/SDL.h>

class Camera;
class Map
{
public:
	Map(Camera &c);
	~Map();

	 int height;
	 int width;
	 int depth;

    int NUM_THREADS;

    static constexpr int CHUNK_SIZE = 16;
    static constexpr int MAX_HEIGHT = 256;
    static constexpr int MAX_LIGHT_LEVEL = 16;

    const static int half_limit = std::numeric_limits<int>::max() / 2;

    struct light_t {
        int x, y, z;
    };

    struct chunk_t {

        enum ChunkState {
            DEFAULT_STAGE = 0,
            GEN_STAGE,
            AO_STAGE,
            BUILD_STAGE,
            UPLOAD_STAGE,
            READY_STAGE,
            NUM_STAGES
        };

        ChunkState curStage;

        chunk_t();
        ~chunk_t();

        void PushLights(Map &map);
        void PopLights(Map &map);

        Mesh mesh;
        uint8_t iBricks[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
        uint8_t iLightLevels[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
        uint8_t ambientVecs[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE][6][4];
        
        std::vector<light_t>lightList;
        std::vector<light_t>pushedLights;
        bool bVisible;

        bool bGen;
        // This is for preventing Generate recursively calling itself. This can probably be done better
        bool bIsCurrentlyGenerating;
        bool bIniialBuild, bInitialAOBuild;

        void Generate(int chunkx, int chunkz, Map &map);
    };

    enum Priority {
        IMMEDIATE = 0,
        ONE,
        TWO,
        THREE,
        NUM_PRIORITIES            
    };
    struct build_schedule_info_t {
        int x, z;
        Priority priorityLevel;
    };

    int IdFromName(std::string str) {
        return BrickNameMap[str];
    }

    /* This function makes a terrible amount of effort to prevent negative indices
      because they were causing a lot of trouble. */
	inline int GetBrick(int x, int z, int y) {
        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        int xindex = x % CHUNK_SIZE;
        int zindex = z % CHUNK_SIZE;

        int val = 255;
        auto key = std::make_pair(xchunk,zchunk);
        auto it = Chunks.find(key);
        if(it != Chunks.end()) {
            auto &chunk = Chunks[key];
            val = chunk.iBricks[xindex][y][zindex];
        }

        return val;
	}

    inline void SetBrick(int x, int z, int y, int id) {
        using namespace std;

        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        int xindex = x % CHUNK_SIZE;
        int zindex = z % CHUNK_SIZE;


        auto key = std::make_pair(xchunk,zchunk);
        auto it = Chunks.find(key);
        if(it != Chunks.end()) {
            auto &chunk = Chunks[key];
            chunk.iBricks[xindex][y][zindex] = id;
        }
	}

    inline int GetLightLevel(int x, int z, int y) {
		return 0;
	}

    inline void SetLightLevel(int x, int z, int y, int lvl) {
	}

    void AddLight(int x, int z, int y, bool bRemove) {
        int chunkx = floor(x / CHUNK_SIZE);
        int chunkz = floor(z / CHUNK_SIZE);

        if(bRemove) {
            auto &list = Chunks[std::make_pair(chunkx,chunkz)].lightList;
            for(size_t i = 0;i < list.size();i++) {
                if(list[i].x == x && list[i].y == y && list[i].z == z) {
                    list.erase(list.begin()+i);
                    break;
                }
            }
        }
        else {
            Chunks[std::make_pair(chunkx,chunkz)].lightList.push_back({x,y,z});
        }

        struct node { int x, y, z; float v; };
        std::vector<node>visited;

        float start = MAX_LIGHT_LEVEL;
        std::queue<node> positions;
        positions.push({x, y, z, start});
        int iter_count = 0;

        int RECURSIVE_LIMIT = 80960;

        std::vector<glm::vec3> dirs = {
            {-1, 0, 0},
            {1, 0, 0},
            {0, 0, -1},
            {0, 0, 1},
            {0, -1, 0},
            {0, 1, 0},
        };
        
        const float fallOff = 1;
        while (!positions.empty()) {
            if(iter_count > RECURSIVE_LIMIT) {
                break;
            }

            auto currentPos = positions.front();
            positions.pop();
            
            int posx = currentPos.x;
            int posy = currentPos.y;
            int posz = currentPos.z;

            bool bFound = false;
            for(auto &p: visited) {
                if(p.x == posx && p.y == posy && p.z == posz) {
                    bFound = true;
                    break;
                }
            }
            if(bFound) {
                continue;
            }
            if(currentPos.v <= 0) {
                continue;
            }

            if(GetBrick(posx,posz,posy) != 0) { // If solid brick
                if(posx != x && posz != z && posy != y) {
                    continue;
                }
            }

            if(GetBrick(posx,posz,posy) != 0 && GetBrick(posx,posz,posy) != 3) {
                continue;
            }

            visited.push_back({posx, posy, posz});

            int lvl = GetLightLevel(posx,posz,posy);
            if(bRemove) {
                if(currentPos.v > 0) {
                    SetLightLevel(posx, posz, posy ,lvl-(int)currentPos.v);
                }
            }
            else {
                SetLightLevel(posx, posz, posy ,lvl+(int)currentPos.v);
            }

            for(size_t i = 0;i < dirs.size();i++) {
                glm::vec3 &d = dirs[i];

                positions.push({posx + (int)d.x, posy + (int)d.y, posz + (int)d.z, currentPos.v-fallOff});
            }
            
            iter_count++;
        }
        visited.clear();
        std::cout << "iter_count: " << iter_count << std::endl;

        // Schedule mesh updates
        for(int mx = chunkx - 2;mx < chunkx + 2;mx++){
            for(int mz = chunkz - 2;mz < chunkz + 2;mz++){
                ScheduleMeshBuild({mx,mz,Map::Priority::ONE});
            }
        }
    }

    void BuildChunk(int chunkX, int chunkZ);
    void BuildChunkAO(int chunkX, int chunkZ);

    void RebuildLights();

    void RebuildAll();

	void FromBMP(std::string sfile);
	void Draw(Camera &cam);

    void RunBuilder();

    void LoadBrickMetaData();

    // Take a chunk position and buiilds the 9 chunks within and around that point
    void ScheduleAdjacentChunkBuilds(int startx, int startz, Priority level);

    std::vector<vec3_t>lights;

    std::vector<std::tuple<float, float, RGB>>toDraw;

    int int_pair(int x, int z) {
        std::string str = std::to_string(x) + std::to_string(z);
        return std::stoi(str);
    }

    std::vector<std::string> GetTextureFilenames() {
        return BrickTextureFilenames;
    }

    bool IsDay() const ;
    void SetDay(bool b);

    std::vector<std::array<int,6>>GetLookupArr() const;
    void FillWater(int x, int z, int y);

    void ScheduleMeshBuild(build_schedule_info_t info);

    chunk_t *GetChunk(int x, int z) {
        x *= CHUNK_SIZE;
        z *= CHUNK_SIZE;

/*        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;*/

        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        auto chunk_index = std::make_pair(xchunk,zchunk);

        return &Chunks[chunk_index];
    }

    float GetBrickTransparency(int id) {
        if(id < 0 || id >= BrickTransparencies.size())
            return 1;
        return BrickTransparencies[id];
    }

    // Makes all chunks within the view distance rebuild their meshes.
    // This is more for debugging than anything.
    void RebuildAllVisible();


    bool IsBrickSurroundedByOpaque(int x, int z, int y) {
        // These are checked in a funny order because they are in order of most-to-least likely
        if(GetBrick(x,z,y+1) <= 0 
        || GetBrick(x-1,z,y) <= 0
        || GetBrick(x+1,z,y) <= 0
        || GetBrick(x,z-1,y) <= 0
        || GetBrick(x,z+1,y) <= 0
        || GetBrick(x,z,y-1) <= 0){
            return false;
        }
        return true;
    }

    // Pre-generates the world given a radius around a chunk
    void GenerateChunksFromOrigin(int fromX, int fromZ, int radius);
private:
    std::thread threads[16];
    std::vector<std::string> TextureNamesFromFile(std::string filename);
    std::vector<std::string>BrickTextureFilenames;
    std::vector<build_schedule_info_t>ScheduledBuilds;

private:
    bool bIsDay;
    std::vector<std::thread>BuilderThreads;
    Camera &camera;
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
    std::vector<float>BrickTransparencies;

    std::map<std::string, int>BrickNameMap;
    std::map <std::pair<int,int>, chunk_t>Chunks;

};

#endif
