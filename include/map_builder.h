#ifndef MAP_BUILDER_H
#define MAP_BUILDER_H

#include "map.h"

class MapBuilder {
    public:
        MapBuilder(Map &map);
        ~MapBuilder();

        void Build();
    private:
        Map &myMap;
};


#endif
