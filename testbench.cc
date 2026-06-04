/*
Playing with complex M31 prime field based Schnorr signatures

Copyright 2026 Ahmet Inan <xdsopl@gmail.com>
*/

#include <random>
#include <cassert>
#include <iostream>
#include <functional>
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
	std::random_device rd;
	std::default_random_engine generator(rd());
	typedef std::uniform_int_distribution<int> distribution;
	auto rnd_key = std::bind(distribution(2, order-1), generator);
	// keypair
	uint32_t private_key = rnd_key();
	CM31 fingerprint = M31(1) / (pow(G, private_key));
	// signing
	uint32_t nonce = rnd_key();
	CM31 point = pow(G, nonce);
	const int msg_len = 123;
	uint8_t message[msg_len];
	auto rnd_dat = std::bind(distribution(0, 255), generator);
	for (int i = 0; i < msg_len; ++i)
		message[i] = rnd_dat();
	uint32_t hash = point.real()() ^ point.imag()();
	for (int i = 0; i < msg_len; ++i)
		hash ^= message[i];
	hash %= order;
	uint32_t scalar = (nonce + (uint64_t(hash) * private_key)) % order;
	// verification
	CM31 point_v = pow(G, scalar) * pow(fingerprint, hash);
	uint32_t hash_v = point_v.real()() ^ point_v.imag()();
	for (int i = 0; i < msg_len; ++i)
		hash_v ^= message[i];
	hash_v %= order;
	assert(hash == hash_v);
	return 0;
}

