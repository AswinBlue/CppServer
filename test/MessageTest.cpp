#include "Message.h"
using namespace std;

enum class Sample : uint32_t 
{
    First,
    Second,
    Third
};

int main(void) {
    net::message<Sample> msg;
    msg.header.id = Sample::Second;

    int a = 3;
    bool b = true;
    float c = 3.141592f;
    struct
    {
        float x;
        long y;
    }d[5];

    cout << a << " " << b << " " << c << " " << d << endl;
    msg << a << b << c << d;

    a =99;
    b = false;
    c = 0.1f;

    msg >> d >> c >> b >> a;

    cout << a << " " << b << " " << c << " " << d << endl;

    return 0;

}
