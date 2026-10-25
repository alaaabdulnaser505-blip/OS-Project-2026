#ifndef INC_FIXED_POINT_H
#define INC_FIXED_POINT_H

#include <inc/assert.h>
#include <inc/string.h>

/* Parameters. */
#define FIX_BITS 32             /* Total bits per fixed-point number. */
#define FIX_P 17                /* Number of integer bits. */
#define FIX_Q 14                /* Number of fractional bits. */
#define FIX_F (1 << FIX_Q)      /* pow(2, FIX_Q). */

#define FIX_MIN_INT (-FIX_MAX_INT)      /* Smallest representable integer. */
#define FIX_MAX_INT ((1 << FIX_P) - 1)  /* Largest representable integer. */

/* A fixed-point number. */
typedef struct
  {
    int f;
  }
fixed_point_t;

/* Returns a fixed-point number with F as its internal value. */
static inline fixed_point_t
__mk_fix (int f)
{
  fixed_point_t x;
  x.f = f;
  return x;
}

/* Returns fixed-point number corresponding to integer N. */
static inline fixed_point_t
fix_int (int n)
{
  assert(n >= FIX_MIN_INT && n <= FIX_MAX_INT);
  return __mk_fix (n * FIX_F);
}

/* Returns fixed-point number corresponding to N divided by D. */
static inline fixed_point_t
fix_frac (int n, int d)
{
	assert (d != 0);
	assert (n / d >= FIX_MIN_INT && n / d <= FIX_MAX_INT);
  return __mk_fix ((long long) n * FIX_F / d);
}

/* Returns X rounded to the nearest integer. */
static inline int
fix_round (fixed_point_t x)
{
	if (x.f >= 0)
		return (x.f + FIX_F / 2) / FIX_F;
	else
		return (x.f - FIX_F / 2) / FIX_F;
}

/* Returns X truncated down to the nearest integer. */
static inline int
fix_trunc (fixed_point_t x)
{
  return x.f / FIX_F;
}

/* Returns X + Y. */
static inline fixed_point_t
fix_add (fixed_point_t x, fixed_point_t y)
{
  return __mk_fix (x.f + y.f);
}

/* Returns X - Y. */
static inline fixed_point_t
fix_sub (fixed_point_t x, fixed_point_t y)
{
  return __mk_fix (x.f - y.f);
}

/* Returns X * Y. */
static inline fixed_point_t
fix_mul (fixed_point_t x, fixed_point_t y)
{
  return __mk_fix ((long long) x.f * y.f / FIX_F);
}

/* Returns X * N. */
static inline fixed_point_t
fix_scale (fixed_point_t x, int n)
{
	assert (n >= 0);
  return __mk_fix (x.f * n);
}

/* Returns X / Y. */
static inline fixed_point_t
fix_div (fixed_point_t x, fixed_point_t y)
{
  return __mk_fix ((long long) x.f * FIX_F / y.f);
}

/* Returns X / N. */
static inline fixed_point_t
fix_unscale (fixed_point_t x, int n)
{
  assert (n > 0);
  return __mk_fix (x.f / n);
}

/* Returns 1 / X. */
static inline fixed_point_t
fix_inv (fixed_point_t x)
{
  return fix_div (fix_int (1), x);
}

/* Returns -1 if X < Y, 0 if X == Y, 1 if X > Y. */
static inline int
fix_compare (fixed_point_t x, fixed_point_t y)
{
  return x.f < y.f ? -1 : x.f > y.f;
}

/*2023*/
static inline void fixedPt2Str(fixed_point_t f, int num_dec_digits, char* output)
{
	int mulFactor = 1;
	for (int i = 0; i < num_dec_digits; ++i) {
		mulFactor *= 10;
	}
	int scaledVal = fix_round(fix_scale(f, mulFactor)) ;
	int integer = scaledVal/mulFactor;
	int fraction = scaledVal%mulFactor;
	char intPart[20] ; ltostr(integer, intPart);
	char fractPart[20] ; ltostr(fraction, fractPart);
	int tmp = mulFactor / 10;

	char zeros[10] = "";
	while (fraction < tmp)
	{
		strcconcat("0", zeros, zeros);
		tmp /= 10;
	}
	char fractPart2[20];
	strcconcat(zeros, fractPart, fractPart2);

	//cprintf("integer = %d, intPart = %s - fraction = %d, fractPart = %s\n", integer, intPart, fraction , fractPart2);
	strcconcat(intPart, ".", intPart);
	strcconcat(intPart, fractPart2, output);

}

#endif /* inc/fixed_point.h */
