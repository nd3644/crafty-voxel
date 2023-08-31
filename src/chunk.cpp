#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <libnoise/noise.h>
#include <gsl/gsl_spline.h>

#include "map.h"

#include <cassert>

/* const int numPoints = 6;
double ax[numPoints] = { -1, -0.8, 0.3, 0.5, 0.8, 1.0 };
double ay[numPoints] = { 20, 05, 80, 90, 110, 256 }; */


std::vector<double>ax = { -1, -0.9, -0.8, -0.5, 0.0, 0.5, 0.8, 1 };
std::vector<double>ay = { 25, 30, 100, 105, 130, 150, 200, 256 };

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

Map::chunk_t::chunk_t() {
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
}

Map::chunk_t::~chunk_t() {
}

void Map::chunk_t::PushLights(Map &map) {
//            std::cout << "pushlen: " << lightList.size() << std::endl;
    for(light_t &l: lightList) {
        pushedLights.push_back(l);
        map.AddLight(l.x, l.z, l.y, true);
    }
}

void Map::chunk_t::PopLights(Map &map) {
    for(light_t &l: pushedLights) {
        map.AddLight(l.x, l.z, l.y, false);
    }
    pushedLights.clear();
}

void Map::chunk_t::Generate(int chunkx, int chunkz, Map &map) {
    if(bGen || bIsCurrentlyGenerating)
        return;

    const int SEA_LEVEL = 63;

    bIsCurrentlyGenerating = true;
    bool bMount = (rand()%5 == 1) ? true : false;

    int brickType = 1;
    if(bMount) {
        brickType = 1;
    }

 //   std::cout << "generating " << chunkx << " , " << chunkz << " ";

//    constexpr int HEAT_PERLIN_SEED = 4321;

    using namespace noise;

    double FREQ = 0.0025;

//    module::Perlin normalPerlin;
    module::Perlin normalPerlin, erosionPerlin;
    module::Perlin heatPerlin;
    heatPerlin.SetSeed(5331);
    heatPerlin.SetFrequency(FREQ / 4.0);

    normalPerlin.SetFrequency(FREQ);
    erosionPerlin.SetFrequency(FREQ / 2.0);

    heatShift = 0;//heatPerlin.GetValue(((chunkx*CHUNK_SIZE)+8), ((chunkz*CHUNK_SIZE)+8), 0.5f) / 12.0f;

    module::ScaleBias scaled;
    scaled.SetSourceModule(0, normalPerlin);
    scaled.SetScale(0.7);
    scaled.SetBias(-0.3);

    std::vector<vec3_t>toFill;
    for (int x = 0; x < CHUNK_SIZE;x++) {
        int xindex = (chunkx*CHUNK_SIZE)+x;
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int zindex = (chunkz*CHUNK_SIZE)+z;
            float unit = scaled.GetValue((float)xindex,(float)zindex,0.5);
            int height = 0;//(((unit + 1) / 2) * MAX_HEIGHT);

            normalPerlin.SetOctaveCount(noise::module::DEFAULT_PERLIN_OCTAVE_COUNT);

            if(heatPerlin.GetValue((float)xindex, (float)zindex, 0.5) > 0.5) {
                normalPerlin.SetOctaveCount(5);
            }

            double xValue = unit;
//            double yValue = gsl_spline_eval(spline, xValue, accel);
//            height = yValue;
            height = interpolateY(xValue);

            if(height >= MAX_HEIGHT)
                height = MAX_HEIGHT - 1;
            

            // default to grass
            brickType = map.BrickNameMap["grass_top"];

            for (int y = height; y > 0; y--) {
                brickType = map.BrickNameMap["grass_top"];
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

/*    if(rand() % 6 == 0) {
        int lX = rand() % 16;
        int lZ = rand() % 16;
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
        
    }*/


/*     module::RidgedMulti valleyNoise;
    valleyNoise.SetSeed(987);
    valleyNoise.SetFrequency(FREQ);

    module::ScaleBias scaledValley;
    scaledValley.SetSourceModule(0, valleyNoise);

    const int waterRepDepth = 15;
    for (int x = 0; x < CHUNK_SIZE;x++) {
        int xindex = (chunkx*CHUNK_SIZE)+x;
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int zindex = (chunkz*CHUNK_SIZE)+z;

            if(heatPerlin.GetValue((float)xindex, (float)zindex, 0.5) > 0.5) {
                continue;
            }

            float unit = scaledValley.GetValue((float)xindex,(float)zindex,0.5) * (float)MAX_HEIGHT;
            int depth = (int)unit;

            depth -= (scaled.GetValue((float)xindex,(float)zindex,0.5) * (float)MAX_HEIGHT);

            int yStop = (MAX_HEIGHT-depth < 0) ? 0 : MAX_HEIGHT-depth;

            int replacementDepth = 0;
            for(int y = MAX_HEIGHT;y > yStop;y--) {
                if(y < SEA_LEVEL) {
                    map.SetBrick(xindex,zindex,y,0);
                    map.SetBrick(xindex,zindex,y,map.IdFromName("water"));
                }
                else if(map.GetBrick(xindex,zindex,y) != 0) {
                    map.SetBrick(xindex,zindex,y,0);
                }
                
            }
        }
    } */

    curStage = AO_STAGE;

    bGen = true;
}