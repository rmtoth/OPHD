#include "WarehouseReport.h"

#include "../../Cache.h"
#include "../../Constants.h"
#include "../../StructureManager.h"
#include "../../Things/Structures/Structure.h"
#include "../../Things/Structures/Warehouse.h"

#include <NAS2D/Utility.h>
#include <NAS2D/EventHandler.h>
#include <NAS2D/Renderer/Renderer.h>


using namespace NAS2D;


WarehouseReport::WarehouseReport() :
	fontMedium{fontCache.load(constants::FONT_PRIMARY, constants::FONT_PRIMARY_MEDIUM)},
	fontMediumBold{fontCache.load(constants::FONT_PRIMARY_BOLD, constants::FONT_PRIMARY_MEDIUM)},
	fontBigBold{fontCache.load(constants::FONT_PRIMARY_BOLD, constants::FONT_PRIMARY_HUGE)},
	imageWarehouse{imageCache.load("ui/interface/warehouse.png")},
	btnShowAll{"All"},
	btnSpaceAvailable{"Space Available"},
	btnFull{"Full"},
	btnEmpty{"Empty"},
	btnDisabled{"Disabled"},
	btnTakeMeThere{constants::BUTTON_TAKE_ME_THERE}
{
	add(btnShowAll, {10, 10});
	btnShowAll.size({75, 20});
	btnShowAll.type(Button::Type::BUTTON_TOGGLE);
	btnShowAll.toggle(true);
	btnShowAll.click().connect(this, &WarehouseReport::btnShowAllClicked);

	add(btnSpaceAvailable, {90, 10});
	btnSpaceAvailable.size({100, 20});
	btnSpaceAvailable.type(Button::Type::BUTTON_TOGGLE);
	btnSpaceAvailable.toggle(false);
	btnSpaceAvailable.click().connect(this, &WarehouseReport::btnSpaceAvailableClicked);

	add(btnFull, {195, 10});
	btnFull.size({75, 20});
	btnFull.type(Button::Type::BUTTON_TOGGLE);
	btnFull.toggle(false);
	btnFull.click().connect(this, &WarehouseReport::btnFullClicked);

	add(btnEmpty, {275, 10});
	btnEmpty.size({75, 20});
	btnEmpty.type(Button::Type::BUTTON_TOGGLE);
	btnEmpty.toggle(false);
	btnEmpty.click().connect(this, &WarehouseReport::btnEmptyClicked);

	add(btnDisabled, {355, 10});
	btnDisabled.size({75, 20});
	btnDisabled.type(Button::Type::BUTTON_TOGGLE);
	btnDisabled.toggle(false);
	btnDisabled.click().connect(this, &WarehouseReport::btnDisabledClicked);

	add(btnTakeMeThere, {10, 10});
	btnTakeMeThere.size({140, 30});
	btnTakeMeThere.click().connect(this, &WarehouseReport::btnTakeMeThereClicked);

	add(lstStructures, {10, mRect.y + 115});
	lstStructures.selectionChanged().connect(this, &WarehouseReport::lstStructuresSelectionChanged);

	add(lstProducts, {Utility<Renderer>::get().center().x + 10, mRect.y + 173});

	Utility<EventHandler>::get().mouseDoubleClick().connect(this, &WarehouseReport::doubleClicked);

	Control::resized().connect(this, &WarehouseReport::_resized);
	fillLists();
}


WarehouseReport::~WarehouseReport()
{
	Control::resized().disconnect(this, &WarehouseReport::_resized);
	Utility<EventHandler>::get().mouseDoubleClick().disconnect(this, &WarehouseReport::doubleClicked);
}


void WarehouseReport::computeTotalWarehouseCapacity()
{
	int capacityTotal = 0;
	int capacityAvailable = 0;

	const auto& structures = StructureManager::GetComponents<Warehouse>();
	for (auto warehouseStructure : structures)
	{
		if (warehouseStructure->structure().operational())
		{
			const auto& warehouseProducts = warehouseStructure->products();
			capacityAvailable += warehouseProducts.availableStorage();
			capacityTotal += warehouseProducts.capacity();
		}
	}

	int capacityUsed = capacityTotal - capacityAvailable;

	warehouseCount = structures.size();
	warehouseCapacityTotal = capacityTotal;
	warehouseCapacityPercent = static_cast<float>(capacityUsed) / static_cast<float>(capacityTotal);
}


