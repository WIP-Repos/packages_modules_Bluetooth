/******************************************************************************
 *
 *  Copyright 2006-2015 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *  This file contains simple pairing algorithms using Elliptic Curve
 *  Cryptography for private public key
 *
 ******************************************************************************/
#include "security/ecc/p_256_ecc_pp.h"

#include <stdlib.h>
#include <string.h>

#include "security/ecc/multprecision.h"

namespace bluetooth {
namespace security {
namespace ecc {

const uint32_t* modp = curve_p256.p;

static void p_256_init_point(Point* q) {
  memset(q, 0, sizeof(Point));
}

static void p_256_copy_point(Point* q, const Point* p) {
  memcpy(q, p, sizeof(Point));
}

// q=2q
static void ECC_Double(Point* q, const Point* p) {
  uint32_t t1[KEY_LENGTH_DWORDS_P256];
  uint32_t t2[KEY_LENGTH_DWORDS_P256];
  uint32_t t3[KEY_LENGTH_DWORDS_P256];
  const uint32_t* x1;
  uint32_t* x3;
  const uint32_t* y1;
  uint32_t* y3;
  const uint32_t* z1;
  uint32_t* z3;

  if (multiprecision_iszero(p->z)) {
    multiprecision_init(q->z);
    return;  // return infinity
  }

  x1 = p->x;
  y1 = p->y;
  z1 = p->z;
  x3 = q->x;
  y3 = q->y;
  z3 = q->z;

  multiprecision_mersenns_squa_mod(t1, z1, modp);      // t1=z1^2
  multiprecision_sub_mod(t2, x1, t1, modp);            // t2=x1-t1
  multiprecision_add_mod(t1, x1, t1, modp);            // t1=x1+t1
  multiprecision_mersenns_mult_mod(t2, t1, t2, modp);  // t2=t2*t1
  multiprecision_lshift_mod(t3, t2, modp);
  multiprecision_add_mod(t2, t3, t2, modp);  // t2=3t2

  multiprecision_mersenns_mult_mod(z3, y1, z1, modp);  // z3=y1*z1
  multiprecision_lshift_mod(z3, z3, modp);

  multiprecision_mersenns_squa_mod(y3, y1, modp);  // y3=y1^2
  multiprecision_lshift_mod(y3, y3, modp);
  multiprecision_mersenns_mult_mod(t3, y3, x1, modp);  // t3=y3*x1=x1*y1^2
  multiprecision_lshift_mod(t3, t3, modp);
  multiprecision_mersenns_squa_mod(y3, y3, modp);  // y3=y3^2=y1^4
  multiprecision_lshift_mod(y3, y3, modp);

  multiprecision_mersenns_squa_mod(x3, t2, modp);      // x3=t2^2
  multiprecision_lshift_mod(t1, t3, modp);             // t1=2t3
  multiprecision_sub_mod(x3, x3, t1, modp);            // x3=x3-t1
  multiprecision_sub_mod(t1, t3, x3, modp);            // t1=t3-x3
  multiprecision_mersenns_mult_mod(t1, t1, t2, modp);  // t1=t1*t2
  multiprecision_sub_mod(y3, t1, y3, modp);            // y3=t1-y3
}

// q=q+p,     zp must be 1
static void ECC_Add(Point* r, Point* p, const Point* q) {
  uint32_t t1[KEY_LENGTH_DWORDS_P256];
  uint32_t t2[KEY_LENGTH_DWORDS_P256];
  uint32_t* x1;
  const uint32_t* x2;
  uint32_t* x3;
  uint32_t* y1;
  const uint32_t* y2;
  uint32_t* y3;
  uint32_t* z1;
  const uint32_t* z2;
  uint32_t* z3;

  x1 = p->x;
  y1 = p->y;
  z1 = p->z;
  x2 = q->x;
  y2 = q->y;
  z2 = q->z;
  x3 = r->x;
  y3 = r->y;
  z3 = r->z;

  // if Q=infinity, return p
  if (multiprecision_iszero(z2)) {
    p_256_copy_point(r, p);
    return;
  }

  // if P=infinity, return q
  if (multiprecision_iszero(z1)) {
    p_256_copy_point(r, q);
    return;
  }

  multiprecision_mersenns_squa_mod(t1, z1, modp);      // t1=z1^2
  multiprecision_mersenns_mult_mod(t2, z1, t1, modp);  // t2=t1*z1
  multiprecision_mersenns_mult_mod(t1, x2, t1, modp);  // t1=t1*x2
  multiprecision_mersenns_mult_mod(t2, y2, t2, modp);  // t2=t2*y2

  multiprecision_sub_mod(t1, t1, x1, modp);  // t1=t1-x1
  multiprecision_sub_mod(t2, t2, y1, modp);  // t2=t2-y1

  if (multiprecision_iszero(t1)) {
    if (multiprecision_iszero(t2)) {
      ECC_Double(r, q);
      return;
    } else {
      multiprecision_init(z3);
      return;  // return infinity
    }
  }

  multiprecision_mersenns_mult_mod(z3, z1, t1, modp);  // z3=z1*t1
  multiprecision_mersenns_squa_mod(y3, t1, modp);      // t3=t1^2
  multiprecision_mersenns_mult_mod(z1, y3, t1, modp);  // t4=t3*t1
  multiprecision_mersenns_mult_mod(y3, y3, x1, modp);  // t3=t3*x1
  multiprecision_lshift_mod(t1, y3, modp);             // t1=2*t3
  multiprecision_mersenns_squa_mod(x3, t2, modp);      // x3=t2^2
  multiprecision_sub_mod(x3, x3, t1, modp);            // x3=x3-t1
  multiprecision_sub_mod(x3, x3, z1, modp);            // x3=x3-t4
  multiprecision_sub_mod(y3, y3, x3, modp);            // t3=t3-x3
  multiprecision_mersenns_mult_mod(y3, y3, t2, modp);  // t3=t3*t2
  multiprecision_mersenns_mult_mod(z1, z1, y1, modp);  // t4=t4*t1
  multiprecision_sub_mod(y3, y3, z1, modp);
}

// Computing the Non-Adjacent Form of a positive integer
static void ECC_NAF(uint8_t* naf, uint32_t* NumNAF, uint32_t* k) {
  uint32_t sign;
  int i = 0;
  int j;
  uint32_t var;

  while ((var = multiprecision_most_signbits(k)) >= 1) {
    if (k[0] & 0x01)  // k is odd
    {
      sign = (k[0] & 0x03);  // 1 or 3

      // k = k-naf[i]
      if (sign == 1)
        k[0] = k[0] & 0xFFFFFFFE;
      else {
        k[0] = k[0] + 1;
        if (k[0] == 0)  // overflow
        {
          j = 1;
          do {
            k[j]++;
          } while (k[j++] == 0);  // overflow
        }
      }
    } else
      sign = 0;

    multiprecision_rshift(k, k);
    naf[i / 4] |= (sign) << ((i % 4) * 2);
    i++;
  }

  *NumNAF = i;
}

// Binary Non-Adjacent Form for point multiplication
void ECC_PointMult_Bin_NAF(Point* q, const Point* p, uint32_t* n) {
  uint32_t sign;
  uint8_t naf[256 / 4 + 1];
  uint32_t NumNaf;
  Point minus_p;
  Point r;

  p_256_init_point(&r);

  // initialization
  p_256_init_point(q);

  // -p
  multiprecision_copy(minus_p.x, p->x);
  multiprecision_sub(minus_p.y, modp, p->y);

  multiprecision_init(minus_p.z);
  minus_p.z[0] = 1;

  // NAF
  memset(naf, 0, sizeof(naf));
  ECC_NAF(naf, &NumNaf, n);

  for (int i = NumNaf - 1; i >= 0; i--) {
    p_256_copy_point(&r, q);
    ECC_Double(q, &r);
    sign = (naf[i / 4] >> ((i % 4) * 2)) & 0x03;

    if (sign == 1) {
      p_256_copy_point(&r, q);
      ECC_Add(q, &r, p);
    } else if (sign == 3) {
      p_256_copy_point(&r, q);
      ECC_Add(q, &r, &minus_p);
    }
  }

  multiprecision_inv_mod(minus_p.x, q->z, modp);
  multiprecision_mersenns_squa_mod(q->z, minus_p.x, modp);
  multiprecision_mersenns_mult_mod(q->x, q->x, q->z, modp);
  multiprecision_mersenns_mult_mod(q->z, q->z, minus_p.x, modp);
  multiprecision_mersenns_mult_mod(q->y, q->y, q->z, modp);
}

bool ECC_ValidatePoint(const Point& pt) {
  // Ensure y^2 = x^3 + a*x + b (mod p); a = -3

  // y^2 mod p
  uint32_t y2_mod[KEY_LENGTH_DWORDS_P256] = {0};
  multiprecision_mersenns_squa_mod(y2_mod, (uint32_t*)pt.y, modp);

  // Right hand side calculation
  uint32_t rhs[KEY_LENGTH_DWORDS_P256] = {0};
  multiprecision_mersenns_squa_mod(rhs, (uint32_t*)pt.x, modp);
  uint32_t three[KEY_LENGTH_DWORDS_P256] = {0};
  three[0] = 3;
  multiprecision_sub_mod(rhs, rhs, three, modp);
  multiprecision_mersenns_mult_mod(rhs, rhs, (uint32_t*)pt.x, modp);
  multiprecision_add_mod(rhs, rhs, curve_p256.b, modp);

  return multiprecision_compare(rhs, y2_mod) == 0;
}

}  // namespace ecc
}  // namespace security
}  // namespace bluetooth
