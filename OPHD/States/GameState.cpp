#include "GameState.h"

#include "MainMenuState.h"
#include "MapViewState.h"
#include "MainReportsUiState.h"
#include "Wrapper.h"
#include "../StructureManager.h"

#include <NAS2D/Utility.h>
#include <NAS2D/EventHandler.h>
#include <NAS2D/Mixer/Mixer.h>
#include <NAS2D/Renderer/Renderer.h>


NAS2D::Point<int> MOUSE_COORDS; /**< Mouse Coordinates. Used by other states/wrapers. */


// Explicit constructor needed in implementation file
// If a default constructor was used instead, construction would happen in importing translation unit
// The header uses forward declares for some types, so only incomplete types are available to importing code
// Complete types are needed to construct the std::unique_ptr members
GameState::GameState()
{}


GameState::~GameState()
{
	NAS2D::Utility<StructureManager>::get().dropAllStructures();

	NAS2D::EventHandler& e = NAS2D::Utility<NAS2D::EventHandler>::get();
	e.mouseMotion().disconnect(this, &GameState::onMouseMove);

	NAS2D::Utility<NAS2D::Renderer>::get().fadeComplete().disconnect(this, &GameState::fadeComplete);

	mMainReportsState->hideReports().disconnect(this, &GameState::hideReportsUi);
	mMapView->quit().disconnect(this, &GameState::quitEvent);
	mMapView->showReporstUi().disconnect(this, &GameState::showReportsUi);
	mMapView->mapChanged().disconnect(this, &GameState::mapChanged);

	for (auto takeMeThere : mMainReportsState->takeMeThere())
	{
		takeMeThere->disconnect(this, &GameState::takeMeThere);
	}

	NAS2D::Utility<NAS2D::Mixer>::get().removeMusicCompleteHandler(MakeDelegate(this, &GameState::musicComplete));
	NAS2D::Utility<NAS2D::Mixer>::get().stopAllAudio();
}


/**
 * Internal initializer function.
 */
void GameState::initialize()
{
	NAS2D::EventHandler& e = NAS2D::Utility<NAS2D::EventHandler>::get();
	e.mouseMotion().connect(this, &GameState::onMouseMove);

	mMainReportsState = std::make_unique<MainReportsUiState>();
	mMainReportsState->_initialize();
	mMainReportsState->hideReports().connect(this, &GameState::hideReportsUi);

	for (auto takeMeThere : mMainReportsState->takeMeThere())
	{
		takeMeThere->connect(this, &GameState::takeMeThere);
	}

	NAS2D::Utility<NAS2D::Mixer>::get().addMusicCompleteHandler(MakeDelegate(this, &GameState::musicComplete));
	NAS2D::Utility<NAS2D::Renderer>::get().fadeComplete().connect(this, &GameState::fadeComplete);
	NAS2D::Utility<NAS2D::Renderer>::get().fadeIn(constants::FADE_SPEED);
}


/**
 * Sets a pointer for the MapViewState.
 * 
 * Since the MapViewState is created outside of the GameState, this function
 * takes a pointer to an already instatiated MapViewState object.
 * 
 * \param	state	Pointer to a MapViewState. Ownership is transfered to GameState.
 * 
 * \note	GameState will handle correct destruction of the MapViewState object.
 */
void GameState::mapviewstate(MapViewState* state)
{
	mMapView.reset(state);
	mActiveState = mMapView.get();

	mMapView->quit().connect(this, &GameState::quitEvent);
	mMapView->showReporstUi().connect(this, &GameState::showReportsUi);
	mMapView->mapChanged().connect(this, &GameState::mapChanged);
}


MainReportsUiState& GameState::getMainReportsState()
{
	return *mMainReportsState;
}


/**
 * Mouse motion event handler.
 */
void GameState::onMouseMove(int x, int y, int /*relX*/, int /*relY*/)
{
	MOUSE_COORDS = {x, y};
}


/**
 * Event hanler for a 'fade complete' event raised by the NAS2D::Renderer.
 */
void GameState::fadeComplete()
{
	auto& renderer = NAS2D::Utility<NAS2D::Renderer>::get();
	if (renderer.isFaded())
	{
		mReturnState = new MainMenuState();
	}
}


/**
 * Music Complete event handler.
 * 
 * Called by NAS2D::Mixer upon completion of a music track. This function
 * changes the background music track to a different track in the lineup.
 */
void GameState::musicComplete()
{
	/// \todo	Make me work... once there's some music to listen to. 0.0
}


/**
 * Event handler that responds to a quit event raised by the MapViewState.
 * 
 * This event is raised on game overs and when the user chooses the "Return
 * to Main Menu" from the system options window.
 */
void GameState::quitEvent()
{
	mMapView->deactivate();
	mMainReportsState->deactivate();
}


/**
 * Event handler that responds to a show reports event raised by the MapViewState.
 * 
 * This event is raised whenever a user double-clicks on a factory in the MapViewState.
 */
void GameState::showReportsUi()
{
	mActiveState->deactivate();
	mActiveState = mMainReportsState.get();
	mActiveState->activate();
}


/**
 * Event handler that responds to a hide report event raised by the MainReportsUiState.
 * 
 * This event is raised by the MainReportsUiState whenever the user clicks the Exit
 * UI panel or if the Escape key is pressed.
 */
void GameState::hideReportsUi()
{
	mActiveState->deactivate();
	mActiveState = mMapView.get();
	mActiveState->activate();
}


void GameState::mapChanged()
{
	mMainReportsState->clearLists();
}


/**
 * Event handler that responds to a 'take me there' event raised by the MainReportsUiState.
 * 
 * This event is raised by the MainReportsUiState whenever a "Take Me There" button in any
 * of the report UI panels is clicked.
 */
void GameState::takeMeThere(Structure* structure)
{
	hideReportsUi();
	mMapView->focusOnStructure(structure);
}


NAS2D::State* GameState::update()
{
	if (mActiveState)
	{
		mActiveState->_update();
	}

	return mReturnState;
}
