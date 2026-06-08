/*
Playing with Schnorr signatures based on Edwards curve over M31 prime field

Copyright 2026 Ahmet Inan <xdsopl@gmail.com>
*/

#include <cmath>
#include <random>
#include <cassert>
#include <iostream>
#include <functional>
#include "prime_field.hh"
#include "edwards_curve.hh"

typedef CODE::PrimeField<uint32_t, 0x7FFFFFFF> M31;
typedef EdwardsCurve<M31, 7> EC;
static const EC generator(M31(2), M31(715827882));
static const int cofactor_power = 5;
static const uint32_t cofactor = 1 << cofactor_power;
static const EC base = cofactor * generator;
static const uint32_t order = 33553909;
static const uint32_t total = order << cofactor_power;

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
	EC point = (scalar * base) + (check * fingerprint);
	return check == hash(point, message, length);
}

template <typename EC>
int64_t bsgs(EC base, EC target, int64_t bound)
{
	const int m = (int)std::ceil(std::sqrt((double)bound));
	assert(m <= 65536);
	uint64_t *bs_tmp = new uint64_t[m];
	EC baby;
	for (int i = 0; i < m; ++i) {
		bs_tmp[i] = uint64_t(i) << 32 | baby.compress();
		baby += base;
	}
	EC giant = -baby;
	auto index = [](uint32_t val) { return ((val >> 16) ^ val) & 65535; };
	uint8_t *bs_cnt = new uint8_t[65536];
	for (int i = 0; i < 65536; ++i)
		bs_cnt[i] = 0;
	for (int i = 0; i < m; ++i)
		bs_cnt[index(bs_tmp[i] & 0xFFFFFFFF)]++;
	uint16_t *bs_ptr = new uint16_t[65536];
	bs_ptr[0] = 0;
	for (int i = 1; i < 65536; ++i)
		bs_ptr[i] = bs_ptr[i - 1] + bs_cnt[i - 1];
	for (int i = 0; i < 65536; ++i)
		bs_cnt[i] = 0;
	uint64_t *bs_val = new uint64_t[m];
	for (int i = 0; i < m; ++i) {
		int idx = index(bs_tmp[i] & 0xFFFFFFFF);
		bs_val[bs_ptr[idx] + bs_cnt[idx]++] = bs_tmp[i];
	}
	delete[] bs_tmp;
	EC gamma(target);
	for (int i = 0; i < m; ++i) {
		uint32_t val = gamma.compress();
		int idx = index(val);
		int ptr = bs_ptr[idx];
		int cnt = bs_cnt[idx];
		for (int j = ptr; j < ptr + cnt; ++j) {
			if (uint32_t(bs_val[j] & 0xFFFFFFFF) == val) {
				uint32_t num = (bs_val[j] >> 32) & 65535;
				delete[] bs_val;
				delete[] bs_cnt;
				delete[] bs_ptr;
				std::cerr << "found target after " << m << " baby and " << i << " giant steps" << std::endl;
				return int64_t(i) * m + num;
			}
		}
		gamma += giant;
	}
	std::cerr << "what?" << std::endl;
	delete[] bs_val;
	delete[] bs_cnt;
	delete[] bs_ptr;
	return -1;
}

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	if (0) {
		const int d = 7;
		assert(pow(M31(d), (M31::P-1)/2) == -M31(1));
		typedef EdwardsCurve<M31, d> EC;
		M31 x(2);
		M31 y(EC::findY(x));
		std::cerr << "testing EC<" << d << ">(" << x() << ", " << y() << ")" << std::endl;
		int64_t bound = M31::P + 1 + (int64_t)std::ceil(2 * std::sqrt((double)M31::P));
		int64_t fourth = bsgs(EC(x, y), EC(-M31(1), M31(0)), bound / 4);
		assert(fourth >= 0);
		int order = 4 * fourth;
		std::cerr << std::endl << "order = " << order << " twist = " << (2 * (M31::P + 1LL) - order) << std::endl;
		return 0;
	}
	assert((total / 4) * generator == EC(-M31(1), M31(0)));
	assert((total / 2) * generator == EC(M31(0), -M31(1)));
	assert(((total / 4) * 3) * generator == EC(M31(1), M31(0)));
	assert(total * generator == EC());
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
	EC public_key = -(private_key * base);
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
	uint32_t compressed = public_key.compress();
	// decompression
	EC fingerprint(compressed);
	// validation
	assert(fingerprint.isValid());
	assert(order * fingerprint == EC());
	assert(scalar < order);
	// verification
	if (!verify(fingerprint, scalar, check, message, length)) {
		std::cerr << "verification failed!" << std::endl;
		return 1;
	}
	// breaking
	if (0) {
		EC tmp(base);
		for (uint32_t i = 2; i < order; ++i) {
			if (-public_key == (tmp += base)) {
				assert(private_key == i);
				std::cerr << "found private key after " << i << " iterations" << std::endl;
				return 0;
			}
		}
		std::cerr << "huh?" << std::endl;
	}
	// BSGS
	if (0) {
		uint64_t key = bsgs(base, -public_key, order);
		assert(key >= 2 && key < order);
		assert(private_key == uint32_t(key));
	}
	return 0;
}

