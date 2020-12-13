#pragma once

#include "Core/ListBoxBase.h"

#include <NAS2D/Signal.h>
#include <NAS2D/Renderer/Point.h>

#include <string>
#include <vector>


class Factory;


/**
 * Implements a ListBox control.
 */
class FactoryListBox : public ListBoxBase
{
public:
	using SelectionChangedCallback = NAS2D::Signals::Signal<Factory*>;

	struct FactoryListBoxItem : public ListBoxItem
	{
		FactoryListBoxItem(std::string textDescription, Factory* newFactory, NAS2D::Point<int> iconPosition) :
			ListBoxItem{textDescription},
			factory{newFactory},
			icon_slice{iconPosition}
		{}

		Factory* factory = nullptr;
		NAS2D::Point<int> icon_slice;
	};


	FactoryListBox();

	void addItem(Factory* factory);
	void setSelected(Factory*);

	Factory* selectedFactory();

	void update() override;
};
