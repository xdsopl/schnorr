/*
Playing with Schnorr signatures based on Edwards curve over M31 prime field

Copyright 2026 Ahmet Inan <xdsopl@gmail.com>
*/

#include <random>
#include <cassert>
#include <iostream>
#include <functional>
#include "prime_field.hh"
#include "edwards_curve.hh"

typedef CODE::PrimeField<uint32_t, 0x7FFFFFFF> M31;
typedef EdwardsCurve<M31, 7> EC;
static const EC generator(M31(2), M31(715827882));
static const EC base = 32U * generator;
static const uint32_t order = 33553909;
static const uint32_t total = order << 5;

uint32_t fnv1a_init()
{
	return 0x811C9DC5;
}

uint32_t fnv1a_update8(uint32_t prev, uint8_t data)
{
	return (prev ^ data) * 0x01000193;
}

uint32_t fnv1a_update32(uint32_t prev, uint32_t data)
{
	prev = fnv1a_update8(prev, data & 255);
	prev = fnv1a_update8(prev, (data >> 8) & 255);
	prev = fnv1a_update8(prev, (data >> 16) & 255);
	return fnv1a_update8(prev, (data >> 24) & 255);
}

uint32_t hash(EC point, const uint8_t *message, int length)
{
	uint32_t hash = fnv1a_init();
	hash = fnv1a_update32(hash, point.x());
	hash = fnv1a_update32(hash, point.y());
	for (int i = 0; i < length; ++i)
		hash = fnv1a_update8(hash, message[i]);
	return hash % order;
}

void sign(uint32_t private_key, uint32_t nonce, uint32_t *scalar, uint32_t *check, const uint8_t *message, int length)
{
	EC point = nonce * base;
	*check = hash(point, message, length);
	*scalar = (nonce + (uint64_t(*check) * private_key)) % order;
}

bool verify(EC fingerprint, uint32_t scalar, uint32_t check, const uint8_t *message, int length)
{
	EC point = (scalar * base) - (check * fingerprint);
	return check == hash(point, message, length);
}

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	if (0) {
		const int d = 7;
		typedef EdwardsCurve<M31, d> EC;
		M31 x(2);
		M31 y(EC::findY(x));
		std::cerr << "testing EC<" << d << ">(" << x() << ", " << y() << ")";
		EC gen(x, y), tmp(gen);
		for (uint32_t i = 1; i; ++i) {
			if (!(i & 0xFFFFFFF))
				std::cerr << " " << ((100 * (i >> 28)) / 16) << "%";
			tmp += gen;
			if (tmp == gen) {
				std::cerr << std::endl << "order = " << i << std::endl;
				return 0;
			}
		}
		std::cerr << std::endl << "rats!" << std::endl;
		return 1;
	}
	assert((total / 2) * generator == EC(M31(0), -M31(1)));
	assert(total * generator == EC(M31(0), M31(1)));
	if (0) {
		EC tmp(base);
		for (uint32_t i = 1; i < order; ++i)
			assert(base != (tmp += base));
		tmp += base;
		assert(tmp == base);
	}
	std::random_device rd;
	std::default_random_engine rnd_gen(rd());
	typedef std::uniform_int_distribution<int> uni_dis;
	auto rnd_key = std::bind(uni_dis(2, order-1), rnd_gen);
	// keypair
	uint32_t private_key = rnd_key();
	EC public_key = private_key * base;
	// message
	const int length = 123;
	uint8_t message[length];
	auto rnd_dat = std::bind(uni_dis(0, 255), rnd_gen);
	for (int i = 0; i < length; ++i)
		message[i] = rnd_dat();
	// signing
	uint32_t scalar, check;
	sign(private_key, rnd_key(), &scalar, &check, message, length);
	// compression
	uint32_t fingerprint = public_key.compress();
	// verification
	if (!verify(EC(fingerprint), scalar, check, message, length)) {
		std::cerr << "verification failed!" << std::endl;
		return 1;
	}
	return 0;
}

