#pragma once

#include "../Common.h"

#include <NAS2D/Renderer/Point.h>
#include <NAS2D/Renderer/Vector.h>


class Mine;
class Thing;
class Robot;
class Structure;


class Tile
{
public:
	enum class Overlay
	{
		Communications,
		Connectedness,
		TruckingRoutes,

		None
	};

public:
	Tile() = default;
	Tile(NAS2D::Point<int>, int, TerrainType);
	Tile(const Tile&) = delete;
	Tile& operator=(const Tile&) = delete;
	Tile(Tile&&) noexcept;
	Tile& operator=(Tile&&) noexcept;
	~Tile();

	TerrainType index() const { return mIndex; }
	void index(TerrainType index) { mIndex = index; }

	NAS2D::Point<int> position() const { return mPosition; }

	int depth() const { return mDepth; }
	void depth(int i) { mDepth = i; }

	bool bulldozed() const { return index() == TerrainType::Dozed; }

	bool excavated() const { return mExcavated; }
	void excavated(bool value) { mExcavated = value; }

	bool connected() const { return mConnected; }
	void connected(bool value) { mConnected = value; }

	Thing* thing() const { return mThing; }

	bool empty() const { return mThing == nullptr; }

	bool hasMine() const { return mMine != nullptr; }

	Structure* structure() const;
	Robot* robot() const;

	bool thingIsStructure() const { return structure() != nullptr; }
	bool thingIsRobot() const { return robot() != nullptr; }

	void pushThing(Thing*);
	void deleteThing();

	void removeThing();

	Mine* mine() { return mMine; }
	void pushMine(Mine*);

	void overlay(Overlay overlay) { mOverlay = overlay; }
	Overlay overlay() const { return mOverlay; }

private:
	TerrainType mIndex = TerrainType::Dozed;

	NAS2D::Point<int> mPosition; /**< Tile Position Information */
	int mDepth = 0; /**< Tile Position Information */

	Thing* mThing = nullptr;
	Mine* mMine = nullptr;

	Overlay mOverlay{ Overlay::None };

	bool mExcavated = true; /**< Used when a Digger uncovers underground tiles. */
	bool mConnected = false; /**< Flag indicating that this tile is connected to the Command Center. */
};

const NAS2D::Color& overlayColor(Tile::Overlay, bool);
const NAS2D::Color& overlayColor(Tile::Overlay);
const NAS2D::Color& overlayHighlightColor(Tile::Overlay);

using TileList = std::vector<Tile*>;
