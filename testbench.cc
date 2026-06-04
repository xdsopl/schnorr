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
static const CM31 generator(M31(2), M31(1268011823));
static const uint32_t order = M31::P + 1;

bool verify(CM31 fingerprint, uint32_t scalar, uint32_t hash, uint8_t *message, int length)
{
	CM31 point_v = pow(generator, scalar) * pow(fingerprint, hash);
	uint32_t hash_v = point_v.real()() ^ point_v.imag()();
	for (int i = 0; i < length; ++i)
		hash_v ^= message[i];
	hash_v %= order;
	return hash == hash_v;
}

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	assert(norm(generator) == M31(1));
	assert(pow(generator, order / 2) == -CM31(M31(1)));
	assert(pow(generator, order) == CM31(M31(1)));
	if (0) {
		CM31 tmp(generator);
		for (uint32_t i = 1; i < order; ++i)
			assert(norm(tmp *= generator) == M31(1) && tmp != generator);
		tmp *= generator;
		assert(tmp == generator);
	}
	std::random_device rd;
	std::default_random_engine gen(rd());
	typedef std::uniform_int_distribution<int> dist;
	auto rnd_key = std::bind(dist(2, order-1), gen);
	// keypair
	uint32_t private_key = rnd_key();
	CM31 fingerprint = M31(1) / (pow(generator, private_key));
	// signing
	uint32_t nonce = rnd_key();
	CM31 point = pow(generator, nonce);
	const int length = 123;
	uint8_t message[length];
	auto rnd_dat = std::bind(dist(0, 255), gen);
	for (int i = 0; i < length; ++i)
		message[i] = rnd_dat();
	uint32_t hash = point.real()() ^ point.imag()();
	for (int i = 0; i < length; ++i)
		hash ^= message[i];
	hash %= order;
	uint32_t scalar = (nonce + (uint64_t(hash) * private_key)) % order;
	// verification
	if (!verify(fingerprint, scalar, hash, message, length)) {
		std::cerr << "verification failed!" << std::endl;
		return 1;
	}
	return 0;
}

