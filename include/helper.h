#ifndef HELPER_H
#define HELPER_H

template <typename T>
T ClampValue(T value, T lower, T upper) {
    if(value < lower)
        return lower;
    else if(value > upper)
        return upper;
    return value;
}


#endif
