#include "Random.h"
#include <cstdlib>
#include <ctime>

Random::Random(void)
{
}

Random::~Random(void)
{
}

void Random::Seed()
{
	srand(static_cast<unsigned int>(time(NULL)));
}

void Random::Seed(unsigned int _seed)
{
	srand(_seed);
}

float Random::RandomRange(float _min, float _max)
{
	return _min + (_max - _min) * Random::RandomFactor();
}

float Random::RandomCentered(float _center, float _range)
{
	return _center + (_range * Random::RandomFactor() * (rand() % 2 ? 1 : -1));
}

float Random::RandomFactor()
{
	return (float)rand() / (float)RAND_MAX;
}

float Random::RandomFactorPM()
{
	return ((float)rand() / (float)RAND_MAX) * (rand() % 2 ? 1 : -1);
}

bool Random::RandomChance(float _chance)
{
	return (float)rand() / (float)RAND_MAX < _chance;
}

int Random::RandomIndex(int _array_size)
{
	return rand() % _array_size;
}

int Random::RandomQuantity(int _min, int _max)
{
	return _min + rand() % (1 + _max - _min);
}
