#ifndef __MAP_H__
#define __MAP_H__

using namespace std;

int Map::loadMapFromFile(char* file) {
    //TODO load from file

    // testCode
    this->geography.map = new int[1024 * 1024];
}

int Map::updateUser(int userId, int newX, int newY) {
    //TODO : find user, change user

}

int* Map::getMap() {
    return this->geography.map;
}

#endif
