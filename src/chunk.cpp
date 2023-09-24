#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <gsl/gsl_spline.h>

#include "map.h"
#include "helper.h"

#include <cassert>

/* const int numPoints = 6;
double ax[numPoints] = { -1, -0.8, 0.3, 0.5, 0.8, 1.0 };
double ay[numPoints] = { 20, 05, 80, 90, 110, 256 }; */

Eternal::Sprite chunk_t::perlinSprite;
noise::module::Perlin chunk_t::normalPerlin, chunk_t::erosionPerlin;
noise::module::RidgedMulti chunk_t::ridgedPerlin;
noise::module::Perlin chunk_t::heatPerlin;


std::vector<double> ax = { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
std::vector<double> ay = { 20, 40, 60, 65, 100, 130, 150, 170, 180, 230, 240 }; 

double interpolateY(double x)
{
    assert(ax.size() == ay.size());

    int numPoints = ax.size();

    if(x < -1)
        x = -1;
    if(x > 1)
        x = 1;
    
    gsl_interp_accel* accel = gsl_interp_accel_alloc();
    gsl_interp* interp = gsl_interp_alloc(gsl_interp_linear, numPoints);

    gsl_interp_init(interp, ax.data(), ay.data(), numPoints);
    double interpolatedY = gsl_interp_eval(interp, ax.data(), ay.data(), x, accel);

    gsl_interp_free(interp);
    gsl_interp_accel_free(accel);

    return interpolatedY;
}

chunk_t::chunk_t() {
    curStage = DEFAULT_STAGE;
    bVisible = false;
    bGen = false;
    bIsCurrentlyGenerating = false;
    bIniialBuild = bInitialAOBuild = false;
    for(int i = 0;i < CHUNK_SIZE;i++) {
        for(int j = 0;j < CHUNK_SIZE;j++) {     
            for(int y = 0;y < MAX_HEIGHT;y++) {
                iBricks[i][y][j] = 0;
                iLightLevels[i][y][j] = 0;


                for(int f = 0;f < 6;f++) {
                    for(int v = 0;v < 4;v++) {
                        // ambientVecs[i][y][j][f][v] = 1;
                    }
                }
            }
        }
    }

    heatShift = 1.0f;
    iRebuildCounter = 0;
    bRequiresRebuild = false;
}

chunk_t::~chunk_t() {
}

void chunk_t::Generate(int chunkx, int chunkz, Map &map) {
    if(bGen || bIsCurrentlyGenerating)
        return;

    for(int x = -1;x < 1;x++) {
        if(x == 0)
            continue;
        for(int z = -1;z < 1;z++) {
            if(z == 0)
                continue;
            map.GetChunk(chunkx,chunkz)->bRequiresRebuild = true;
        }
    }

    const int SEA_LEVEL = 84;

    bIsCurrentlyGenerating = true;
    bool bMount = (rand()%5 == 1) ? true : false;

    int brickType = 1;
    if(bMount) {
        brickType = 1;
    }

//    std::cout << "generating " << chunkx << " , " << chunkz << std::endl;
//    constexpr int HEAT_PERLIN_SEED = 4321;

    using namespace noise;

    double FREQ = 0.0005;

//    module::Perlin normalPerlin;
    static module::Perlin normalPerlin, secondPerlin;
    normalPerlin.SetOctaveCount(8);

    static module::Perlin ridgedPerlin;
    ridgedPerlin.SetSeed(5653);
    static module::Perlin heatPerlin;

    heatPerlin.SetSeed(65456);
    heatPerlin.SetFrequency(FREQ / 2.0);
    heatPerlin.SetPersistence(0.2);

    normalPerlin.SetFrequency(FREQ);
    secondPerlin.SetFrequency(FREQ / 2.0);

    secondPerlin.SetSeed(444);
    secondPerlin.SetOctaveCount(8);

    module::Add addModule;
    addModule.SetSourceModule(0, normalPerlin);
    addModule.SetSourceModule(1, secondPerlin);

    module::ScaleBias scaled;
    scaled.SetSourceModule(0, addModule);
    scaled.SetScale(0.8);

    heatShift = 0;//heatPerlin.GetValue(((chunkx*CHUNK_SIZE)+8), ((chunkz*CHUNK_SIZE)+8), 0.5f) / 12.0f;

    std::vector<vec3_t>toFill;
    for (int x = 0; x < CHUNK_SIZE;x++) {
        int xindex = (chunkx*CHUNK_SIZE)+x;
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int zindex = (chunkz*CHUNK_SIZE)+z;
            float unit = scaled.GetValue((float)xindex,(float)zindex,0.5);

            int height = 0;//(((unit + 1) / 2) * MAX_HEIGHT);

            height = interpolateY(unit);

            height = ((scaled.GetValue((float)xindex,(float)zindex,0.5)+1)/2.0) * MAX_HEIGHT;

            height = ClampValue(height, 0, MAX_HEIGHT-1);

            // default to grass
            brickType = map.BrickNameMap["grass_top"];

            for (int y = height; y > 0; y--) {

                if(height < MAX_HEIGHT && map.GetBrick(xindex,zindex,y+1) == 0)
                    brickType = map.BrickNameMap["grass_top"];
                else
                    brickType = map.BrickNameMap["stone"];

                if(y > 200) {
                    brickType = map.BrickNameMap["snow_top"];
                }

				map.SetBrick(xindex, zindex, y, brickType);
			}

            for(int y = 1;y < SEA_LEVEL + 5;y++) {
                if(map.GetBrick(xindex,zindex,y) == 1) {
                    map.SetBrick(xindex,zindex,y,map.IdFromName("sand"));
                }
            }
            for(int y = 1;y < SEA_LEVEL;y++) {
                if(map.GetBrick(xindex,zindex,y) == 0) {
                    map.SetBrick(xindex,zindex,y,map.IdFromName("water"));
                    if(heatPerlin.GetValue((float)xindex, (float)zindex, 0.5) > 0.5) {
                        brickType = map.BrickNameMap["sand"];
                    }
                }
            }
		}
	}

    // Plant some trees
    if(rand() % 6 == 0) {
        int lX = rand() % 16;
        int lZ = rand() % 16;

        if(lX < 4)
            lX = 4;
        if(lZ < 4)
            lZ = 4;
        
        if(lX > 13)
            lX = 13;
        if(lZ > 13)
            lZ = 13;

        lX += chunkx * CHUNK_SIZE;
        lZ += chunkz * CHUNK_SIZE;
        int height = (rand() % 6) + 5;

        bool bCanPlant = false;

        int groundLvl = 1;
        for(groundLvl = 1;groundLvl < MAX_HEIGHT;groundLvl++) {
            if(map.GetBrick(lX,lZ,groundLvl) == 0) {
                if(map.GetBrick(lX,lZ,groundLvl-1) == map.IdFromName("grass_top")) { // Only grow ontop of grass
                    bCanPlant = true;
                }
                break;
            }
        }

        if(bCanPlant) {
            for(int y = 0;y < groundLvl+height;y++) {
                map.SetBrick(lX,lZ,y,map.IdFromName("tree"));
            }

            for(int y = 0;y < 4;y++) {
                int length = 4 - y;
                for(int x = lX - length;x <= lX + length;x++) {
                    for(int z = lZ - length;z <= lZ + length;z++) {
                        map.SetBrick(x,z,(groundLvl+height)+y,map.IdFromName("leaves"));
                    }
                }
            }
            for(int x = lX - 3;x <= lX + 3;x++) {
                for(int z = lZ - 3;z <= lZ + 3;z++) {
                    map.SetBrick(x,z,(groundLvl+height)-1,map.IdFromName("leaves"));
                }
            }
        }
        
    }

    // Dig cavess
/*      module::RidgedMulti caves;
    caves.SetFrequency(FREQ*30);
    caves.SetOctaveCount(4);
    caves.SetSeed(33356);

    module::Perlin caves2;
    caves2.SetFrequency(FREQ*30);
    caves2.SetOctaveCount(4);
    caves2.SetSeed(33357);

    module::Add addedCaves;
    addedCaves.SetSourceModule(0, caves);
    addedCaves.SetSourceModule(1, caves2);

    module::ScaleBias scaledCaves;
    scaledCaves.SetSourceModule(0, addedCaves);
    scaledCaves.SetBias(-0.25);
    scaledCaves.SetScale(0.8);

    for (int x = 0; x < CHUNK_SIZE;x++) {
        int xindex = (chunkx*CHUNK_SIZE)+x;
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int zindex = (chunkz*CHUNK_SIZE)+z;
            for(int y = SEA_LEVEL-4;y > 0;y--) { //-4 is just a random offset
                double val = scaledCaves.GetValue((float)xindex * 0.4, (float)(y-(SEA_LEVEL/2)), (float)zindex * 0.4); // 0.4 scaling factor
                if(val > 0.8) {
                    int curBrick = map.GetBrick(xindex,zindex,y);
                    if(curBrick == map.IdFromName("stone")
                    || map.GetBrick(xindex,zindex,y) == map.IdFromName("sand")) {
                        map.SetBrick(xindex,zindex,y,0);
                    }
                }
            }
        }
    } 
 */

    // Apply bedrock
    for (int x = 0; x < CHUNK_SIZE;x++) {
        int xindex = (chunkx*CHUNK_SIZE)+x;
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int zindex = (chunkz*CHUNK_SIZE)+z;
            map.SetBrick(xindex,zindex,2,map.IdFromName("bedrock"));
        }
    }

    curStage = BUILD_STAGE;

    bGen = true;
}