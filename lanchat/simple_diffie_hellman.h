#ifndef SIMPLEDIFFIEHELLMAN_H
#define SIMPLEDIFFIEHELLMAN_H

/**
 * @brief sqrt_floor
 * @param value
 * @return
 */
template<typename T>
T sqrt_floor(T value)
{
  static_assert(std::numeric_limits<T>::is_integer,
                "sqrt_floor is usable only with integer types!");
  if (0 >= value)
    return 0;
  if (3 >= value)
    return 1;

  T result, devider = value;
  do
    {
      result = devider;
      devider = ((value / devider + devider) >> 1);
    }
  while (result > devider);

  return result;
}

/**
 * @brief The SimpleDiffieHellman class
 */
template <typename KT>
class SimpleDiffieHellman
{
public:
  // Methods that require specialization
  static void mul(KT& r, const KT& a, const KT& b, const KT& m);
  static void createRandom(KT& p);
  static void createRandom(KT& p, const KT& m);
  static bool isPrime(const KT& p);
  static void createPrime(KT& p);

public:
  // Methods that does not require specialization
  // Now specialization realized for quint32, AKA unsigned int
  static bool isOdd(const KT& r)
  {
    return !isZero(r & 0x1);
  }

  static bool isZero(const KT& r)
  {
    return (r == 0);
  }

  static void pow(KT& r, const KT& a, const KT& b, const KT& m)
  {
    r = 1;
    KT multiplicator(a);
    KT exponent(b);
    while (!isZero(exponent))
      {
        if (isOdd(exponent))
          mul(r, r, multiplicator, m);

        exponent >>= 1;
        mul(multiplicator, multiplicator, multiplicator, m);
      }
  }

  static void prepareRequest(KT& p, KT& q, KT& a, KT& A)
  {
    while (true)
      {
        createPrime(p);
        if (isPrime((p - 1) / 2))
          break;
      }
    createRandom(q, p);
    createRandom(a, p);
    pow(A, q, a, p);
  }

  static void prepareResponse(const KT& p, const KT& q, const KT& A, KT& b,
                              KT& B, KT& K)
  {
    createRandom(b, p);
    pow(B, q, b, p);
    pow(K, A, b, p);
  }

  static void calculateKey(const KT& p, const KT& a, KT& B, KT& K)
  {
    pow(K, B, a, p);
  }
};

#endif // SIMPLEDIFFIEHELLMAN_H
