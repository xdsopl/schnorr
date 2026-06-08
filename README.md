
### Playing with Schnorr signatures based on Edwards curve over M31 prime field

DISCLAIMER:

Don't use any of this for important stuff. I am just playing around at the moment.

Quick start:

```
make test
```

### Toy Edwards Curve over M31

An Edwards curve x² + y² = 1 + d·x²y² with **d = 7** over 𝔽_p, p = 2³¹ − 1.

### Verification in Maxima

d = 7 is a non-square mod p (completeness condition):
```maxima
jacobi(7, 2^31 - 1);
=> -1
```

Curve and twist orders:
```maxima
ifactors(1073725088);
=> [[2, 5], [33553909, 1]]

ifactors(2 * (2^31 - 1) + 2 - 1073725088);
=> [[2, 5], [100663819, 1]]
```

Both large factors are prime, cofactor is 32.
