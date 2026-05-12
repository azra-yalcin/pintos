#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

/* 17.14 Fixed-Point Arithmetic
   ==============================
   Integer bits : 17
   Fraction bits: 14
   Scale factor : f = 2^14 = 16384

   Representation: real number x -> integer (x * f)
*/

#include <stdint.h>

typedef int32_t fixed_point_t;

#define FP_SHIFT 14
#define FP_F    (1 << FP_SHIFT)   /* 16384 */

/* Convert integer to fixed-point. */
static inline fixed_point_t
fp_from_int (int n)
{
  return n * FP_F;
}

/* Convert fixed-point to integer (truncate toward zero). */
static inline int
fp_to_int_zero (fixed_point_t x)
{
  return x / FP_F;
}

/* Convert fixed-point to integer (round to nearest). */
static inline int
fp_to_int_nearest (fixed_point_t x)
{
  if (x >= 0)
    return (x + FP_F / 2) / FP_F;
  else
    return (x - FP_F / 2) / FP_F;
}

/* Add two fixed-point numbers. */
static inline fixed_point_t
fp_add (fixed_point_t x, fixed_point_t y)
{
  return x + y;
}

/* Subtract two fixed-point numbers. */
static inline fixed_point_t
fp_sub (fixed_point_t x, fixed_point_t y)
{
  return x - y;
}

/* Add fixed-point and integer. */
static inline fixed_point_t
fp_add_int (fixed_point_t x, int n)
{
  return x + n * FP_F;
}

/* Subtract integer from fixed-point. */
static inline fixed_point_t
fp_sub_int (fixed_point_t x, int n)
{
  return x - n * FP_F;
}

/* Multiply two fixed-point numbers. */
static inline fixed_point_t
fp_mul (fixed_point_t x, fixed_point_t y)
{
  return (fixed_point_t) (((int64_t) x * y) / FP_F);
}

/* Multiply fixed-point by integer. */
static inline fixed_point_t
fp_mul_int (fixed_point_t x, int n)
{
  return x * n;
}

/* Divide two fixed-point numbers. */
static inline fixed_point_t
fp_div (fixed_point_t x, fixed_point_t y)
{
  return (fixed_point_t) (((int64_t) x * FP_F) / y);
}

/* Divide fixed-point by integer. */
static inline fixed_point_t
fp_div_int (fixed_point_t x, int n)
{
  return x / n;
}

#endif /* threads/fixed-point.h */
