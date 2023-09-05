#ifndef POSITION_H_
#define POSITION_H_

typedef struct position {
    int ln, col;
    position() : ln(1), col(1) {};
    position(int l, int c) : ln(l), col(c) {};
} position_t;

#endif // POSITION_H_