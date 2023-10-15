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


namespace continentalness {
    double ax[] = { -1, -0.5, -0.4, -0.1,  -0.05, 0.0,    0.4, 0.8, 1 };
    double ay[] = { 50.0 / 255.0,    51.0 / 255.0,  70.0 / 255.0,  71.0 / 255.0, 120.0 / 255.0, 125.0 / 255.0, 130.0 / 255.0, 135.0 / 255.0, 180 / 255.0 };

    double interpolateY(double x)
    {
        int numPoints = sizeof(ax) / sizeof(ax[0]);

        if(x < -1)
            x = -1;
        if(x > 1)
            x = 1;
        
        gsl_interp_accel* accel = gsl_interp_accel_alloc();
        gsl_interp* interp = gsl_interp_alloc(gsl_interp_cspline, numPoints);

        gsl_interp_init(interp, ax, ay, numPoints);
        double interpolatedY = gsl_interp_eval(interp, ax, ay, x, accel);
        interpolatedY += 0.1;
        if(interpolatedY > 1)
            interpolatedY = 1;

        gsl_interp_free(interp);
        gsl_interp_accel_free(accel);

        return interpolatedY;
    }
}

namespace erosion {
     double ax[] = { -100.0 / 100.0, -70.0 / 100.0, -35.0 / 100.0, -25.0 / 100.0, -10.0 / 100.0, 50.0 / 100.0, 80.0 / 100.0, 85.0 / 100.0, 88.0 / 100.0, 90.0 / 100.0, 100.0 / 100.0 };
     double ay[] = { 250.0 / 255.0,  200.0 / 255.0, 170.0 / 255.0, 175.0 / 255.0, 100.0 / 255.0, 90.0 / 255.0, 92.0 / 255.0, 115.0 / 255.0, 115.0 / 255.0, 92.0 / 255.0, 89.0 / 255.0 };  

    double interpolateY(double x)
    {
        int numPoints = sizeof(ax) / sizeof(ax[0]);

        if(x < -1)
            x = -1;
        if(x > 1)
            x = 1;
        
        gsl_interp_accel* accel = gsl_interp_accel_alloc();
        gsl_interp* interp = gsl_interp_alloc(gsl_interp_cspline, numPoints);

        gsl_interp_init(interp, ax, ay, numPoints);
        double interpolatedY = gsl_interp_eval(interp, ax, ay, x, accel);
        interpolatedY += 0.1;
        if(interpolatedY > 1)
            interpolatedY = 1;

        gsl_interp_free(interp);
        gsl_interp_accel_free(accel);

        return interpolatedY;
    }
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

    bIsCurrentlyGenerating = true;
    bool bMount = (rand()%5 == 1) ? true : false;

    int brickType = 1;
    if(bMount) {
        brickType = 1;
    }

//    std::cout << "generating " << chunkx << " , " << chunkz << std::endl;
//    constexpr int HEAT_PERLIN_SEED = 4321;

    using namespace noise;
//    module::Perlin normalPerlin;

    module::Perlin normalPerlin, secondPerlin;

    ConfigureContinentalness(normalPerlin);
    ConfigureErosion(secondPerlin);

    heatShift = 0;//heatPerlin.GetValue(((chunkx*CHUNK_SIZE)+8), ((chunkz*CHUNK_SIZE)+8), 0.5f) / 12.0f;

    std::vector<vec3_t>toFill;
    for (int x = 0; x < CHUNK_SIZE;x++) {
        int xindex = (chunkx*CHUNK_SIZE)+x;
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int zindex = (chunkz*CHUNK_SIZE)+z;

            double firstVal = normalPerlin.GetValue((float)xindex,(float)zindex,0.5);
            double secondVal = secondPerlin.GetValue((float)xindex,(float)zindex,0.5);

            
            double a = continentalness::interpolateY(firstVal);
            double b = erosion::interpolateY(secondVal);

            int height = 0;//(((unit + 1) / 2) * MAX_HEIGHT);

            height = (a*b) * MAX_HEIGHT;

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


void chunk_t::ConfigureContinentalness(noise::module::Perlin &module) {
    module.SetSeed(DEFAULT_CONTINENTALNESS_SEED);
    module.SetFrequency(DEFAULT_FREQUENCY);
    module.SetOctaveCount(DEFAULT_CONTINENTALNESS_OCTAVES);
}

void chunk_t::ConfigureErosion(noise::module::Perlin &module) {
    module.SetSeed(DEFAULT_EROSION_SEED);
    module.SetFrequency(DEFAULT_FREQUENCY);
}