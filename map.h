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
    static constexpr int MAX_HEIGHT = 64;
    static constexpr int MAX_LIGHT_LEVEL = 32;

    const static int half_limit = std::numeric_limits<int>::max() / 2;

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
                    }
                }
            }
        }

        ~chunk_t() {
        }

        Mesh mesh, transMesh;
        uint8_t iBricks[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
        uint8_t iLightLevels[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
        std::vector<vec3_t>lightList;

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
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit || y < 0 || y >= 60) {
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

        // It's important to make sure the chunk was generated at this point because
        // adjacent chunks may be trying to access data here.
        if(!chunk.bGen) {
            chunk.Generate(origxchunk,origzchunk,*this);
        }

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

        if(!Chunks[chunk_index].bGen)
            Chunks[chunk_index].Generate(origxchunk,origzchunk,*this);

        Chunks[std::make_pair(xchunk,zchunk)].iBricks[xindex][y][zindex] = id;
	}

    inline int GetLightLevel(int x, int z, int y) {
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit || y < 0 || y >= 60) {
            return -1;
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

		return chunk.iLightLevels[xindex][y][zindex];
	}

    inline void SetLightLevel(int x, int z, int y, int lvl) {
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

        if(lvl < 0)
            lvl = 0;

        Chunks[std::make_pair(xchunk,zchunk)].iLightLevels[xindex][y][zindex] = lvl;
	}

    void AddLight(int x, int z, int y, bool bRemove) {
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

        
        const float fallOff = 2;
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

            if(GetBrick(posx,posz,posy) != 0) { // If solid brick
                if(posx != x && posz != z && posy != y) {
                    continue;
                }
            }

            visited.push_back({posx, posy, posz});

            int lvl = GetLightLevel(posx,posz,y);
            if(bRemove) {
                if(currentPos.v > 0) {
                    SetLightLevel(posx, posz, posy ,lvl-(int)currentPos.v);
                }
            }
            else {
                if(currentPos.v > 0) {
                    SetLightLevel(posx, posz, posy ,lvl+(int)currentPos.v);
                }
            }

            for(int i = 0;i < dirs.size();i++) {
                glm::vec3 &d = dirs[i];

                if(GetBrick(posx + (int)d.x, posz + (int)d.z, posy + (int)d.y) == 0 && currentPos.v > 1)
                    positions.push({posx + (int)d.x, posy + (int)d.y, posz + (int)d.z, currentPos.v-fallOff});
            }
            
            iter_count++;
        }
        visited.clear();
        //std::cout << "iter_count: " << iter_count << std::endl;
    }

    void BuildChunk(int x, int z);
    void BuildChunkTrans(int x, int z);
    void RebuildLights();

    void RebuildAll();

	void FromBMP(std::string sfile);
	void Draw();

    void RunBuilder();

    void LoadBrickMetaData();

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
private:
    std::vector<std::string> TextureNamesFromFile(std::string filename);
    std::vector<std::string>BrickTextureFilenames;
    std::vector<build_schedule_info_t>ScheduledBuilds;

private:
    bool bIsDay;
    int viewDist;
    std::vector<std::thread>BuilderThreads;
    Camera &camera;
    float fAmbient;
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
    std::vector<float>BrickTransparencies;

    std::map<std::string, int>BrickNameMap;
    std::map <std::pair<int,int>, chunk_t>Chunks;

};

#endif
