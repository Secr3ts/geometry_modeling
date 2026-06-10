#include <stdbool.h>
#include "myVector3D.h"
#include "myPoint3d.h"

inline bool isInArea(myPoint3D *a, myPoint3D *b, myPoint3D *c)
{
    bool r = false;

    myPoint3D ah = myPoint3D(a->X / a->Z, a->Y / a->Z, 1);
    myPoint3D bh = myPoint3D(b->X / b->Z, b->Y / b->Z, 1);
    myPoint3D ch = myPoint3D(c->X / c->Z, c->Y / c->Z, 1);

    myVector3D ab = myVector3D(bh.X - ah.X, bh.Y - ah.Y, 1);
    myVector3D ac
    
    return r;
}