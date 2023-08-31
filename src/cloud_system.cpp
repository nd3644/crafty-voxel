#include "cloud_system.h"

#include <GL/glew.h>
#include "cube.h"
#include "mesh.h"
#include "map.h"

CloudSystem::CloudSystem() {
}

CloudSystem::~CloudSystem() {
}

void CloudSystem::Update(Camera &cam) {
    while(myClouds.size() < 50) {
        cloud_t cloud;
        cloud.x = cam.position.x;
        cloud.x += (rand() % 2048) - 512;

        cloud.z = cam.position.z;
        cloud.z += (rand() % 2048) - 512;
        myClouds.push_back(cloud);
    }

    for(std::size_t i = 0;i < myClouds.size();i++) {
        auto &cloud = myClouds[i];
        cloud.z += 0.005f;
        if(abs(cloud.z - cam.position.z) > 256) {
            myClouds.erase(myClouds.begin()+i);
        }
    }
}

void CloudSystem::Draw() {
    float CLOUD_DEFAULT_ALTITUDE = Map::MAX_HEIGHT + 8;
    ;
    glDisable(GL_CULL_FACE);
    for(auto &cloud: myClouds) {
        Mesh mesh;
        mesh.SetTranslation(cloud.x,CLOUD_DEFAULT_ALTITUDE,cloud.z);
        for(size_t i = 0;i < vertList.size();i++) {
            mesh.Index1(1);
            mesh.Vert3(vertList[i].x * cloud.w, vertList[i].y * 1, vertList[i].z * cloud.l);
            mesh.TexCoord2(uvList[i].x, uvList[i].y);
            mesh.Color4(1,1,1,1);
        }
        mesh.BindBufferData();

        mesh.Draw(Mesh::Mode::MODE_TRIANGLES);
    }
}