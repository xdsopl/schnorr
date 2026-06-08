/*
Playing with complex M31 prime field based Schnorr signatures

Copyright 2026 Ahmet Inan <xdsopl@gmail.com>
*/

#include <cmath>
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

uint32_t hash(CM31 point, const uint8_t *message, int length)
{
	uint32_t hash = fnv1a_init();
	hash = fnv1a_update32(hash, point.real()());
	hash = fnv1a_update32(hash, point.imag()());
	for (int i = 0; i < length; ++i)
		hash = fnv1a_update8(hash, message[i]);
	return hash % order;
}

void sign(uint32_t private_key, uint32_t nonce, uint32_t *scalar, uint32_t *check, const uint8_t *message, int length)
{
	CM31 point = pow(generator, nonce);
	*check = hash(point, message, length);
	*scalar = (nonce + (uint64_t(*check) * private_key)) % order;
}

bool verify(CM31 fingerprint, uint32_t scalar, uint32_t check, const uint8_t *message, int length)
{
	CM31 point = pow(generator, scalar) * pow(conj(fingerprint), check);
	return check == hash(point, message, length);
}

uint32_t compress(CM31 point)
{
	uint32_t sign = point.imag()() & 1;
	return point.real()() | (sign << 31);
}

CM31 decompress(uint32_t val)
{
	uint32_t sign = val >> 31;
	M31 real(val & 0x7FFFFFFF);
	M31 imag(pow(M31(1) - (real * real), 1U << 29));
	if ((imag() & 1) != sign)
		imag = -imag;
	return CM31(real, imag);
}

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	M31 real(2);
	M31 imag(pow(M31(1) - (real * real), 1U << 29));
	assert(CM31(real, imag) == generator);
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
	CM31 public_key = pow(generator, private_key);
	// message
	const int length = 123;
	uint8_t message[length];
	auto rnd_dat = std::bind(dist(0, 255), gen);
	for (int i = 0; i < length; ++i)
		message[i] = rnd_dat();
	// signing
	uint32_t scalar, check;
	sign(private_key, rnd_key(), &scalar, &check, message, length);
	// compression
	uint32_t compressed = compress(public_key);
	// decompression
	CM31 fingerprint = decompress(compressed);
	// verification
	if (!verify(fingerprint, scalar, check, message, length)) {
		std::cerr << "verification failed!" << std::endl;
		return 1;
	}
	// breaking
	if (0) {
		CM31 tmp(generator);
		for (uint32_t i = 2; i < order; ++i) {
			if (public_key == (tmp *= generator)) {
				assert(private_key == i);
				std::cerr << "found private key after " << i << " iterations" << std::endl;
				return 0;
			}
		}
		std::cerr << "huh?" << std::endl;
	}
	// BSGS
	if (0) {
		const int m = (int)std::ceil(std::sqrt((double)order));
		assert(m <= 65536);
		uint64_t *bs_tmp = new uint64_t[m];
		CM31 baby(M31(1));
		for (int i = 0; i < m; ++i) {
			bs_tmp[i] = uint64_t(i) << 32 | compress(baby);
			baby *= generator;
		}
		CM31 giant = conj(baby);
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
		CM31 gamma(public_key);
		for (int i = 0; i < m; ++i) {
			uint32_t val = compress(gamma);
			int idx = index(val);
			int ptr = bs_ptr[idx];
			int cnt = bs_cnt[idx];
			for (int j = ptr; j < ptr + cnt; ++j) {
				if (uint32_t(bs_val[j] & 0xFFFFFFFF) == val) {
					uint32_t num = (bs_val[j] >> 32) & 65535;
					uint32_t key = (uint64_t(i) * m + num) % order;
					assert(private_key == key);
					std::cerr << "found private key after " << m << " baby and " << i << " giant steps" << std::endl;
					goto end;
				}
			}
			gamma *= giant;
		}
		std::cerr << "what?" << std::endl;
end:
		delete[] bs_val;
		delete[] bs_cnt;
		delete[] bs_ptr;
	}
	return 0;
}

