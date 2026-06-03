/*
Playing with complex M31 prime field based Schnorr signatures

Copyright 2026 Ahmet Inan <xdsopl@gmail.com>
*/

#include <cassert>
#include <iostream>
#include "prime_field.hh"
#include "complex_field.hh"

typedef CODE::PrimeField<uint32_t, 0x7FFFFFFF> M31;
typedef CODE::ComplexField<M31> CM31;

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	CM31 G(M31(2), M31(1268011823));
	assert(norm(G) == M31(1));
	uint32_t order = M31::P + 1;
	assert(pow(G, order / 2) == -CM31(M31(1)));
	assert(pow(G, order) == CM31(M31(1)));
	if (0) {
		CM31 tmp(G);
		for (uint32_t i = 1; i < order; ++i)
			assert(norm(tmp *= G) == M31(1) && tmp != G);
		tmp *= G;
		assert(tmp == G);
	}
	return 0;
}

