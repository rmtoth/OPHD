#include "Population.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>


namespace {
	const int studentToScientistRate = 35;
	const int studentToAdultBase = 190;
	const int adultToRetireeBase = 2000;

	const std::array moraleModifierTable{
		MoraleModifier{50, 50, 110, 80},  // Excellent
		MoraleModifier{25, 25, 90, 75},   // Good
		MoraleModifier{0, 0, 60, 40},     // Fair
		MoraleModifier{-25, -25, 40, 20}, // Poor
		MoraleModifier{-50, -50, 20, 10}  // Terrible
	};


	std::default_random_engine pop_generator;
	std::uniform_int_distribution<int> pop_distribution(0, 99);

	auto random_0_99 = std::bind(pop_distribution, pop_generator);

	/**
	 * Convenience function to cast a MoraleLevel enumerator
	 * into an array index.
	 */
	std::size_t moraleIndex(int morale)
	{
		return static_cast<std::size_t>(std::clamp(morale, 1, 999) / 200);
	}
}


Population::Population() :
	mBirthCount(0),
	mDeathCount(0),
	mStarveRate(50)
{
	mPopulation.fill(0);
	mPopulationGrowth.fill(0);
	mPopulationDeath.fill(0);
}


/**
 * Clears entire population.
 */
void Population::clear()
{
	mPopulation.fill(0);
}


/**
 * Populates a given population with a specified number of colonists given some base values.
 *
 * \param	role		Segment of the population to populate.
 * \param	count		Base age in months of the population to populated.
 */
void Population::addPopulation(PersonRole role, int count)
{
	mPopulation[role] += count;
}


/**
 * Gets the size of the entire population.
 */
int Population::size()
{
	int count = 0;
	for (auto& populationRole : mPopulation)
	{
		count += populationRole;
	}
	return count;
}


/**
 * Gets the size of a specific segment of the population.
 */
int Population::size(PersonRole personRole)
{
	return mPopulation[personRole];
}


int Population::adults() const
{
	return mPopulation[PersonRole::ROLE_STUDENT] + mPopulation[PersonRole::ROLE_WORKER] + mPopulation[PersonRole::ROLE_SCIENTIST] + mPopulation[PersonRole::ROLE_RETIRED];
}

/**
 * Population check for new children.
 * 
 * \todo	Account for nurseries when implemented.
 */
void Population::spawn_children(int morale, int residences, int nurseries)
{
	if (residences < 1 && nurseries < 1) { return; }

	// This should be adjusted to maybe two or three kids per couple to allow for higher growth rates.
	if (mPopulation[PersonRole::ROLE_SCIENTIST] + mPopulation[PersonRole::ROLE_WORKER] > mPopulation[PersonRole::ROLE_CHILD])
	{
		mPopulationGrowth[PersonRole::ROLE_CHILD] += mPopulation[PersonRole::ROLE_SCIENTIST] / 4 + mPopulation[PersonRole::ROLE_WORKER] / 2;

		int divisor = moraleModifierTable[moraleIndex(morale)].fertilityRate;

		int newChildren = mPopulationGrowth[PersonRole::ROLE_CHILD] / divisor;
		mPopulationGrowth[PersonRole::ROLE_CHILD] = mPopulationGrowth[PersonRole::ROLE_CHILD] % divisor;

		mPopulation[PersonRole::ROLE_CHILD] += newChildren;
		mBirthCount = newChildren;
	}
}


void Population::spawn_students()
{
	if (mPopulation[PersonRole::ROLE_CHILD] > 0)
	{
		mPopulationGrowth[PersonRole::ROLE_STUDENT] += mPopulation[PersonRole::ROLE_CHILD];

		int divisor = std::max(adults(), studentToAdultBase);
		divisor = ((divisor / 40) * 3 + 16) * 4;

		int newStudents = mPopulationGrowth[PersonRole::ROLE_STUDENT] / divisor;
		mPopulationGrowth[PersonRole::ROLE_STUDENT] = mPopulationGrowth[PersonRole::ROLE_STUDENT] % divisor;

		mPopulation[PersonRole::ROLE_STUDENT] += newStudents;
		mPopulation[PersonRole::ROLE_CHILD] -= newStudents;
	}
}


