#pragma once

#include "Structure.h"

#include "../../Mine.h"

/**
 * Implements the Mine Facility.
 */
class MineFacility: public Structure
{
public:
	using ExtensionCompleteCallback = NAS2D::Signals::Signal<MineFacility*>;
public:
	MineFacility(Mine* mine);

	void mine(Mine* mine) { mMine = mine; }
	void maxDepth(int depth) { mMaxDepth = depth; }

	bool extending() const;
	bool canExtend() const;
	void extend();

	int digTimeRemaining() const;

	int assignedTrucks()  const { return mAssignedTrucks; }
	int maxTruckCount() const { return mMaxTruckCount; }

	void addTruck() { mAssignedTrucks = std::clamp(mAssignedTrucks + 1, 1, mMaxTruckCount); }
	void removeTruck() { mAssignedTrucks = std::clamp(mAssignedTrucks - 1, 1, mMaxTruckCount); }

	/**
	 * Gets a pointer to the mine the MineFacility manages.
	 */
	Mine* mine() { return mMine; }

	ExtensionCompleteCallback& extensionComplete() { return mExtensionComplete; }

protected:
	void think() override;

private:
	MineFacility() = delete;
	MineFacility(const MineFacility&) = delete;
	MineFacility& operator=(const MineFacility&) = delete;

private:
	void activated() override;

private:
	int mMaxDepth = 0; /**< Maximum digging depth. */
	int mDigTurnsRemaining = 0; /**< Turns remaining before extension is complete. */
	int mAssignedTrucks = 1; /**< All mine facilities are built with at least one truck. */
	int mMaxTruckCount = 10;

	Mine* mMine = nullptr; /**< Mine that this facility manages. */

	ExtensionCompleteCallback mExtensionComplete; /**< Called whenever an extension is completed. */
};