void WarehouseReport::_fillListFromWarehouseList(const WarehouseList& list)
{
	for (auto warehouse : list)
	{
		Structure* structure = &warehouse->structure();
		lstStructures.addItem(structure);
		StructureListBox::StructureListBoxItem* item = lstStructures.last();

		// \fixme	Abuse of interface to achieve custom results.
		ProductPool& products = warehouse->products();

		if (structure->state() != StructureState::Operational) { item->structureState = structure->stateDescription(); }
		else if (products.empty()) { item->structureState = constants::WAREHOUSE_EMPTY; }
		else if (products.atCapacity()) { item->structureState = constants::WAREHOUSE_FULL; }
		else if (!products.empty() && !products.atCapacity()) { item->structureState = constants::WAREHOUSE_SPACE_AVAILABLE; }
	}
}


/**
 * Inherited interface. A better name for this function would be
 * fillListWithAll() or something to that effect.
 */
void WarehouseReport::fillLists()
{
	lstStructures.clear();

	_fillListFromWarehouseList(StructureManager::GetComponents<Warehouse>());

	lstStructures.setSelection(0);
	computeTotalWarehouseCapacity();
}


void WarehouseReport::fillListSpaceAvailable()
{
	lstStructures.clear();

	WarehouseList list;
	for (auto wh : StructureManager::GetComponents<Warehouse>())
	{
		if (!wh->products().atCapacity() && !wh->products().empty() && (wh->structure().operational() || wh->structure().isIdle()))
		{
			list.push_back(wh);
		}
	}

	_fillListFromWarehouseList(list);

	lstStructures.setSelection(0);
	computeTotalWarehouseCapacity();
}



void WarehouseReport::fillListFull()
{
	lstStructures.clear();

	WarehouseList list;
	for (auto wh : StructureManager::GetComponents<Warehouse>())
	{
		if (wh->products().atCapacity() && (wh->structure().operational() || wh->structure().isIdle()))
		{
			list.push_back(wh);
		}
	}

	_fillListFromWarehouseList(list);

	lstStructures.setSelection(0);
	computeTotalWarehouseCapacity();
}


void WarehouseReport::fillListEmpty()
{
	lstStructures.clear();

	WarehouseList list;
	for (auto wh : StructureManager::GetComponents<Warehouse>())
	{
		if (wh->products().empty() && (wh->structure().operational() || wh->structure().isIdle()))
		{
			list.push_back(wh);
		}
	}

	_fillListFromWarehouseList(list);

	lstStructures.setSelection(0);
	computeTotalWarehouseCapacity();
}


void WarehouseReport::fillListDisabled()
{
	lstStructures.clear();

	WarehouseList list;
	for (auto wh : StructureManager::GetComponents<Warehouse>())
	{
		if (wh->structure().disabled() || wh->structure().destroyed())
		{
			list.push_back(wh);
		}
	}

	_fillListFromWarehouseList(list);

	lstStructures.setSelection(0);
	computeTotalWarehouseCapacity();
}


void WarehouseReport::doubleClicked(EventHandler::MouseButton button, int x, int y)
{
	if (!visible()) { return; }
	if (button != EventHandler::MouseButton::BUTTON_LEFT) { return; }

	if (selectedWarehouse && lstStructures.rect().contains(NAS2D::Point{x, y}))
	{
		takeMeThereCallback()(selectedWarehouse);
	}
}


void WarehouseReport::clearSelected()
{
	lstStructures.clearSelected();
	selectedWarehouse = nullptr;
}


void WarehouseReport::refresh()
{
	btnShowAllClicked();
}


void WarehouseReport::selectStructure(Structure* structure)
{
	lstStructures.setSelected(structure);
	selectedWarehouse = structure;
}


void WarehouseReport::_resized(Control*)
{
	lstStructures.size({(mRect.width / 2) - 20, mRect.height - 126});
	lstProducts.size({(mRect.width / 2) - 20, mRect.height - 184});
	lstProducts.position({Utility<Renderer>::get().center().x + 10, lstProducts.positionY()});

	btnTakeMeThere.position({Utility<Renderer>::get().size().x - 150, positionY() + 35});
}


