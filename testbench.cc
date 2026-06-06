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

uint32_t compress(EC point)
{
	uint32_t sign = point.y() & 1;
	return point.x() | (sign << 31);
}

EC decompress(uint32_t val)
{
	uint32_t sign = val >> 31;
	M31 x(val & 0x7FFFFFFF);
	M31 y(EC::findY(x));
	if ((y() & 1) != sign)
		y = -y;
	return EC(x, y);
}

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	M31 x(2);
	M31 y(EC::findY(x));
	if (0) {
		EC gen(x, y), tmp(gen);
		for (uint32_t i = 1; i <= 0xFFFFFFFF; ++i) {
			tmp += gen;
			if (tmp == gen) {
				std::cerr << "order = " << i << std::endl;
				return 0;
			}
		}
	}
	//std::cerr << "EC(" << x() << ", " << y() << ")"<< std::endl;
	assert(EC(x, y) == generator);
	assert((total / 2) * generator == EC(M31(0), -M31(1)));
	assert(total * generator == EC(M31(0), M31(1)));
	if (0) {
		EC tmp(generator);
		for (uint32_t i = 1; i < total; ++i)
			assert(generator != (tmp += generator));
		tmp += generator;
		assert(tmp == generator);
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
	uint32_t fingerprint = compress(public_key);
	// verification
	if (!verify(decompress(fingerprint), scalar, check, message, length)) {
		std::cerr << "verification failed!" << std::endl;
		return 1;
	}
	return 0;
}

