#include "MajorEventAnnouncement.h"

#include "../Cache.h"
#include "../Constants.h"

#include <NAS2D/Utility.h>
#include <NAS2D/Renderer/Renderer.h>


using namespace NAS2D;


MajorEventAnnouncement::MajorEventAnnouncement() :
	mHeader{imageCache.load("ui/interface/colony_ship_crash.png")}
{
	position({0, 0});
	size({522, 340});

	add(btnClose, {5, 310});
	btnClose.size({512, 25});
	btnClose.click().connect(this, &MajorEventAnnouncement::btnCloseClicked);

	anchored(true);
}


void MajorEventAnnouncement::btnCloseClicked()
{
	hide();
}


void MajorEventAnnouncement::announcement(AnnouncementType a)
{
	switch (a)
	{
	case AnnouncementType::ANNOUNCEMENT_COLONY_SHIP_CRASH:
		mMessage = "Colony ship deorbited and crashed on the surface.";
		break;
	case AnnouncementType::ANNOUNCEMENT_COLONY_SHIP_CRASH_WITH_COLONISTS:
		mMessage = "Colony ship deorbited and crashed on the surface but you left colonists on board!";
		break;
	default:
		throw std::runtime_error("MajorEventAnnouncement::announcement(): Called with invalid announcement type.");
	}
}


void MajorEventAnnouncement::update()
{
	if (!visible()) { return; }

	Window::update();

	auto& renderer = Utility<Renderer>::get();

	renderer.drawImage(mHeader, position() + NAS2D::Vector{5, 25});

	const auto& font = fontCache.load(constants::FONT_PRIMARY, constants::FONT_PRIMARY_NORMAL);
	renderer.drawText(font, mMessage, position() + NAS2D::Vector{5, 290}, NAS2D::Color::White);
}
