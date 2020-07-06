#pragma once

#include <NAS2D/Renderer/Renderer.h>
#include <NAS2D/Renderer/Color.h>
#include <NAS2D/Renderer/Point.h>
#include <NAS2D/Renderer/Vector.h>
#include <string>
#include <vector>
#include <cstddef>

// Draw a 2 dimensional table of text. Determine cell size based on inserted text, font, and padding. Only allows one line of text per cell.
class StringTable
{
public:
	using CellCoordinate = NAS2D::Point<std::size_t>;

	enum class Justification
	{
		Left,
		Right
	};

	struct Cell
	{
		// Set textColor to ColorEmpty to indicate cell should use default StringTable color
		static const NAS2D::Color ColorEmpty;

		// Use StringTable::mDefaultFont if not set
		NAS2D::Font* font = nullptr;
		std::string text;
		Justification justification = Justification::Left;
		NAS2D::Color textColor = ColorEmpty;
	};

	Cell& operator[](const CellCoordinate& coordinate);
	Cell& at(std::size_t column, std::size_t row);

	// Set default fonts in constructor
	StringTable(std::size_t columns, std::size_t rows);

	void draw(NAS2D::Renderer& renderer) const;

	void position(NAS2D::Point<float> position);
	NAS2D::Point<float> position() const;

	void setDefaultFont(NAS2D::Font& font);
	void setDefaultTitleFont(NAS2D::Font* font);
	void setDefaultTextColor(NAS2D::Color textColor);

	void setHorizontalPadding(float horizontalPadding);
	void setVerticalPadding(float verticalPadding);

	void setColumnJustification(std::size_t column, Justification justification);

	// Call after updating table properties to recompute cell positions
	void computeRelativeCellPositions();

private:
	// Purposely hide textRelativePosition from public access
	struct CellWithPosition : Cell
	{
		// Position relative to the StringTable's position
		NAS2D::Vector<float> textRelativePosition;
	};

	std::vector<CellWithPosition> mCells;
	const std::size_t mColumnCount;
	const std::size_t mRowCount;
	NAS2D::Point<float> mPosition;
	NAS2D::Font* mDefaultFont;
	NAS2D::Font* mDefaultTitleFont;
	NAS2D::Color mDefaultTextColor = NAS2D::Color::White;
	float mHorizontalPadding = 5;
	float mVerticalPadding = 0;

	void accountForCellJustification(std::size_t index, float columnWidth);
	std::vector<float> computeColumnWidths() const;
	std::vector<float> computeRowHeights() const;

	std::size_t getCellIndex(const CellCoordinate& cellCoordinate) const;
	CellCoordinate getCellCoordinate(std::size_t index) const;
	void checkCellIndex(const CellCoordinate& cellCoordinate) const;

	NAS2D::Font* getCellFont(std::size_t index) const;
	bool isFirstColumn(std::size_t index) const;
};
