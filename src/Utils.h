#ifndef __UTILS_H__
#define __UTILS_H__

void IntToArray(int num, uint8_t* array, int len)
{
    for (int i = len - 1; i >= 0; --i)
    {
        int div = num % 10;
        num -= div;
        array[i] = div + '0';
        num /= 10;

        if (num == 0)
            break;
    }
}

#endif