void WarehouseReport::filterButtonClicked()
{
	btnShowAll.toggle(false);
	btnSpaceAvailable.toggle(false);
	btnFull.toggle(false);
	btnEmpty.toggle(false);
	btnDisabled.toggle(false);
}


void WarehouseReport::btnShowAllClicked()
{
	filterButtonClicked();
	btnShowAll.toggle(true);

	fillLists();
}


void WarehouseReport::btnSpaceAvailableClicked()
{
	filterButtonClicked();
	btnSpaceAvailable.toggle(true);

	fillListSpaceAvailable();
}


void WarehouseReport::btnFullClicked()
{
	filterButtonClicked();
	btnFull.toggle(true);

	fillListFull();
}


void WarehouseReport::btnEmptyClicked()
{
	filterButtonClicked();
	btnEmpty.toggle(true);

	fillListEmpty();
}


void WarehouseReport::btnDisabledClicked()
{
	filterButtonClicked();
	btnDisabled.toggle(true);

	fillListDisabled();
}


void WarehouseReport::btnTakeMeThereClicked()
{
	takeMeThereCallback()(selectedWarehouse);
}


void WarehouseReport::lstStructuresSelectionChanged()
{
	selectedWarehouse = lstStructures.selectedStructure();

	if (selectedWarehouse != nullptr)
	{
		lstProducts.productPool(selectedWarehouse->Get<Warehouse>()->products());
	}

	btnTakeMeThere.visible(selectedWarehouse != nullptr);
}


void WarehouseReport::drawLeftPanel(Renderer& renderer)
{
	const auto textColor = NAS2D::Color{0, 185, 0};
	renderer.drawText(fontMediumBold, "Warehouse Count", NAS2D::Point{10, positionY() + 40}, textColor);
	renderer.drawText(fontMediumBold, "Total Storage", NAS2D::Point{10, positionY() + 62}, textColor);
	renderer.drawText(fontMediumBold, "Capacity Used", NAS2D::Point{10, positionY() + 84}, textColor);

	const auto warehouseCountText = std::to_string(warehouseCount);
	const auto warehouseCapacityText = std::to_string(warehouseCapacityTotal);
	const auto countTextWidth = fontMedium.width(warehouseCountText);
	const auto capacityTextWidth = fontMedium.width(warehouseCapacityText);
	renderer.drawText(fontMedium, warehouseCountText, NAS2D::Point{mRect.width / 2 - 10 - countTextWidth, positionY() + 35}, textColor);
	renderer.drawText(fontMedium, warehouseCapacityText, NAS2D::Point{mRect.width / 2 - 10 - capacityTextWidth, positionY() + 57}, textColor);

	const auto capacityUsedTextWidth = fontMediumBold.width("Capacity Used");
	const auto capacityBarWidth = mRect.width / 2 - 30 - capacityUsedTextWidth;
	const auto capacityBarPositionX = 20 + capacityUsedTextWidth;
	drawBasicProgressBar(capacityBarPositionX, positionY() + 84, capacityBarWidth, 20, warehouseCapacityPercent);
}


void WarehouseReport::drawRightPanel(Renderer& renderer)
{
	if (!selectedWarehouse) { return; }

	const auto positionX = renderer.center().x + 10;
	renderer.drawText(fontBigBold, selectedWarehouse->name(), NAS2D::Point{positionX, positionY() + 2}, NAS2D::Color{0, 185, 0});
	renderer.drawImage(imageWarehouse, NAS2D::Point{positionX, positionY() + 35});
}


void WarehouseReport::update()
{
	if (!visible()) { return; }
	auto& renderer = Utility<Renderer>::get();

	// Left Panel
	drawLeftPanel(renderer);
	const auto positionX = renderer.center().x;
	renderer.drawLine(NAS2D::Point{positionX, positionY() + 10}, NAS2D::Point{positionX, positionY() + mRect.height - 10}, NAS2D::Color{0, 185, 0});
	drawRightPanel(renderer);

	UIContainer::update();
}
