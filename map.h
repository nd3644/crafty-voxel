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
        chunk_t() {
            bGen = false;
            bIsCurrentlyGenerating = false;
            bIniialBuild = false;
            for(int i = 0;i < CHUNK_SIZE;i++) {
                for(int j = 0;j < CHUNK_SIZE;j++) {     
                    for(int y = 0;y < MAX_HEIGHT;y++) {
                        iBricks[i][y][j] = 0;
                        iLightLevels[i][y][j] = 0;


                        for(int f = 0;f < 6;f++) {
                            for(int v = 0;v < 4;v++) {
                                ambientVecs[i][y][j][f][v] = 1;
                            }
                        }
                    }
                }
            }
        }

        ~chunk_t() {
        }

        void PushLights(Map &map) {
//            std::cout << "pushlen: " << lightList.size() << std::endl;
            for(light_t &l: lightList) {
                pushedLights.push_back(l);
                map.AddLight(l.x, l.z, l.y, true);
            }
        }

        void PopLights(Map &map) {
            for(light_t &l: pushedLights) {
                map.AddLight(l.x, l.z, l.y, false);
            }
            pushedLights.clear();
        }

        Mesh mesh, transMesh;
        uint8_t iBricks[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
        int iLightLevels[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
        float ambientVecs[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE][6][4];
        std::vector<light_t>lightList;
        std::vector<light_t>pushedLights;

        bool bGen;
        // This is for preventing Generate recursively calling itself. This can probably be done better
        bool bIsCurrentlyGenerating;
        bool bIniialBuild;

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
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit || y < 0 || y >= MAX_HEIGHT) {
            return 0;
        }

        int origxchunk = x / CHUNK_SIZE;
        int origzchunk = z / CHUNK_SIZE;

        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;

        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        int xindex = x % CHUNK_SIZE;
        int zindex = z % CHUNK_SIZE;

        auto chunk_index = std::make_pair(xchunk,zchunk);

        auto &chunk = Chunks[chunk_index];

		return chunk.iBricks[xindex][y][zindex];
	}

    inline void SetBrick(int x, int z, int y, int id) {
        using namespace std;
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit || y < 0 || y >= MAX_HEIGHT) {
            return;
        }

        int origxchunk = x / CHUNK_SIZE;
        int origzchunk = z / CHUNK_SIZE;

        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;

        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        int xindex = x % CHUNK_SIZE;
        int zindex = z % CHUNK_SIZE;

        auto chunk_index = std::make_pair(xchunk,zchunk);

        auto &chunk = Chunks[chunk_index];

        chunk.iBricks[xindex][y][zindex] = id;
	}

    inline int GetLightLevel(int x, int z, int y) {
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit || y < 0 || y >= MAX_HEIGHT) {
            return -1;
        }

//        int origxchunk = x / CHUNK_SIZE;
//        int origzchunk = z / CHUNK_SIZE;

        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;

        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        int xindex = x % CHUNK_SIZE;
        int zindex = z % CHUNK_SIZE;

        auto chunk_index = std::make_pair(xchunk,zchunk);

        auto &chunk = Chunks[chunk_index];

		return chunk.iLightLevels[xindex][y][zindex];
	}

    inline void SetLightLevel(int x, int z, int y, int lvl) {
        using namespace std;
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit || y < 0 || y >= MAX_HEIGHT) {
            return;
        }

//        int origxchunk = x / CHUNK_SIZE;
//        int origzchunk = z / CHUNK_SIZE;

        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;

        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        int xindex = x % CHUNK_SIZE;
        int zindex = z % CHUNK_SIZE;

//        auto chunk_index = std::make_pair(xchunk,zchunk);

        if(lvl < 0)
            lvl = 0;

        Chunks[std::make_pair(xchunk,zchunk)].iLightLevels[xindex][y][zindex] = lvl;
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

    void BuildChunk(int x, int z);
    void BuildChunkTrans(int x, int z);
    void RebuildLights();

    void RebuildAll();

	void FromBMP(std::string sfile);
	void Draw();

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
        return &Chunks[std::make_pair(x,z)];
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
        if(GetBrick(x-1,z,y) > 0 && GetBrickTransparency(GetBrick(x-1,z,y)) == 1
        && GetBrick(x+1,z,y) > 0 && GetBrickTransparency(GetBrick(x+1,z,y)) == 1
        && GetBrick(x,z-1,y) > 0 && GetBrickTransparency(GetBrick(x,z-1,y)) == 1
        && GetBrick(x,z+1,y) > 0 && GetBrickTransparency(GetBrick(x,z+1,y)) == 1
        && GetBrick(x,z,y+1) > 0 && GetBrickTransparency(GetBrick(x,z,y+1)) == 1
        && GetBrick(x,z,y-1) > 0 && GetBrickTransparency(GetBrick(x,z,y-1)) == 1) {
            return true;
        }
        return false;
    }
private:
    bool BrickAOIsEqual(int x1, int y1, int z1, int x2, int y2, int z2) {
        int xchunk1 = floor(x1 / CHUNK_SIZE);
        int zchunk1 = floor(z1 / CHUNK_SIZE);

        int xindex1 = x1 % CHUNK_SIZE;
        int zindex1 = z1 % CHUNK_SIZE;

        int xchunk2 = floor(x2 / CHUNK_SIZE);
        int zchunk2 = floor(z2 / CHUNK_SIZE);

        int xindex2 = x2 % CHUNK_SIZE;
        int zindex2 = z2 % CHUNK_SIZE;

        chunk_t &chunk1 = Chunks[std::make_pair(xchunk1, zchunk1)];
        chunk_t &chunk2 = Chunks[std::make_pair(xchunk2, zchunk2)];

        for(int f = 0;f < 6;f++) {
            for(int v = 0;v < 4;v++) {
                if(chunk1.ambientVecs[xindex1][y1][zindex1][f][v] != chunk2.ambientVecs[xindex2][y2][zindex2][f][v])
                    return false;
            }
        }
        return true;
    }
    std::vector<std::string> TextureNamesFromFile(std::string filename);
    std::vector<std::string>BrickTextureFilenames;
    std::vector<build_schedule_info_t>ScheduledBuilds;

private:
    bool bIsDay;
    int viewDist;
    std::vector<std::thread>BuilderThreads;
    Camera &camera;
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
    std::vector<float>BrickTransparencies;

    std::map<std::string, int>BrickNameMap;
    std::map <std::pair<int,int>, chunk_t>Chunks;

};

#endif
