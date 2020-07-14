#pragma once

#include "UIContainer.h"
#include "Slider.h"
#include "../../Constants/UiConstants.h"

#include <NAS2D/Signal.h>
#include <NAS2D/EventHandler.h>
#include <NAS2D/Renderer/Color.h>

#include <string>
#include <vector>
#include <cstddef>


/**
 * Implements a base ListBox control that handles the low-level drawing,
 * input handling and management code while leaving the specific implementation
 * to more derived types.
 * 
 * \note	This is an abstract class -- it will need to be inherited from
 *			in order to be used.
 */
class ListBoxBase : public Control
{
public:
	/**
	 * Callback signal fired whenever the list selection changes.
	 */
	using SelectionChangedCallback = NAS2D::Signals::Signal<>;

	/**
	 * Derived SpecialListBox types can inherit from this struct
	 * for specialized information needed for derived types.
	 */
	struct ListBoxItem
	{
		ListBoxItem() = default;
		ListBoxItem(std::string text) : Text(text) {}
		virtual ~ListBoxItem() = default;

		std::string Text;
	};


	ListBoxBase();
	~ListBoxBase() override;

	void addItem(ListBoxItem*);
	void removeItem(ListBoxItem*);
	void clearItems();

	std::size_t count() const;
	bool empty() const;

	unsigned int currentHighlight() const;
	unsigned int currentSelection() const;
	void setSelection(int selection);

	const std::string& selectionText() const;

	void clearSelection();

	SelectionChangedCallback& selectionChanged() { return mSelectionChanged; }

	void update() override = 0;


protected:
	/**
	 * List of ListBoxItem's.
	 * 
	 * \note	Pointers used for polymorphism.
	 * 
	 * \note	SpecialListBox owns pointers in this list and will
	 *			handle memory internally.
	 */
	using ItemList = std::vector<ListBoxItem*>;


	void _update_item_display();

	unsigned int item_width() const { return mItemWidth; }
	unsigned int item_height() const { return mItemHeight; }
	void item_height(int);

	unsigned int draw_offset() const { return mCurrentOffset; }

	void visibilityChanged(bool) override;


	ItemList mItems; /**< List of Items. */

private:
	void _init();
	void slideChanged(float newPosition);

	virtual void onMouseDown(NAS2D::EventHandler::MouseButton button, int x, int y);
	void onMouseMove(int x, int y, int relX, int relY);
	void onMouseWheel(int x, int y);

	void onSizeChanged() override;


	unsigned int mCurrentHighlight = constants::NO_SELECTION; /**< Currently highlighted selection index. */
	unsigned int mCurrentSelection = constants::NO_SELECTION; /**< Current selection index. */
	unsigned int mCurrentOffset = 0; /**< Draw Offset. */

	int mItemHeight = 1; /**< Height of a ListBoxItem. */
	int mItemWidth = 0; /**< Width of a ListBoxItem. */
	int mLineCount = 0; /**< Number of lines that can be displayed. */

	bool mHasFocus = false;

	NAS2D::Color mText = NAS2D::Color::White; /**< Text Color */
	NAS2D::Color mHighlightBg = NAS2D::Color::DarkGreen; /**< Highlight Background color. */
	NAS2D::Color mHighlightText = NAS2D::Color::White; /**< Text Color for an item that is currently highlighted. */

	SelectionChangedCallback mSelectionChanged; /**< Callback for selection changed callback. */
	Slider mSlider; /**< Slider control. */
};
