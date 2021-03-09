#pragma once

#include "ReportInterface.h"

#include "../ProductListBox.h"
#include "../StructureListBox.h"

#include "../Core/Button.h"

#include <vector>


namespace NAS2D {
	class Font;
	class Image;
}

class Warehouse;
class Structure;


class WarehouseReport : public ReportInterface
{
public:
	WarehouseReport();
	~WarehouseReport() override;

	void fillLists() override;
	void clearSelected() override;

	void refresh() override;
	void selectStructure(Structure*) override;

	void update() override;

private:
	using WarehouseList = std::vector<Warehouse*>;
	void computeTotalWarehouseCapacity();

	void _fillListFromWarehouseList(const WarehouseList&);

	void _resized(Control*);

	void btnShowAllClicked();
	void btnSpaceAvailableClicked();
	void btnFullClicked();
	void btnEmptyClicked();
	void btnDisabledClicked();

	void btnTakeMeThereClicked();

	void fillListSpaceAvailable();
	void fillListFull();
	void fillListEmpty();
	void fillListDisabled();

	void doubleClicked(NAS2D::EventHandler::MouseButton, int, int);

	void lstStructuresSelectionChanged();

	void filterButtonClicked();

	void drawLeftPanel(NAS2D::Renderer&);
	void drawRightPanel(NAS2D::Renderer&);

	const NAS2D::Font& fontMedium;
	const NAS2D::Font& fontMediumBold;
	const NAS2D::Font& fontBigBold;
	const NAS2D::Image& imageWarehouse;

	Structure* selectedWarehouse = nullptr;

	Button btnShowAll;
	Button btnSpaceAvailable;
	Button btnFull;
	Button btnEmpty;
	Button btnDisabled;

	Button btnTakeMeThere;

	StructureListBox lstStructures;
	ProductListBox lstProducts;

	std::size_t warehouseCount;
	int warehouseCapacityTotal;
	float warehouseCapacityPercent = 0.0f;
};
