#ifndef __XCB_RNG_H__
#define __XCB_RNG_H__

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef signed char schar;

#define XCB_RNG_COEFF 4164903690U

/*!
   Random Number Generator

   The class implements XCB_RNG using Multiply-with-Carry algorithm
*/
class XCB_RNG {
public:
  enum { UNIFORM = 0, NORMAL = 1 };

  XCB_RNG() { state = 0xffffffff; }
  XCB_RNG(unsigned long long _state) { state = _state ? _state : 0xffffffff; }
  //! updates the state and returns the next 32-bit unsigned integer random
  //! number
  unsigned next() {
    state = (unsigned long long)(unsigned)state * XCB_RNG_COEFF +
            (unsigned)(state >> 32);
    return (unsigned)state;
  }

  operator uchar() { return (uchar)next(); }
  operator schar() { return (schar)next(); }
  operator ushort() { return (ushort)next(); }
  operator short() { return (short)next(); }
  operator unsigned() { return next(); }
  //! returns a random integer sampled uniformly from [0, N).
  unsigned operator()(unsigned N) { return (unsigned)uniform(0, N); }
  unsigned operator()() { return next(); }
  operator int() { return (int)next(); }
  // * (2^32-1)^-1
  operator float() { return next() * 2.3283064365386962890625e-10f; }
  operator double() {
    unsigned t = next();
    return (((unsigned long long)t << 32) | next()) *
           5.4210108624275221700372640043497e-20;
  }
  //! returns uniformly distributed integer random number from [a,b) range
  int uniform(int a, int b) { return a == b ? a : (int)(next() % (b - a) + a); }
  //! returns uniformly distributed floating-point random number from [a,b)
  //! range
  float uniform(float a, float b) { return ((float)*this) * (b - a) + a; }
  //! returns uniformly distributed double-precision floating-point random
  //! number from [a,b) range
  double uniform(double a, double b) { return ((double)*this) * (b - a) + a; }

  //! returns Gaussian random variate with mean zero.
  //  double gaussian(double sigma);

  unsigned long long state;
};

#endif /*__XCB_RNG_H__*/