void Population::spawn_adults(int universities)
{
	//-- New Adults --//
	if (mPopulation[PersonRole::ROLE_STUDENT] > 0)
	{
		mPopulationGrowth[PersonRole::ROLE_WORKER] += mPopulation[PersonRole::ROLE_STUDENT];

		int divisor = std::max(adults(), studentToAdultBase);
		divisor = ((divisor / 40) * 3 + 45) * 4;

		int newAdult = mPopulationGrowth[PersonRole::ROLE_WORKER] / divisor;
		mPopulationGrowth[PersonRole::ROLE_WORKER] = mPopulationGrowth[PersonRole::ROLE_WORKER] % divisor;

		// account for universities
		if (universities > 0 && random_0_99() < studentToScientistRate)
		{
			mPopulation[PersonRole::ROLE_SCIENTIST] += newAdult;
		}
		else
		{
			mPopulation[PersonRole::ROLE_WORKER] += newAdult;
		}

		mPopulation[PersonRole::ROLE_STUDENT] -= newAdult;
	}
}


void Population::spawn_retiree()
{
	int total_adults = mPopulation[PersonRole::ROLE_WORKER] + mPopulation[PersonRole::ROLE_SCIENTIST];
	if (total_adults > 0)
	{
		mPopulationGrowth[PersonRole::ROLE_RETIRED] += total_adults / 10;

		int divisor = std::max(total_adults, adultToRetireeBase);

		divisor = ((divisor / 40) * 3 + 40) * 4;

		int retiree = mPopulationGrowth[PersonRole::ROLE_RETIRED] / divisor;
		mPopulationGrowth[PersonRole::ROLE_RETIRED] = mPopulationGrowth[PersonRole::ROLE_RETIRED] % divisor;

		mPopulation[PersonRole::ROLE_RETIRED] += retiree;

		/** Workers retire earlier than scientists. */
		if (random_0_99() < 45) { if (mPopulation[PersonRole::ROLE_SCIENTIST] > 0) { mPopulation[PersonRole::ROLE_SCIENTIST] -= retiree; } }
		else { if (mPopulation[PersonRole::ROLE_WORKER] > 0) { mPopulation[PersonRole::ROLE_WORKER] -= retiree; } }
	}
}


void Population::kill_children(int morale, int nurseries)
{
	if (mPopulation[PersonRole::ROLE_CHILD] > 0)
	{
		mPopulationDeath[PersonRole::ROLE_CHILD] += mPopulation[PersonRole::ROLE_CHILD];

		int divisor = moraleModifierTable[moraleIndex(morale)].mortalityRate + (nurseries * 10);

		int deaths = mPopulationDeath[PersonRole::ROLE_CHILD] / divisor;
		mPopulationDeath[PersonRole::ROLE_CHILD] = mPopulationDeath[PersonRole::ROLE_CHILD] % divisor;

		mPopulation[PersonRole::ROLE_CHILD] -= deaths;
		mDeathCount += deaths;

		if (mPopulation[PersonRole::ROLE_CHILD] <= 0)
		{
			mPopulationDeath[PersonRole::ROLE_CHILD] = 0;
			mPopulationGrowth[PersonRole::ROLE_STUDENT] = 0;
		}
	}
}


