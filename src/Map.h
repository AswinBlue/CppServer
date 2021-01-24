typedef struct geography{
    int size_x;
    int size_y;
    int* map;
}Geography;

class Map {
private:
    Geography geography;
public:
    Map() {}
    int loadMapFromFile(char* file);
    int updateUser(int userId, int newX, int newY);
    int* getMap();
};

