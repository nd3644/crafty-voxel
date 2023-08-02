#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <libnoise/noise.h>
#include <gsl/gsl_spline.h>

#include "map.h"

const int numPoints = 6;
double ax[numPoints] = { -1, -0.8, 0.3, 0.5, 0.8, 1.0 };
double ay[numPoints] = { 30, 10, 100, 110, 130, 200 };

double interpolateY(double x)
{
    if(x < -1)
        x = -1;
    if(x > 1)
        x = 1;
    
    gsl_interp_accel* accel = gsl_interp_accel_alloc();
    gsl_interp* interp = gsl_interp_alloc(gsl_interp_linear, numPoints);

    gsl_interp_init(interp, ax, ay, numPoints);
    double interpolatedY = gsl_interp_eval(interp, ax, ay, x, accel);

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
                        ambientVecs[i][y][j][f][v] = 1;
                    }
                }
            }
        }
    }
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

    std::cout << "generating " << chunkx << " , " << chunkz << " ";

    static int counter = 0;

    using namespace noise;

    module::Perlin normalPerlin;
    normalPerlin.SetFrequency(0.2);

    module::Perlin baseFlatTerrain;

    module::ScaleBias flatTerrain;
    flatTerrain.SetSourceModule(0, baseFlatTerrain);
    flatTerrain.SetScale(0.125);

    module::RidgedMulti mountainTerrain;
    mountainTerrain.SetLacunarity(1.5);
    mountainTerrain.SetOctaveCount(1);

    module::Perlin terrainType;
    terrainType.SetFrequency(0.2);
    terrainType.SetPersistence(0.25);

    module::Select finalTerrain;
    finalTerrain.SetSourceModule(0, flatTerrain);
    finalTerrain.SetSourceModule(1, mountainTerrain);
    finalTerrain.SetControlModule(terrainType);
    finalTerrain.SetBounds(0.0, 1000.0);
    finalTerrain.SetEdgeFalloff(0.125);

    std::vector<vec3_t>toFill;
    for (int x = 0; x < CHUNK_SIZE;x++) {
        int xindex = (chunkx*CHUNK_SIZE)+x;
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int zindex = (chunkz*CHUNK_SIZE)+z;
            float unit = normalPerlin.GetValue((float)xindex / 100.0f,(float)zindex / 100.0f,0.5);
            int height = 0;//(((unit + 1) / 2) * MAX_HEIGHT);

            double xValue = unit;
//            double yValue = gsl_spline_eval(spline, xValue, accel);
//            height = yValue;
            height = interpolateY(xValue);

            if(height >= MAX_HEIGHT)
                height = MAX_HEIGHT - 1;
            
            for (int y = height; y > 0; y--) {
				map.SetBrick(xindex, zindex, y, brickType);

                if(y > 96) {
                    map.SetBrick(xindex, zindex, y, map.BrickNameMap["stone"]);
                }
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

    curStage = AO_STAGE;

    bGen = true;
}