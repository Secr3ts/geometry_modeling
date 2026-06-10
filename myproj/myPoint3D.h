#include <math.h>
#include <string>
#include <format>
#include <sstream>

#ifndef __INCLUDEPOINT3D
#define __INCLUDEPOINT3D 

class myVector3D;

class myPoint3D
{
  //The x,y,z coordinates of this point.
public:
    union
    {
        double components[3];
        struct { double X, Y, Z; };
    };


  myPoint3D();

  myPoint3D(double x, double y, double z);
  double dist(myPoint3D p1);
  myPoint3D operator+(myVector3D & v1);
  myPoint3D operator+(myPoint3D & v1);
  myPoint3D & operator+=(myVector3D & v1);
  myPoint3D & operator+=(myPoint3D & v1);

  myPoint3D & operator/=(double d);
  myPoint3D & operator*=(double d);

  myPoint3D operator/(double d);
  myPoint3D operator*(double d);

  myVector3D operator-(myPoint3D & p1);
  void rotate(myVector3D & lp, double theta);
  void print(char *s);
  double dist(myPoint3D *p1, myPoint3D *p2);
  double dist(myPoint3D *p1, myPoint3D *p2, myPoint3D *p3);
  void circumcenter(myPoint3D *p1, myPoint3D *p2, myPoint3D *p3, myPoint3D *p4);
  
  /*
   Zeroes out a point
  */
  inline void zeroes() {
      X = 0.f;
      Y = 0.f;
      Z = 0.f;
  };

  /*
    Returns a string representation of a point
  */
  inline std::string to_s() const {
      std::ostringstream oss;
      oss << "{ x: " << X << ",y: " << Y << ",z: " << Z << "}";
      return oss.str();
  };
};

#endif


