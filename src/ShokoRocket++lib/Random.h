#pragma once

class Random
{
private:
	Random(void);
public:
	~Random(void);

	static float RandomRange(float _min, float _max);
	static float RandomCentered(float _center, float _range);
	static float RandomFactor();
	static float RandomFactorPM();
	static bool RandomChance(float _chance);
	static void Seed();
	static void Seed(unsigned int _seed);
	static int RandomIndex(int _array_size);
	static int RandomQuantity(int _min, int _max);
};