void Population::kill_students(int morale, int hospitals)
{
	if (mPopulation[PersonRole::ROLE_CHILD] > 0)
	{
		mPopulationDeath[PersonRole::ROLE_STUDENT] += mPopulation[PersonRole::ROLE_STUDENT];

		int divisor = moraleModifierTable[moraleIndex(morale)].mortalityRate + (hospitals * 65);

		int deaths = mPopulationDeath[PersonRole::ROLE_STUDENT] / divisor;
		mPopulationDeath[PersonRole::ROLE_STUDENT] = mPopulationDeath[PersonRole::ROLE_STUDENT] % divisor;

		mPopulation[PersonRole::ROLE_STUDENT] -= deaths;
		mDeathCount += deaths;

		if (mPopulation[PersonRole::ROLE_STUDENT] <= 0)
		{
			mPopulationDeath[PersonRole::ROLE_STUDENT] = 0;
			mPopulationGrowth[PersonRole::ROLE_WORKER] = 0;
		}
	}
}


void Population::kill_adults(Population::PersonRole role, int morale, int hospitals)
{
	// Worker Deaths
	if (mPopulation[role] > 0)
	{
		mPopulationDeath[role] += mPopulation[role];
		int divisor = moraleModifierTable[moraleIndex(morale)].mortalityRate + 250 + (hospitals * 60);

		int deaths = mPopulationDeath[role] / divisor;
		mPopulationDeath[role] = mPopulationDeath[role] % divisor;

		mPopulation[role] -= deaths;
		mDeathCount += deaths;

		if (mPopulation[role] == 0)
		{
			mPopulationDeath[role] = 0;
		}
	}
}


/**
 * Determine how much food should be consumed and kill off any population that
 * starves.
 *
 * \return	Actual amount of food consumed.
 */
int Population::consume_food(int food)
{
	// If there's no food kill everybody (humans can survive up to 21 days without food, one turn == minimum 28 days)
	if (food == 0)
	{
		mDeathCount = size();
		clear();
		return 0;
	}

	int population_fed = food * 10;
	if (population_fed >= size())
	{
		return size() / 10;
	}

	// Addition of U[0,100) ensures the correct amount of people die on average.
	int population_to_kill = ((size() - population_fed) * mStarveRate + random_0_99()) / 100;

	// Distribute deaths evenly among roles.
	int roles_with_nonzero_population = PersonRole::ROLE_COUNT;
	while (population_to_kill >= roles_with_nonzero_population)
	{
		int deaths_per_role = population_to_kill / roles_with_nonzero_population;
		roles_with_nonzero_population = PersonRole::ROLE_COUNT;
		for (int role = 0; role < PersonRole::ROLE_COUNT; ++role)
		{
			int deaths_in_role = std::min(deaths_per_role, mPopulation[role]);
			mPopulation[role] -= deaths_in_role;
			mDeathCount += deaths_in_role;
			population_to_kill -= deaths_in_role;
			if (mPopulation[role] == 0)
				--roles_with_nonzero_population;
		}
		// If any role had a smaller population than deaths_per_role we may
		// still have many deaths to distribute and will iterate once more.
	}

	// There may be a few remaining deaths which still need to be distributed across a random subset of roles.
	// Round-robin from a random starting role should be good enough.
	int role = random_0_99() % PersonRole::ROLE_COUNT;
	while (population_to_kill)
	{
		if (mPopulation[role] > 0)
		{
			--mPopulation[role];
			++mDeathCount;
			--population_to_kill;
		}
		role = (role + 1) % PersonRole::ROLE_COUNT;
	}

	// actual amount of food used for the fed part of the population.
	return population_fed / 10;
}


/**
 * \return	Actual amount of food consumed.
 */
int Population::update(int morale, int food, int residences, int universities, int nurseries, int hospitals)
{
	mBirthCount = 0;
	mDeathCount = 0;

	spawn_children(morale, residences, nurseries);
	spawn_students();
	spawn_adults(universities);
	spawn_retiree();

	kill_children(morale, nurseries);
	kill_students(morale, hospitals);

	// Workers will die more often than scientists.
	if (random_0_99() < 45) { kill_adults(PersonRole::ROLE_SCIENTIST, morale, hospitals); }
	else { kill_adults(PersonRole::ROLE_WORKER, morale, hospitals); }

	kill_adults(PersonRole::ROLE_RETIRED, morale, hospitals);

	return consume_food(food);
}
