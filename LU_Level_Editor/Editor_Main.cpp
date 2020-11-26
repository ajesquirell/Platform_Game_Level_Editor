#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_GRAPHICS2D
#include "olcPGEX_Graphics2D.h"

#include "Tiles.h"
//#include "Animator.h"
#include "Assets.h"
#include "Button.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <future>

constexpr char LEVEL_PATH[] = "../Levels/";;

using namespace std::chrono_literals;


class Editor : public olc::PixelGameEngine
{
public:
	Editor()
	{
		sAppName = "Editor";
	}

	enum
	{
		MODE_START,
		MODE_NEW,
		MODE_EDIT,
		MODE_INFO,
		MODE_ERROR,
		MODE_TEST
	};

	int nMode = MODE_START;
	std::fstream myfile;

	int nLevelLoadingCnt = 0;
	int nLevelLoadingPercent = 0;
	/*===========Threads==============*/
	std::atomic<bool> atomicWorkerThreadActive = false; // Use an atomic flag
	std::atomic<bool> done = false; // Use an atomic flag

	std::promise<bool> promise;
	std::thread workerThread;

	/*===========New Screen===========*/
	std::vector<Button*> vecNewScreenButtons;
	std::string s; // New Level Name
	int newX = 128, newY = 64; // New level size
	float fNewScreenTimer = 0.0f;

	/*===========Start Screen===========*/
	std::vector<std::string> vLevelNames;

	int nLevelSelectBoxPosX = 5;
	int nLevelSelectBoxPosY = 25;
	int nLevelSelectBoxWidth = 200;
	int nLevelSelectBoxHeight = 400;

	// Number of level names that can fit onscreen
	int nVisibleLevelNames = (nLevelSelectBoxHeight) / 25;

	int nSelection = 0;
	int nSelectionOffset = 0;

	olc::Pixel p1, p2; // For changing mouse over colors
	

	/*=====================================*/

	/*===========Edit Screen===========*/
	std::vector<std::string> vecTileNames; // Acts as a map from ints to strings

	float fScalingFactor = 1.7f; // Adjust this to zoom

	olc::vi2d vGameScreenPos = { 25, 30 };

	int nGameScreenWidth = (int)(22 * fScalingFactor) * 12; // 444 - Hard code the level edit window to be the same aspect ratio as the game even when scaled (and in the editor we will be able to zoom in and out, but default zoom is the game's view)
	int nGameScreenHeight = (int)(22 * fScalingFactor) * 11; // 407

	cTile** vecTiles = nullptr; //Array of cTile pointers
	int nWidth, nHeight;

	float fCameraPosX = 5.0f; // Camera determines which tiles will be drawn
	float fCameraPosY = 5.0f;
	float fViewOffsetX = 0.0f; // ViewOffset determines where on the screen they will start to be drawn (essentially acts like the camera when zoomed out far enough)
	float fViewOffsetY = 0.0f;

	olc::vf2d fOldMousePos = { 0, 0 }; // For mouse camera movement

	// Tile selection in edit mode
	int nTileBoxPosX;
	int nTileBoxPosY;
	int nTileBoxWidth;
	int nTileBoxHeight;
	int nVisibleTileNames;

	int nTilePotentialSelect = 0; // For mouse over tile selection
	int nTileSelection = 0;
	int nTileSelectionOffset = 0;
	
	int nSelectedTileX;
	int nSelectedTileY;
	// ============================

	int nBrushSize = 1;

	olc::Pixel editScreenColor = olc::CYAN;

	std::string sSaveErrorMsg;
	bool bShowSaveText = false;
	bool bSaveOkay = false;
	float fSaveTextTimer = 0.0f;

	std::vector<Button*> vecButtons;
	/*=================================*/

	/*===========Error Screen===========*/
	int nErrorCode = 0; // Default
	/*=================================*/

public:
	bool OnUserCreate() override
	{
		// Some Layer managing
		CreateLayer();
		CreateLayer();
		EnableLayer(1, true);
		EnableLayer(2, true);

		ClearAllLayers(olc::BLANK);

		SetDrawTarget(nullptr);

		//Already checked before this class is created that level exits.
		//Obviously you could mess this up, but this level of error 
		//checking is good enough for the purposes of this application
		myfile.open("level.lvl");

		LoadLevelNames();


		// Assets
		CreateKeyMap(); // Create a list of sprites we're using
		Assets::get().LoadTileSprites(this, vecTileNames); // Load respective sprites from that list

		/*vLevelNames.push_back("Wow ville");
		vLevelNames.push_back("Wow ville1");
		vLevelNames.push_back("Wow ville2");
		vLevelNames.push_back("Wow ville3");
		vLevelNames.push_back("Wow ville4");
		vLevelNames.push_back("Wow ville5");
		vLevelNames.push_back("Wow ville6");
		vLevelNames.push_back("Wow ville7");
		vLevelNames.push_back("Wow ville8");
		vLevelNames.push_back("Wow ville9");
		vLevelNames.push_back("Wow ville10");
		vLevelNames.push_back("CrapTown");
		vLevelNames.push_back("CrapTown1");
		vLevelNames.push_back("CrapTown2");
		vLevelNames.push_back("CrapTown3");
		vLevelNames.push_back("CrapTown4");
		vLevelNames.push_back("CrapTown5");
		vLevelNames.push_back("CrapTown6");
		vLevelNames.push_back("CrapTown7");
		vLevelNames.push_back("CrapTown8");
		vLevelNames.push_back("CrapTown9");
		vLevelNames.push_back("CrapTown10");
		vLevelNames.push_back("CrapTown11");*/

		nTileBoxPosX = ScreenWidth() - 140;
		nTileBoxPosY = 30;
		nTileBoxWidth = 130;
		nTileBoxHeight = 250;
		nVisibleTileNames = (nTileBoxHeight) / 25;

		//===========================Add NEW Screen Buttons=========================================
		Button* n = new RectButton({ float(ScreenWidth() / 2 - 130), float(210) }, { 55, 25 }, "+", 2.0f, [](void* e) -> void {static_cast<Editor*>(e)->newX++; });
		n->buttonColor1 = olc::BLACK;
		n->buttonColor2 = olc::RED;
		n->textColor1 = olc::WHITE;
		n->textColor2 = olc::YELLOW;
		vecNewScreenButtons.push_back(n);

		n = new RectButton({ float(ScreenWidth() / 2 - 130), float(280) }, { 55, 25 }, "-", 2.0f, [](void* e) -> void {static_cast<Editor*>(e)->newX--; });
		n->buttonColor1 = olc::BLACK;
		n->buttonColor2 = olc::RED;
		n->textColor1 = olc::WHITE;
		n->textColor2 = olc::YELLOW;
		vecNewScreenButtons.push_back(n);

		n = new RectButton({ float(ScreenWidth() / 2 + 65), float(210) }, { 55, 25 }, "+", 2.0f, [](void* e) -> void {static_cast<Editor*>(e)->newY++; });
		n->buttonColor1 = olc::BLACK;
		n->buttonColor2 = olc::RED;
		n->textColor1 = olc::WHITE;
		n->textColor2 = olc::YELLOW;
		vecNewScreenButtons.push_back(n);

		n = new RectButton({ float(ScreenWidth() / 2 + 65), float(280) }, { 55, 25 }, "-", 2.0f, [](void* e) -> void {static_cast<Editor*>(e)->newY--; });
		n->buttonColor1 = olc::BLACK;
		n->buttonColor2 = olc::RED;
		n->textColor1 = olc::WHITE;
		n->textColor2 = olc::YELLOW;
		vecNewScreenButtons.push_back(n);
		//==========================================================================================

		//===========================Add EDIT Screen Buttons=========================================
		// Plus brush size
		Button* b = new CircleButton({ float((nTileBoxPosX + nTileBoxWidth / 2) - 20), float(nTileBoxPosY + nTileBoxHeight + 23) }, 10, "+", 1.5f);
		b->buttonColor1 = olc::DARK_GREY; 
		b->buttonColor2 = olc::BLUE; 
		b->textColor1 = olc::WHITE; 
		b->textColor2 = olc::GREEN;
		b->func = [](void* e) -> void {Editor* d = static_cast<Editor*>(e); d->nBrushSize++; };
		vecButtons.push_back(b);

		// Minus brush size
		b = new CircleButton({ float(nTileBoxPosX + nTileBoxWidth / 2) + 20, float(nTileBoxPosY + nTileBoxHeight + 23) }, 10, "-", 1.5f );
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::BLUE;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::GREEN;
		b->func = [](void* e) -> void {Editor* d = static_cast<Editor*>(e); d->nBrushSize--; };
		vecButtons.push_back(b);

		// Change level size buttons

		// Right
		b = new RectButton({ float(vGameScreenPos.x + nGameScreenWidth + 3), float(vGameScreenPos.y + nGameScreenHeight / 2 - 30) }, { 10, 20 }, "+", 1.0f, [](void* e) -> void {static_cast<Editor*>(e)->ChangeLevelSize(0, true); });
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::RED;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::YELLOW;
		vecButtons.push_back(b);

		b = new RectButton({ float(vGameScreenPos.x + nGameScreenWidth + 3), float(vGameScreenPos.y + nGameScreenHeight / 2) }, { 10, 20 }, "-", 1.0f, [](void* e) -> void {static_cast<Editor*>(e)->ChangeLevelSize(0, false); });
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::RED;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::YELLOW;
		vecButtons.push_back(b);
		
		// Bottom
		b = new RectButton({ float(vGameScreenPos.x + nGameScreenWidth / 2 - 25), float(vGameScreenPos.y + nGameScreenHeight + 3) }, { 20, 10 }, "+", 1.0f, [](void* e) -> void {static_cast<Editor*>(e)->ChangeLevelSize(1, true); });
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::RED;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::YELLOW;
		vecButtons.push_back(b);

		b = new RectButton({ float(vGameScreenPos.x + nGameScreenWidth / 2 + 5), float(vGameScreenPos.y + nGameScreenHeight + 3) }, { 20, 10 }, "-", 0.95f, [](void* e) -> void {static_cast<Editor*>(e)->ChangeLevelSize(1, false); });
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::RED;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::YELLOW;
		vecButtons.push_back(b);

		// Left
		b = new RectButton({ float(vGameScreenPos.x - 13), float(vGameScreenPos.y + nGameScreenHeight / 2 - 30) }, { 10, 20 }, "+", 1.0f, [](void* e) -> void {static_cast<Editor*>(e)->ChangeLevelSize(2, true); });
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::RED;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::YELLOW;
		vecButtons.push_back(b);

		b = new RectButton({ float(vGameScreenPos.x - 13), float(vGameScreenPos.y + nGameScreenHeight / 2) }, { 10, 20 }, "-", 1.0f, [](void* e) -> void {static_cast<Editor*>(e)->ChangeLevelSize(2, false); });
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::RED;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::YELLOW;
		vecButtons.push_back(b);

		// Top
		b = new RectButton({ float(vGameScreenPos.x + nGameScreenWidth / 2 - 25), float(vGameScreenPos.y - 13) }, { 20, 10 }, "+", 1.0f, [](void* e) -> void {static_cast<Editor*>(e)->ChangeLevelSize(3, true); });
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::RED;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::YELLOW;
		vecButtons.push_back(b);

		b = new RectButton({ float(vGameScreenPos.x + nGameScreenWidth / 2 + 5), float(vGameScreenPos.y - 13) }, { 20, 10 }, "-", 0.95f, [](void* e) -> void {static_cast<Editor*>(e)->ChangeLevelSize(3, false); });
		b->buttonColor1 = olc::DARK_GREY;
		b->buttonColor2 = olc::RED;
		b->textColor1 = olc::WHITE;
		b->textColor2 = olc::YELLOW;
		vecButtons.push_back(b);

		//============================================================================
		//TEST

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		switch (nMode)
		{
		case MODE_START:
			return UpdateStartScreen(fElapsedTime);
		case MODE_NEW:
			return UpdateNewScreen(fElapsedTime);
		case MODE_EDIT:
			return UpdateEditScreen(fElapsedTime);
		case MODE_INFO:
			return UpdateInfoScreen(fElapsedTime);
		case MODE_ERROR:
			return UpdateErrorScreen(fElapsedTime);
		case MODE_TEST:
			return UpdateTestScreen(fElapsedTime);
		}

		return true;
	}

	bool OnUserDestroy() override
	{
		for (int i = 0; i < nWidth * nHeight; i++)
			delete vecTiles[i];

		delete[] vecTiles;

		return true;
	}

	bool UpdateStartScreen(float fElapsedTime)
	{
		if (!atomicWorkerThreadActive)
		{
			if (IsFocused())
			{
				if (GetKey(olc::DOWN).bPressed)
				{
					nSelection++;
					if (nSelection > vLevelNames.size() - 1) nSelection = vLevelNames.size() - 1;
				}

				if (GetKey(olc::UP).bPressed)
				{
					nSelection--;
					if (nSelection < 0) nSelection = 0;
				}

				if (GetKey(olc::ENTER).bReleased)
				{
					workerThread = std::thread([this]() {atomicWorkerThreadActive = true; LoadLevelData(nSelection); done = true; });
					//auto future = std::async(std::launch::async, [](void* e) -> void {static_cast<Editor*>(e)->LoadLevelData(static_cast<Editor*>(e)->nSelection); }, this);
					//nMode = MODE_TEST;

					// Use wait_for() with zero milliseconds to check thread status.
					//auto status = future.wait_for(0ms);

					// Print status.
					/*if (status == std::future_status::ready) {
						std::cout << "Thread finished" << std::endl;
					}
					else {
						std::cout << "Thread still running" << std::endl;
					}*/

					//void result = future.get(); // Get result.


					//return true;
					//if (LoadLevelData(nSelection))
					//{
					//	//workerThread = std::thread([](void* e) -> bool {return static_cast<Editor*>(e)->LoadLevelData(static_cast<Editor*>(e)->nSelection); });

					//	ClearAllLayers(olc::BLANK);
					//	nMode = MODE_EDIT;
					//	return true; // So that after we clear the screen, the stuff below doesn't get drawn again before switching modes(I'm trying to avoid using Clear() on all layer every fram in the edit screen)
					//}
					//else
					//	nMode = MODE_ERROR;
				}

				// Mouse over New Level Box
				if (GetMouseX() <= ScreenWidth() - 100 && GetMouseX() >= ScreenWidth() - 200
					&& GetMouseY() <= 170 && GetMouseY() >= 100)
				{
					p1 = olc::YELLOW; p2 = olc::YELLOW;
					// Mouse click on new level box
					if (GetMouse(0).bReleased)
						nMode = MODE_NEW;
				}
				else
				{
					p1 = olc::DARK_GREY;
					p2 = olc::WHITE;
				}
			}

			Clear(olc::DARK_GREEN);
			DrawString(olc::vi2d(5, 5), "Choose file to edit, or select \"New Level\"");

			// New Level
			FillRect(olc::vi2d(ScreenWidth() - 200, 100), olc::vi2d(100, 70), olc::CYAN);
			DrawRect(olc::vi2d(ScreenWidth() - 200, 100), olc::vi2d(100, 70), p1);
			DrawRect(olc::vi2d(ScreenWidth() - 200 + 1, 100 + 1), olc::vi2d(100 - 2, 70 - 2), p1);
			DrawString(olc::vi2d(ScreenWidth() - 187, 120), "New\nLevel", p2, 2);

			// Select Level
			FillRect(olc::vi2d(nLevelSelectBoxPosX, nLevelSelectBoxPosY), olc::vi2d(nLevelSelectBoxWidth, nLevelSelectBoxHeight), olc::GREY);
			DrawRect(olc::vi2d(nLevelSelectBoxPosX, nLevelSelectBoxPosY), olc::vi2d(nLevelSelectBoxWidth, nLevelSelectBoxHeight), olc::DARK_GREY);
			DrawRect(olc::vi2d(nLevelSelectBoxPosX + 1, nLevelSelectBoxPosY + 1), olc::vi2d(nLevelSelectBoxWidth - 2, nLevelSelectBoxHeight - 2), olc::DARK_GREY);

			nSelectionOffset = (nSelection + 1) - nVisibleLevelNames; // Determine which level names to show
			if (nSelectionOffset < 0) nSelectionOffset = 0;

			FillRect(olc::vi2d(nLevelSelectBoxPosX + 3, ((nSelection - nSelectionOffset) * 25 + 8 - 5) + nLevelSelectBoxPosY), olc::vi2d(nLevelSelectBoxWidth - 5, 18), olc::DARK_YELLOW); // Selction Box
			for (int i = 0; i < nVisibleLevelNames && i < vLevelNames.size(); i++)
			{
				DrawString(olc::vi2d(nLevelSelectBoxPosX + 6, (i * 25 + 8) + nLevelSelectBoxPosY), vLevelNames[i + nSelectionOffset]);
			}
		}
		else // Loading level thread started
		{
			DrawStringDecal({ float(ScreenWidth() / 2 - 20), float(ScreenHeight() / 3) }, "Loading: " + std::to_string(nLevelLoadingPercent) + "%", olc::BLACK, { 1.0f, 1.0f });

			if (done) // Loading level thread is finished
			{
				if (workerThread.joinable())
					workerThread.join();

				ClearAllLayers(olc::BLANK);
				nMode = MODE_EDIT;
			}
		}
		return true;
	}

	bool UpdateEditScreen(float fElapsedTime)
	{
		// Some calculations for tiles

		// Determine tile size with scaling factors for fixed window size	
		int nTileWidth = 22 * fScalingFactor; // Redefine width and height for varying zoom (fScalingFactor) levels
		int nTileHeight = 22 * fScalingFactor;
		float fVisibleTilesX = (float)nGameScreenWidth / nTileWidth; // This is a float now because we can see sometimes a fraction of a tile when we zoom in
		float fVisibleTilesY = (float)nGameScreenHeight / nTileHeight;

		// Get offsets for where to start drawing tiles if zoomed out far enough (So that it doesn't lock to top left corner)
		//float fViewOffsetX = fVisibleTilesX > nWidth ? ((float)(ScreenWidth() / 2 - (fVisibleTilesX * nTileWidth) / 2)) : 0.0f;
		//float fViewOffsetY = fVisibleTilesY > nHeight ? ((float)(ScreenHeight() / 2 - (fVisibleTilesY * nTileHeight) / 2)) : 0.0f;

		//Calculate Top-Leftmost visible tile
		float fOffsetX = fCameraPosX - (float)fVisibleTilesX / 2.0f;
		float fOffsetY = fCameraPosY - (float)fVisibleTilesY / 2.0f;

		// Get offsets for smooth movement
		float fTileOffsetX = (fOffsetX - (int)fOffsetX) * nTileWidth; // Note: this is already scaled by the fScalingFactor (in nTileWidth / nTileHeight)
		float fTileOffsetY = (fOffsetY - (int)fOffsetY) * nTileHeight;

		if (IsFocused())
		{
			
			if (GetKey(olc::P).bReleased)
			{
				if (SaveLevel(nSelection))
					bSaveOkay = true;
				else
					bSaveOkay = false;

				fSaveTextTimer = 3.0f;
				bShowSaveText = true;
			}

			//ZOOM
			if (GetKey(olc::DOWN).bPressed)
			{
				fScalingFactor -= 0.1f;
			}

			if (GetKey(olc::UP).bPressed)
			{
				fScalingFactor += 0.1f;
			}
			if (GetMouseWheel() > 0)
			{
				fScalingFactor += 0.1f;
			}
			if (GetMouseWheel() < 0)
			{
				fScalingFactor -= 0.1f;
			}
			// END ZOOM

			// CAMERA MOVEMENT
			// Note: CameraPos and ViewOffset are clamped below, so we don't check for "zoom level" (fVisibleTiles > nWidth/nHeight) to determine which "camera" to adjust
			if (GetKey(olc::W).bHeld)
			{
				// Zoomed in camera
				if (GetKey(olc::SHIFT).bHeld)
					fCameraPosY -= 60.0f * fElapsedTime;
				else
					fCameraPosY -= 3.0f * fElapsedTime;

				// Zoomed out camera
				if (GetKey(olc::SHIFT).bHeld)
					fViewOffsetY += 200.0f * fElapsedTime;
				else
					fViewOffsetY += 50.0f * fElapsedTime;
			}

			if (GetKey(olc::S).bHeld)
			{
				// Zoomed in camera
				if (GetKey(olc::SHIFT).bHeld)
					fCameraPosY += 60.0f * fElapsedTime;
				else
					fCameraPosY += 3.0f * fElapsedTime;

				// Zoomed out camera
				if (GetKey(olc::SHIFT).bHeld)
					fViewOffsetY -= 200.0f * fElapsedTime;
				else
					fViewOffsetY -= 50.0f * fElapsedTime;
			}
			if (GetKey(olc::A).bHeld)
			{
				// Zoomed in camera
				if (GetKey(olc::SHIFT).bHeld)
					fCameraPosX -= 60.0f * fElapsedTime;
				else
					fCameraPosX -= 3.0f * fElapsedTime;

				// Zoomed out camera
				if (GetKey(olc::SHIFT).bHeld)
					fViewOffsetX += 200.0f * fElapsedTime;
				else
					fViewOffsetX += 50.0f * fElapsedTime;
			}

			if (GetKey(olc::D).bHeld)
			{
				// Zoomed in camera
				if (GetKey(olc::SHIFT).bHeld)
					fCameraPosX += 60.0f * fElapsedTime;
				else
					fCameraPosX += 3.0f * fElapsedTime;

				// Zoomed out camera
				if (GetKey(olc::SHIFT).bHeld)
					fViewOffsetX -= 200.0f * fElapsedTime;
				else
					fViewOffsetX -= 50.0f * fElapsedTime;
			}

			if (GetMouse(1).bHeld)
			{
				// Zoomed in camera
				fCameraPosX -= (GetMousePos().x - fOldMousePos.x) / nTileWidth;
				fCameraPosY -= (GetMousePos().y - fOldMousePos.y) / nTileHeight;

				// Zoomed out camera
				fViewOffsetX += (GetMousePos().x - fOldMousePos.x); // fViewOffset in screen/pixel units, fCameraPos in tile units
				fViewOffsetY += (GetMousePos().y - fOldMousePos.y);
			}
			fOldMousePos = GetMousePos();

			// END CAMERA MOVEMENT

			// Mouse over a tile selection
			for (int i = 0; i < vecTileNames.size(); i++)
			{
				if (GetMouseX() > nTileBoxPosX + 3 && GetMouseX() < nTileBoxPosX + nTileBoxWidth
					&& GetMouseY() > nTileBoxPosY + (i * 30 + 13 - 10) && GetMouseY() < nTileBoxPosY + (i * 30 + 13 - 10) + 28)
				{
					nTilePotentialSelect = i;

					if (GetMouse(0).bReleased)
					{
						nTileSelection = i;
					}
					break; // Need this to break out of for loop because of the below else statement. Otherwise this would always set nTilePotentialSelect to -1
				}
				else
				{
					nTilePotentialSelect = -1; // -1 means do not draw the selection box - prevents the mouse-over selection box from lingering if your mouse is no longer over it
				}
			}

			// Use mouse to edit tiles
			if (GetMouseX() >= vGameScreenPos.x && GetMouseX() < vGameScreenPos.x + nGameScreenWidth
				&& GetMouseY() >= vGameScreenPos.y && GetMouseY() < vGameScreenPos.y + nGameScreenHeight)
			{
				int nMouseNewX = GetMouseX() - vGameScreenPos.x;
				int nMouseNewY = GetMouseY() - vGameScreenPos.y;

				nSelectedTileX = std::floor((nMouseNewX + fTileOffsetX - fViewOffsetX) / nTileWidth); // Normalize mouse coordinates to tile space
				nSelectedTileY = std::floor((nMouseNewY + fTileOffsetY - fViewOffsetY) / nTileHeight); // Floor is so when we mouse over into the negative range(left/top side of level), it won't integer cut off to 0, 
																										// keeping tile 0 selected until we get all the way to -1

				if (nSelectedTileX + fOffsetX < nWidth && nSelectedTileX >= 0
					&& nSelectedTileY + fOffsetY < nHeight && nSelectedTileY >= 0)
				{
					DrawStringDecal({ (float)vGameScreenPos.x, (float)vGameScreenPos.y - 10 }, std::to_string((int)(nSelectedTileX + fOffsetX)) + ", " + std::to_string((int)(nSelectedTileY + fOffsetY)));

					// Set that tile to selected tile
					if (GetMouse(0).bHeld)
					{
						for (int i = 0; i < nBrushSize; i++)
						{
							for (int j = 0; j < nBrushSize; j++)
							{
								if (nSelectedTileX + fOffsetX + i < nWidth && nSelectedTileY + fOffsetY + j < nHeight)
								{
									delete GetTile(nSelectedTileX + fOffsetX + i, nSelectedTileY + fOffsetY + j);
									SetTile(nSelectedTileX + fOffsetX + i, nSelectedTileY + fOffsetY + j, new cTile(vecTileNames[nTileSelection], nTileSelection, true, true));

								}
							}
						}
					}
					if (GetKey(olc::SPACE).bHeld && GetMouse(0).bHeld)
					{
						for (int i = 0; i < nBrushSize; i++)
						{
							for (int j = 0; j < nBrushSize; j++)
							{
								if (nSelectedTileX + fOffsetX + i < nWidth && nSelectedTileY + fOffsetY + j < nHeight)
								{
									delete GetTile(nSelectedTileX + fOffsetX + i, nSelectedTileY + fOffsetY + j);
									SetTile(nSelectedTileX + fOffsetX + i, nSelectedTileY + fOffsetY + j, new cTile("Blank", -1, false, false));
								}
							}
						}
					}
				}
				else
				{
					nSelectedTileX = -1; // Don't draw
					nSelectedTileY = -1;
				}
			}
			else
			{
				nSelectedTileX = -1;
				nSelectedTileY = -1;
			}

			// Update buttons
			for (int i = 0; i < vecButtons.size(); i++)
			{
				if (vecButtons[i]->IsPointInside(olc::vf2d(GetMouseX(), GetMouseY())))
				{
					vecButtons[i]->OnMouseOver();

					if (GetMouse(0).bReleased)
					{
						vecButtons[i]->OnClick(this);
					}
				}
				else
					vecButtons[i]->OnMouseExit();
			}

			//Hover over info button
			if (GetMouseX() > (float)ScreenWidth() - 28 && GetMouseX() < (float)ScreenWidth() - 28 + (Assets::get().GetSprite("Info")->width * 0.2f)
				&& GetMouseY() > 4 && GetMouseY() < 4 + (Assets::get().GetSprite("Info")->height * 0.2f))
			{
				// Draw Info Button Highlighted
				DrawDecal({ (float)ScreenWidth() - 27, 5 }, Assets::get().GetDecal("Info"), { 0.2f, 0.2f }, { 0, olc::CYAN.g, olc::CYAN.b });

				if (GetMouse(0).bReleased)
				{
					nMode = MODE_INFO;
				}
			}
			else
			{
				// Draw Info Button
				DrawDecal({ (float)ScreenWidth() - 27, 5 }, Assets::get().GetDecal("Info"), { 0.2f, 0.2f });
			}
		}

		// Clamp camera to game boundaries (Before just clamped fOffset, but this makes the camera respond instantly)
		if (fCameraPosX > nWidth - fVisibleTilesX / 2) fCameraPosX = nWidth - fVisibleTilesX / 2;
		if (fCameraPosY > nHeight - fVisibleTilesY / 2) fCameraPosY = nHeight - fVisibleTilesY / 2;
		if (fCameraPosX < fVisibleTilesX / 2) fCameraPosX = fVisibleTilesX / 2;
		if (fCameraPosY < fVisibleTilesY / 2) fCameraPosY = fVisibleTilesY / 2;


		// Clamp camera to game boundaries (maybe excessive with the above in place, because fCameraPos determines fOffset, but just in case)
		if (fOffsetX > nWidth - fVisibleTilesX) { fOffsetX = nWidth - fVisibleTilesX; }
		if (fOffsetY > nHeight - fVisibleTilesY) fOffsetY = nHeight - fVisibleTilesY;
		if (fOffsetX < 0) fOffsetX = 0;
		if (fOffsetY < 0) fOffsetY = 0;

		// Clamp fViewOffset to screen boundaries
		if (fVisibleTilesX > nWidth)
		{
			if (fViewOffsetX > nGameScreenWidth - nWidth * nTileWidth) fViewOffsetX = nGameScreenWidth - nWidth * nTileWidth;
			if (fViewOffsetX < 0) fViewOffsetX = 0;
		}
		else
			fViewOffsetX = 0;

		if (fVisibleTilesY > nHeight)
		{
			if (fViewOffsetY > nGameScreenHeight - nHeight * nTileHeight) fViewOffsetY = nGameScreenHeight - nHeight * nTileHeight;
			if (fViewOffsetY < 0) fViewOffsetY = 0;
		}
		else
			fViewOffsetY = 0;

		// Clamp zoom
		if (fScalingFactor < 0.2f) fScalingFactor = 0.2f; // Prevent divide by 0 and limit zoom out
		if (fScalingFactor > 7.2f) fScalingFactor = 7.2f; // Don't need to go any bigger than this

		//Clamp brush size
		if (nBrushSize < 1) nBrushSize = 1;
		if (nBrushSize > nWidth / 2) nBrushSize = nWidth / 2;
		if (nBrushSize > nHeight / 2) nBrushSize = nHeight / 2;

		// ========================================================================================== LAYER 2 ==========================================================================================
		SetDrawTarget(2);

		// Edit Window Background Color fill
		FillRectDecal(vGameScreenPos, { (float)nGameScreenWidth, (float)nGameScreenHeight }, olc::DARK_CYAN);
		FillRectDecal(vGameScreenPos + olc::vf2d(fViewOffsetX, fViewOffsetY), { (float)(nWidth * nTileWidth), (float)(nHeight * nTileHeight) }, editScreenColor);

		// ======================== Draw Tiles in Edit Window ===============================
		//Draw visible tile map (overdraw 2 tiles to account for all situations with zooming/fScalingFactor)
		for (int x = 0; x < fVisibleTilesX + 2; x++)
		{
			for (int y = 0; y < fVisibleTilesY + 2; y++)
			{
				if (x + fOffsetX < nWidth && y + fOffsetY < nHeight) // Because we're overdrawing, we want to prevent accessing a tile outside of range
				{
					/*
					//================================== PARTIAL SPRITE TO PREVENT LEAKING OVER SCREEN BOUNDARY - WORKS OK ==========================================================
					// This whole mess is because I wanted a more elegant way than "overdrawing" as explained in Javid's platform game vid. Also the whole reason we're using decals - to scale by a float value w/o the performance penalties of the GFX2D library
					if (x == 0 && y != 0)
						DrawPartialDecal(olc::vi2d(x * nTileWidth + vGameScreenPos.x, y * nTileHeight - fTileOffsetY + vGameScreenPos.y), GetTile(x + fOffsetX, y + fOffsetY)->decal, olc::vi2d(fTileOffsetX / fScalingFactor, 0), olc::vi2d(GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->width, GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->height) - olc::vi2d(fTileOffsetX / fScalingFactor, 0), olc::vf2d(fScalingFactor, fScalingFactor));
					//DrawPartialSprite(x * nTileWidth + vGameScreenPos.x, y * nTileHeight - fTileOffsetY + vGameScreenPos.y, GetTile(x + fOffsetX, y + fOffsetY)->sprite, fTileOffsetX, fTileOffsetY, GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->width - fTileOffsetX, GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->height - fTileOffsetY, 2);
					if (y == 0 && x != 0)
						DrawPartialDecal(olc::vi2d(x * nTileWidth - fTileOffsetX + vGameScreenPos.x, y * nTileHeight + vGameScreenPos.y), GetTile(x + fOffsetX, y + fOffsetY)->decal, olc::vi2d(0, fTileOffsetY / fScalingFactor), olc::vi2d(GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->width, GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->height) - olc::vi2d(0, fTileOffsetY / fScalingFactor), olc::vf2d(fScalingFactor, fScalingFactor));
					if (x == 0 && y == 0)
						DrawPartialDecal(olc::vi2d(x * nTileWidth + vGameScreenPos.x, y * nTileHeight + vGameScreenPos.y), GetTile(x + fOffsetX, y + fOffsetY)->decal, olc::vi2d(fTileOffsetX / fScalingFactor, fTileOffsetY / fScalingFactor), olc::vi2d(GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->width, GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->height) - olc::vi2d(fTileOffsetX / fScalingFactor, fTileOffsetY / fScalingFactor), olc::vf2d(fScalingFactor, fScalingFactor));
					if (x != 0 && y != 0)
						DrawDecal(olc::vi2d(x * nTileWidth - fTileOffsetX + vGameScreenPos.x, y * nTileHeight - fTileOffsetY + vGameScreenPos.y), GetTile(x + fOffsetX, y + fOffsetY)->decal, olc::vf2d(fScalingFactor, fScalingFactor));

					if (x == fVisibleTilesX || x == nWidth)
						DrawPartialDecal(olc::vi2d(x * nTileWidth + vGameScreenPos.x, y * nTileHeight - fTileOffsetY + vGameScreenPos.y), GetTile(x + fOffsetX, y + fOffsetY)->decal, olc::vi2d(0, 0), olc::vi2d(GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->width, GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->height) - olc::vi2d(fTileOffsetX / fScalingFactor, 0), olc::vf2d(fScalingFactor, fScalingFactor));
					//==============================================================================================================================================================
					*/

					// =============================== NORMAL OVERDRAW LIKE IN VIDEO - Much better======================================
					DrawDecal(olc::vf2d(x * nTileWidth - fTileOffsetX + vGameScreenPos.x + fViewOffsetX, y * nTileHeight - fTileOffsetY + vGameScreenPos.y + fViewOffsetY), GetTile(x + fOffsetX, y + fOffsetY)->decal, olc::vf2d (fScalingFactor, fScalingFactor));
					//====================================================================================================
				}
			}
		}

		// Highlight selected tile
		if (nSelectedTileX != -1 && nSelectedTileY != -1)
		{
			for (int i = 0; i < nBrushSize; i++)
			{
				for (int j = 0; j < nBrushSize; j++)
				{
					if (nSelectedTileX + fOffsetX + i < nWidth && nSelectedTileY + fOffsetY + j < nHeight)
					{
						FillRectDecal({ (nSelectedTileX + i) * nTileWidth - fTileOffsetX + vGameScreenPos.x + fViewOffsetX, (nSelectedTileY + j) * nTileHeight - fTileOffsetY + vGameScreenPos.y + fViewOffsetY }, { (float)nTileWidth, (float)nTileHeight }, olc::Pixel(~editScreenColor.r, ~editScreenColor.g, ~editScreenColor.b, 75));
					}
				}
			}
		}

		//  ======================== END: Draw Tiles in Edit Window ===============================

		// ========================================================================================== LAYER 1 ==========================================================================================
		SetDrawTarget(1);
		ClearEditScreen(olc::VERY_DARK_BLUE);

		// ========================================================================================== LAYER 0 ==========================================================================================
		SetDrawTarget(nullptr);

		// Edit Window Border
		DrawRect(vGameScreenPos - olc::vi2d(1, 1), olc::vi2d((nGameScreenWidth)+1, (nGameScreenHeight)+1), olc::VERY_DARK_GREY);
		DrawRect(vGameScreenPos - olc::vi2d(2, 2), olc::vi2d((nGameScreenWidth)+3, (nGameScreenHeight)+3), olc::DARK_GREY);

		//Level Name and size
		DrawStringDecal({ (float)vGameScreenPos.x, 5 }, std::to_string(nSelection + 1) + ": " + vLevelNames[nSelection] + " (" + std::to_string(nWidth) + ", " + std::to_string(nHeight) + ")");

		// ========================= Tile selection Window ====================================
		FillRectDecal(olc::vi2d(nTileBoxPosX + 2, nTileBoxPosY + 2), olc::vi2d(nTileBoxWidth - 3, nTileBoxHeight - 3), olc::GREY);
		DrawRect(olc::vi2d(nTileBoxPosX + 1, nTileBoxPosY + 1), olc::vi2d(nTileBoxWidth - 2, nTileBoxHeight - 2), olc::VERY_DARK_GREY);
		DrawRect(olc::vi2d(nTileBoxPosX, nTileBoxPosY), olc::vi2d(nTileBoxWidth, nTileBoxHeight), olc::DARK_GREY);

		nTileSelectionOffset = (nTileSelection + 1) - nVisibleTileNames; // Determine which tile names to show
		if (nTileSelectionOffset < 0) nTileSelectionOffset = 0;

		if (nTilePotentialSelect != -1)
			FillRectDecal(olc::vi2d(nTileBoxPosX + 3, ((nTilePotentialSelect - nTileSelectionOffset) * 30 + 13 - 10) + nTileBoxPosY), olc::vi2d(nTileBoxWidth - 5, 28), olc::DARK_YELLOW); // Mouse over Selction Box

		FillRectDecal(olc::vi2d(nTileBoxPosX + 3, ((nTileSelection - nTileSelectionOffset) * 30 + 13 - 10) + nTileBoxPosY), olc::vi2d(nTileBoxWidth - 5, 28), olc::DARK_RED); // Selction Box

		//Show brush size below window
		DrawStringDecal({ float(nTileBoxPosX + (nTileBoxWidth / 8)), float(nTileBoxPosY + nTileBoxHeight + 3) }, "Brush Size: " + std::to_string(nBrushSize), olc::DARK_YELLOW);

		for (int i = 0; i < nVisibleTileNames && i < vecTileNames.size(); i++)
		{
			if (vecTileNames[i + nTileSelectionOffset].length() > 20) //If string is long, show Sprite before name so that it gets shown
			{

			}
			else // Show the Name first
			{

			}
			// Sprite before Name
			//SetPixelMode(olc::Pixel::MASK);
			//DrawSprite(olc::vi2d(nTileBoxPosX + 6, (i * 30 + 13/2) + nTileBoxPosY), Assets::get().GetSprite(vecTileNames[i + nTileSelectionOffset]));
			//(olc::Pixel::NORMAL);
			//DrawString(olc::vi2d(nTileBoxPosX + nTileBoxWidth - (vecTileNames[i + nTileSelectionOffset].length() * 8) - 8, (i * 30 + 13) + nTileBoxPosY), vecTileNames[i + nTileSelectionOffset]);

			// Name before Sprite
			SetPixelMode(olc::Pixel::MASK);
			DrawDecal(olc::vi2d(nTileBoxPosX + 6 + (vecTileNames[i + nTileSelectionOffset].length() * 8) + 8, (i * 30 + 13 / 2) + nTileBoxPosY), Assets::get().GetDecal(vecTileNames[i + nTileSelectionOffset]));
			SetPixelMode(olc::Pixel::NORMAL);
			DrawStringDecal(olc::vi2d(nTileBoxPosX + 6, (i * 30 + 13) + nTileBoxPosY), vecTileNames[i + nTileSelectionOffset]);
		}
		// ========================= END: Tile selection Window ====================================

		// ========================= END: Brush size buttons ====================================



		// Draw buttons
		for (int i = 0; i < vecButtons.size(); i++)
		{
			vecButtons[i]->DrawSelf(this);
		}



		// ========================= Show text that level was saved ====================================
		if (bShowSaveText)
		{
			if (fElapsedTime < 2.0f) // If saving takes a long time, we don't want to count that first frame - it could be more than 3 seconds
				fSaveTextTimer -= fElapsedTime;

			if (bSaveOkay)
			{
				DrawStringDecal({ float(ScreenWidth() / 2 - 25), 5 }, "Level Saved!", olc::WHITE, { 2.0f, 2.0f });
			}
			else
			{
				DrawStringDecal({ float(ScreenWidth() / 2 - 25), 3 }, "Level FALED to Save!\n" + sSaveErrorMsg + "\nMaybe try again?", olc::WHITE, { 1.0f, 1.0f });
			}

			if (fSaveTextTimer <= 0.0f)
				bShowSaveText = false;
		}
		// =============================================================================================

		

		return true;
	}

	cTile* GetTile(int x, int y)
	{
		if (x >= 0 && x < nWidth && y >= 0 && y < nHeight)
			return vecTiles[y * nWidth + x];
		else
		{
			nErrorCode = 3;
			nMode = MODE_ERROR;
			return new cTile("Not_Found", -2, true, false); // I will rationalize doing this, which normally causes a memory leak, because we are just returning SOMETHING only to get to the error screen
			//return nullptr; // Makes it crash if something goes wrong
			// NO -> this makes a memory leak  return new cTile("Not_Found", true, false); // Use the error tile sprite to show an obvious problem if the index is not within range
		}
	}

	void SetTile(int x, int y, cTile* t)
	{
		if (x >= 0 && x < nWidth && y >= 0 && y < nHeight)
			vecTiles[y * nWidth + x] = t;
	}


	void CreateKeyMap()
	{
		std::ifstream inFile("Tiles.key", std::ios::in | std::ios::binary);
		if (inFile.is_open())
		{
			// Later use a similar map<string, int> that we will use to construct the tiles into a file to save it. If a string isnt recognized/in the map, we could write a -1 for "not Found sprite"
			std::string s;
			while (!inFile.eof())
			{
				inFile >> s;
				vecTileNames.push_back(s);

			}
		}
	}

	bool LoadLevelNames()
	{
		bool bLoaded = false;
		int i = 1;
		while (!bLoaded)
		{
			std::fstream myfile;
			myfile.open(LEVEL_PATH + std::to_string(i) + ".lvl");
			if (myfile.is_open())
			{
				std::string ss;
				std::getline(myfile, ss);
				vLevelNames.push_back(ss);
				myfile.close();
				i++;
			}
			else
			{
				std::cout << "Successfully loaded " << i - 1 << " levels." << std::endl;
				bLoaded = true;
				return true;
			}
		}
	}

	bool LoadLevelData(int selection)
	{
		nLevelLoadingCnt = 0;
		nLevelLoadingPercent = 0;
		std::ifstream inFile(LEVEL_PATH + std::to_string(selection + 1) + ".lvl", std::ios::in | std::ios::binary);
		if (inFile.is_open())
		{
			std::string s; // Already have name, don't do anything with this
			std::getline(inFile, s);

			inFile >> nWidth >> nHeight;
			if (nWidth == 0 || nHeight == 0) // Default for these values is 0, so if not changed there is no level
			{
				std::cerr << "Ensure Level Width and Level Height are respectively defined in the first 2 lines after the level name." << std::endl;
				nErrorCode = 2;
				return false;
			}

			// File should be good, read
			vecTiles = new cTile* [nWidth * nHeight]; // Create array

			for (int x = 0; x < nWidth * nHeight; x++)
			{
				vecTiles[x] = nullptr; //Initialize array to nullptr - So if we don't initialize every tile of the level, we can check for that after file read
			}

			//Read into a string first using >> so that we don't get whitespace in the way
			int nTileType, nTileSolid, nTileBreakable;
			for (int y = 0; y < nHeight; y++)
			{
				for (int x = 0; x < nWidth; x++)
				{
					inFile >> nTileType >> nTileSolid >> nTileBreakable;

					//Interogate vector<string> vecTileNames at vecTileNames[n] to get the string name for the tile
					if (nTileType >= 0 && nTileType < vecTileNames.size()) {
						vecTiles[y * nWidth + x] = new cTile(vecTileNames[nTileType], nTileType, nTileSolid, nTileBreakable);
					}
					else if (nTileType == -1) {
						vecTiles[y * nWidth + x] = new cTile("Blank", -1, nTileSolid, nTileBreakable);
					}
					else
					{
						//std::cerr << "Tile provided at (" << x << ", " << y << ") is not within the range of tiles provided in Tiles.key" << std::endl; // This takes too long if file is really big
						vecTiles[y * nWidth + x] = new cTile("Not_Found", nTileType, true, false); // Will create a tile with a "tile not found" sprite, or something similar as defined in Assets class, to be obvious to the developer/designer that something is wrong
					}
					nLevelLoadingCnt++;
				}
				if (int(((float)nLevelLoadingCnt / (nWidth * nHeight)) * 100) > nLevelLoadingPercent)
				{
					nLevelLoadingPercent = int(((float)nLevelLoadingCnt / (nWidth * nHeight)) * 100);
				}
			}
			return true;
		}
		else
		{
			std::cerr << "Could not open " << LEVEL_PATH + std::to_string(selection + 1) + ".lvl" << std::endl;
			nErrorCode = 1;
			return false;
		}
	}

	bool SaveLevel(int selection)
	{
		std::ofstream outFile;
		outFile.open(LEVEL_PATH + std::string("Last_Save.lvl")); // Don't open file directly because in the case it doesn't open correcly, it will delete all data

		if (outFile.is_open())
		{
			if (selection < vLevelNames.size())
				outFile << vLevelNames[selection] << std::endl;
			outFile << nWidth << std::endl << nHeight << std::endl;

			for (int i = 0; i < nWidth * nHeight; i++)
			{
				// Whether the tile is valid, blank, or not found, this will save in file whatever the tileID is.
				// This is to preserve data for a tile that might be not found in Tiles.key when level is loaded.
				// In case we forget to update Tiles.key and there is a tile out of its range, this will save it the
				// same as we loaded it so we don't lose the data on what that tile was. If it is something out of range,
				// It will still be caught as out of range when we load the level back.
				outFile << vecTiles[i]->iD << " " << vecTiles[i]->solid << " " << vecTiles[i]->bBreakable << " "; 
			}
			outFile.close();

			// Level now saved to "Last_Save.lvl" - so copy to our intended target
			std::error_code ec;
			std::filesystem::copy(LEVEL_PATH + std::string("Last_Save.lvl"), LEVEL_PATH + std::to_string(selection + 1) + ".lvl", std::filesystem::copy_options::overwrite_existing, ec);
			if (ec) {
				if (ec == std::errc::file_exists) {
					// special error handling for file_exists
					// ( Don't need this but keeping it just as an example for how to do specific error checks)

					std::cerr << "Error: " << ec.message() << "\n";
					sSaveErrorMsg = "Error: " + ec.message();
					return false;
				}
				else {
					// generic error handling for all other errors
					// that you don't specifically care about
					std::cerr << "Error: " << ec.message() << "\n";
					sSaveErrorMsg = "Error: " + ec.message();
					return false;
				}
			}
			return true;
		}
		else
		{
			sSaveErrorMsg = "Error: Could not open file for saving.";
			return false; // False will display something on the screen telling user to try again
		}
	}

	bool UpdateNewScreen(float fElapsedTime)
	{
		bool bReady = false;
		if (IsFocused())
		{
			if (GetKey(olc::ENTER).bReleased)
				bReady = true;
		}

		// Update buttons
		for (int i = 0; i < vecNewScreenButtons.size(); i++)
		{
			if (vecNewScreenButtons[i]->IsPointInside(olc::vf2d(GetMouseX(), GetMouseY())))
			{
				vecNewScreenButtons[i]->OnMouseOver();

				if (GetMouse(0).bReleased)
				{
					vecNewScreenButtons[i]->OnClick(this);
					fNewScreenTimer = 0.0f;
				}

				if (GetMouse(0).bHeld) // Button is held in
				{
					if (fElapsedTime > 0.0f)
						fNewScreenTimer += fElapsedTime;
					else
						fNewScreenTimer += 0.0006f; // So if framerate is too high and fElapsedTime is 0, we can still do this

					if (fNewScreenTimer > 0.8f)
					{
						vecNewScreenButtons[i]->OnClick(this);
					}
				}
			}
			else
				vecNewScreenButtons[i]->OnMouseExit();

			if (GetMouse(0).bReleased)
			{
				fNewScreenTimer = 0.0f; // Reset timer even if released click outside of button boundaries
			}
		}

		if (newX < 0) newX = 0;
		if (newY < 0) newY = 0;

		// Draw Buttons
		for (int i = 0; i < vecNewScreenButtons.size(); i++)
		{
			vecNewScreenButtons[i]->DrawSelf(this);
		}

		// Enter Level Name
		Clear(olc::DARK_BLUE);
		DrawString(olc::vi2d((ScreenWidth() / 2) - (17 * 8), 30), "Enter Level Name:", olc::WHITE, 2);
		FillRect(olc::vi2d(ScreenWidth() / 5, 100), olc::vi2d(400, 50), olc::GREY);
		DrawRect(olc::vi2d(ScreenWidth() / 5, 100), olc::vi2d(400, 50), olc::DARK_GREY);
		DrawRect(olc::vi2d(ScreenWidth() / 5 + 1, 100 + 1), olc::vi2d(400 - 2, 50 - 2), olc::DARK_GREY);

		CharInput(s);
		DrawString(olc::vi2d(ScreenWidth() / 5 + 5, 115), s, olc::WHITE, 2);

		// Enter Level Size
		DrawString(olc::vi2d((ScreenWidth() / 2) - (17 * 8), 180), "Enter Level Size:", olc::WHITE, 2);

		DrawString(olc::vi2d(ScreenWidth() / 2 - 160, 250), "X:", olc::RED, 2);
		FillRect(olc::vi2d(ScreenWidth() / 2 - 120 - 10, 240), olc::vi2d(75, 35), olc::GREY);
		DrawRect(olc::vi2d(ScreenWidth() / 2 - 120 - 10, 240), olc::vi2d(75, 35), olc::DARK_GREY);
		DrawRect(olc::vi2d(ScreenWidth() / 2 - 120 - 10 + 1, 240 + 1), olc::vi2d(75 - 2, 35 - 2), olc::DARK_GREY);
		DrawString(olc::vi2d(ScreenWidth() / 2 - 118, 250), std::to_string(newX) , olc::RED, 2);

		DrawString(olc::vi2d(ScreenWidth() / 2 - 20, 250), "by", olc::WHITE, 2);

		DrawString(olc::vi2d(ScreenWidth() / 2 + 35, 250), "Y:", olc::DARK_GREEN, 2);
		FillRect(olc::vi2d(ScreenWidth() / 2 + 65, 240), olc::vi2d(75, 35), olc::GREY);
		DrawRect(olc::vi2d(ScreenWidth() / 2 + 65, 240), olc::vi2d(75, 35), olc::DARK_GREY);
		DrawRect(olc::vi2d(ScreenWidth() / 2 + 65 + 1, 240 + 1), olc::vi2d(75 - 2, 35 - 2), olc::DARK_GREY);
		DrawString(olc::vi2d(ScreenWidth() / 2 + 67 + 10, 250), std::to_string(newY), olc::DARK_GREEN, 2);


		

		if (bReady)
		{
			myfile.open(LEVEL_PATH + std::to_string(vLevelNames.size() + 1) + ".lvl", std::ios::out); // Open file to create it and save level name
			if (myfile.is_open())
			{
				std::cout << LEVEL_PATH + std::to_string(vLevelNames.size() + 1) + ".lvl" + " - File Successfully Created." << std::endl;
				myfile << s << std::endl;
				myfile << newX << " " << newY << std::endl;;

				for (int i = 0; i < newX * newY; i++)
				{
					myfile << -1 << " " << 0 << " " << 0 << " ";
				}
			}
			else
				std::cout << "Unable to create file: " << LEVEL_PATH + std::to_string(vLevelNames.size() + 1) + ".lvl" << std::endl; // Should probably be a new error code later

			myfile.close();

			nSelection = vLevelNames.size();
			vLevelNames.push_back(s);
			
			if (LoadLevelData(nSelection))
			{
				ClearAllLayers(olc::BLANK);
				nMode = MODE_EDIT;
			}
			else
				nMode = MODE_ERROR;
		}

		return true;
	}

	bool UpdateErrorScreen(float fElapsedTime)
	{
		Clear(olc::VERY_DARK_RED);
		DrawString(olc::vi2d(5, 10), "OH NO, SOMETHING BAD HAPPENED...", olc::DARK_GREY, 2);

		SetPixelMode(olc::Pixel::MASK);
		DrawDecal(olc::vi2d(ScreenWidth() / 2 - 47, 30), Assets::get().GetDecal("Error"), { 0.15f, 0.15f });
		SetPixelMode(olc::Pixel::NORMAL);

		DrawString(olc::vi2d(5, 140), "Error Code: " + std::to_string(nErrorCode), olc::DARK_GREY, 2);

		switch (nErrorCode)
		{
		case 0:
			DrawString(olc::vi2d(5, 170), "Default Error Code.\nNot sure what happened...", olc::DARK_GREY, 2);
			break;
		case 1:
			DrawString(olc::vi2d(5, 170), "Could not open file to read level data.", olc::DARK_GREY, 2);
			break;
		case 2:
			DrawString(olc::vi2d(5, 170), "File opened to read level data,\nbut something went wrong reading it.\n\nLevel Width and Height might be missing\nfrom the 2nd and 3rd lines of the file.", olc::DARK_GREY, 2);
			break;
		case 3:
			DrawString(olc::vi2d(5, 170), "Tried indexing into an illegal spot\nin vecTiles.\n\nEnsure things that modify\nthe index (like fOffset) are correct.", olc::DARK_GREY, 2);
			break;
		}
		return true;
	}

	bool UpdateInfoScreen(float fElapsedTime)
	{
		Clear(olc::VERY_DARK_GREEN);
		DrawStringDecal({ 145, 80 }, "Controls:", olc::YELLOW, { 2.5f, 2.5f });
		DrawStringDecal({ 148, 106 }, "\tPlacing Tiles:", olc::MAGENTA, { 1.8f, 1.8f });
		DrawStringDecal({ 148, 130 }, "\t\t\tMouse:\n\n"
										"\t\t\t\t\tLeft Click: Place Tile\n"
										"\t\t\t\t\tSPACE + Left Click: Erase Tile\n\n\n", olc::MAGENTA);
		DrawStringDecal({ 148, 180 }, "\tCamera:\n\n", olc::CYAN, { 1.8f, 1.8f });
		DrawStringDecal({ 148, 204 }, "\t\t\tMouse:\n\n"
											"\t\t\t\t\tRight Click (HOLD): Move Camera\n"
											"\t\t\t\t\tMouse Wheel: Zoom\n\n"
										"\t\t\tKeyboard:\n\n"
											"\t\t\t\t\tW/A/S/D: Move Camera\n"
											"\t\t\t\t\tUP/DOWN: Zoom In/Out", olc::CYAN);

		// Draw Exit X
		int cX = ScreenWidth() - 20;
		int cY = 15;
		int cR = 10;
		DrawCircle({ cX, cY }, cR, olc::BLACK);
		DrawLine({ cX - (cR / 2), cY - (cR / 2) }, { cX + (cR / 2), cY + (cR / 2) }, olc::BLACK);
		DrawLine({ cX - (cR / 2), cY + (cR / 2) }, { cX + (cR / 2), cY - (cR / 2) }, olc::BLACK);

		if (IsFocused())
		{
			int a = std::abs(GetMouseX() - cX);
			int b = std::abs(GetMouseY() - cY);
			if (std::sqrt(a * a + b * b) < cR)
			{
				DrawCircle({ cX, cY }, cR, olc::RED);
				DrawLine({ cX - (cR / 2), cY - (cR / 2) }, { cX + (cR / 2), cY + (cR / 2) }, olc::RED);
				DrawLine({ cX - (cR / 2), cY + (cR / 2) }, { cX + (cR / 2), cY - (cR / 2) }, olc::RED);

				if (GetMouse(0).bReleased) // Back to edit mode
				{
					ClearAllLayers(olc::BLANK);
					nMode = MODE_EDIT;
				}
			}
		}

		return true;
	}

	void ClearEditScreen(olc::Pixel p) // Easier way to keep tiles from showing outside of edit window
	{
		FillRectDecal({ 0.0f, 0.0f }, { (float)ScreenWidth(), (float)vGameScreenPos.y }, p);
		FillRectDecal({ 0, (float)(vGameScreenPos.y + nGameScreenHeight) }, { (float)ScreenWidth(), (float)ScreenHeight() - (float)(vGameScreenPos.y + nGameScreenHeight) }, p);
		FillRectDecal({ 0, (float)vGameScreenPos.y }, { (float)vGameScreenPos.x, (float)nGameScreenHeight + 1.0f }, p);
		FillRectDecal({ (float)(vGameScreenPos.x + nGameScreenWidth), (float)vGameScreenPos.y }, { (float)(ScreenWidth() - (nGameScreenWidth + vGameScreenPos.x)), (float)nGameScreenHeight + 1.0f }, p);
	}

	void ClearAllLayers(olc::Pixel p)
	{
		olc::Sprite* oldTarget = GetDrawTarget();
		for (int i = 0; i < GetLayers().size(); i++)
		{
			SetDrawTarget(i);
			Clear(p);
		}
		SetDrawTarget(oldTarget);
	}

	bool UpdateTestScreen(float fElapsedTime)
	{
		

		//Clear(olc::GREEN);
		FillRectDecal({ 0, 0 }, { 90, 90 }, olc::BLUE);

		//if (nLoadLevelReturnVal == 1)
		{
			if (workerThread.joinable())
				workerThread.join();

			ClearAllLayers(olc::BLANK);
			nMode = MODE_EDIT;
		}
		//else if (nLoadLevelReturnVal == -1)
			nMode = MODE_ERROR;


		//SetDrawTarget(nullptr);
		//Clear(olc::GREEN);

		//DrawSprite({ 30, 30 }, Assets::get().GetSprite("Blank"));
		//DrawRect({ 30, 30 }, { Assets::get().GetSprite("Blank")->width, Assets::get().GetSprite("Blank")->width }, olc::BLACK);
		//DrawString({ 30, 30 }, std::to_string(Assets::get().GetSprite("Blank")->width), olc::BLACK);
		//SetDrawTarget(nullptr);
		
		//FillRect({ 20, 20 }, {50, 50}, olc::DARK_BLUE);
		//FillRectDecal({ 20, 20 }, { 50, 50 }, olc::DARK_BLUE);
		//Clear(olc::BLANK);

		//SetDrawTarget(1);
		//Clear(olc::CYAN);
		//EnableLayer(1, true);
		//FillRectDecal({ 40, 20 }, { 50, 50 }, olc::GREEN);
		//FillRect({ 40, 20 }, { 50, 50 }, olc::GREEN);

		//GradientFillRectDecal({ 20, 60 }, { 500, 800 }, olc::Pixel(rand() % 255, rand() % 255, rand() % 255), olc::Pixel(rand() % 255, rand() % 255, rand() % 255), olc::Pixel(rand() % 255, rand() % 255, rand() % 255), olc::Pixel(rand() % 255, rand() % 255, rand() % 255));

		//DrawStringDecal({ 50, 550 }, std::to_string(GetMouseWheel()));

		return true;
	}

	void ChangeLevelSize(int side, bool increase) // This is like terrible performance but I mean we dont see it, and not running every frame
	{
		cTile** temp = nullptr;

		switch (side)
		{
		case 0: // Right side
			if (increase)
			{
				temp = new cTile * [(nWidth + 1) * nHeight];

				for (int i = 0; i < (nWidth + 1) * nHeight; i++)
					temp[i] = nullptr;

				for (int y = 0; y < nHeight; y++)
				{
					for (int x = 0; x < nWidth; x++)
					{
						temp[y * (nWidth + 1) + x] = vecTiles[y * nWidth + x];
					}
					temp[y * (nWidth + 1) + (nWidth)] = new cTile("Blank", -1, 0, 0);
				}

				nWidth++;
			}
			else // Decrease size from this side
			{
				if (nWidth > 0)
				{
					temp = new cTile * [(nWidth - 1) * nHeight];

					for (int i = 0; i < (nWidth - 1) * nHeight; i++)
						temp[i] = nullptr;

					for (int y = 0; y < nHeight; y++)
					{
						for (int x = 0; x < nWidth - 1; x++)
						{
							temp[y * (nWidth - 1) + x] = vecTiles[y * nWidth + x];
						}
					}

					nWidth--;
				}
			}
			break;
		case 1: // Bottom side
			if (increase)
			{
				temp = new cTile * [nWidth * (nHeight + 1)];

				for (int i = 0; i < nWidth * (nHeight + 1); i++)
					temp[i] = nullptr;

				for (int x = 0; x < nWidth; x++)
				{
					for (int y = 0; y < nHeight; y++)
					{
						temp[y * nWidth + x] = vecTiles[y * nWidth + x];
					}
					temp[nHeight * nWidth + x] = new cTile("Blank", -1, 0, 0);
				}

				nHeight++;
			}
			else
			{
				if (nHeight > 0)
				{
					temp = new cTile * [nWidth * (nHeight - 1)];

					for (int i = 0; i < nWidth * (nHeight - 1); i++)
						temp[i] = nullptr;

					for (int x = 0; x < nWidth; x++)
					{
						for (int y = 0; y < nHeight - 1; y++)
						{
							temp[y * nWidth + x] = vecTiles[y * nWidth + x];
						}
					}

					nHeight--;
				}
			}
			break;
		case 2: // Left side
			if (increase)
			{
				temp = new cTile * [(nWidth + 1) * nHeight];

				for (int i = 0; i < (nWidth + 1) * nHeight; i++)
					temp[i] = nullptr;

				for (int y = 0; y < nHeight; y++)
				{
					for (int x = 0; x < nWidth; x++)
					{
						temp[y * (nWidth + 1) + (x + 1)] = vecTiles[y * nWidth + x]; // Move current tiles over to right 1
					}
					temp[y * (nWidth + 1) + 0] = new cTile("Blank", -1, 0, 0); // Set newly extended left side of level to blanks
				}

				nWidth++;
			}
			else
			{
				if (nWidth > 0)
				{
					temp = new cTile * [(nWidth - 1) * nHeight];

					for (int i = 0; i < (nWidth - 1) * nHeight; i++)
						temp[i] = nullptr;

					for (int y = 0; y < nHeight; y++)
					{
						for (int x = 0; x < nWidth - 1; x++)
						{
							temp[y * (nWidth - 1) + x] = vecTiles[y * nWidth + (x + 1)];
						}
					}

					nWidth--;
				}
			}
			break;
		case 3: // Top side
			if (increase)
			{
				temp = new cTile * [nWidth * (nHeight + 1)];

				for (int i = 0; i < nWidth * (nHeight + 1); i++)
					temp[i] = nullptr;

				for (int x = 0; x < nWidth; x++)
				{
					for (int y = 0; y < nHeight; y++)
					{
						temp[(y + 1) * nWidth + x] = vecTiles[y * nWidth + x];
					}
					temp[0 * nWidth + x] = new cTile("Blank", -1, 0, 0);
				}

				nHeight++;
			}
			else
			{
				if (nHeight > 0)
				{
					temp = new cTile * [nWidth * (nHeight - 1)];

					for (int i = 0; i < nWidth * (nHeight - 1); i++)
						temp[i] = nullptr;

					for (int x = 0; x < nWidth; x++)
					{
						for (int y = 0; y < nHeight - 1; y++)
						{
							temp[y * nWidth + x] = vecTiles[(y + 1) * nWidth + x];
						}
					}

					nHeight--;
				}
			}
			break;
		default:
			break;
		}


		delete[] vecTiles;
		vecTiles = temp;
		//delete[] temp; //No -> vecTiles is now equal to this memory address
	}

	void CharInput(std::string& s)
	{
		if (IsFocused())
		{
			if (GetKey(olc::SHIFT).bHeld)
			{
				if (GetKey(olc::A).bPressed)
					s += 'A';
				if (GetKey(olc::B).bPressed)
					s += 'B';
				if (GetKey(olc::C).bPressed)
					s += 'C';
				if (GetKey(olc::D).bPressed)
					s += 'D';
				if (GetKey(olc::E).bPressed)
					s += 'E';
				if (GetKey(olc::F).bPressed)
					s += 'F';
				if (GetKey(olc::G).bPressed)
					s += 'G';
				if (GetKey(olc::H).bPressed)
					s += 'H';
				if (GetKey(olc::I).bPressed)
					s += 'I';
				if (GetKey(olc::J).bPressed)
					s += 'J';
				if (GetKey(olc::K).bPressed)
					s += 'K';
				if (GetKey(olc::L).bPressed)
					s += 'L';
				if (GetKey(olc::M).bPressed)
					s += 'M';
				if (GetKey(olc::N).bPressed)
					s += 'N';
				if (GetKey(olc::O).bPressed)
					s += 'O';
				if (GetKey(olc::P).bPressed)
					s += 'P';
				if (GetKey(olc::Q).bPressed)
					s += 'Q';
				if (GetKey(olc::R).bPressed)
					s += 'R';
				if (GetKey(olc::S).bPressed)
					s += 'S';
				if (GetKey(olc::T).bPressed)
					s += 'T';
				if (GetKey(olc::U).bPressed)
					s += 'U';
				if (GetKey(olc::V).bPressed)
					s += 'V';
				if (GetKey(olc::W).bPressed)
					s += 'W';
				if (GetKey(olc::X).bPressed)
					s += 'X';
				if (GetKey(olc::Y).bPressed)
					s += 'Y';
				if (GetKey(olc::Z).bPressed)
					s += 'Z';
			}
			else
			{
				if (GetKey(olc::A).bPressed)
					s += 'a';
				if (GetKey(olc::B).bPressed)
					s += 'b';
				if (GetKey(olc::C).bPressed)
					s += 'c';
				if (GetKey(olc::D).bPressed)
					s += 'd';
				if (GetKey(olc::E).bPressed)
					s += 'e';
				if (GetKey(olc::F).bPressed)
					s += 'f';
				if (GetKey(olc::G).bPressed)
					s += 'g';
				if (GetKey(olc::H).bPressed)
					s += 'h';
				if (GetKey(olc::I).bPressed)
					s += 'i';
				if (GetKey(olc::J).bPressed)
					s += 'j';
				if (GetKey(olc::K).bPressed)
					s += 'k';
				if (GetKey(olc::L).bPressed)
					s += 'l';
				if (GetKey(olc::M).bPressed)
					s += 'm';
				if (GetKey(olc::N).bPressed)
					s += 'n';
				if (GetKey(olc::O).bPressed)
					s += 'o';
				if (GetKey(olc::P).bPressed)
					s += 'p';
				if (GetKey(olc::Q).bPressed)
					s += 'q';
				if (GetKey(olc::R).bPressed)
					s += 'r';
				if (GetKey(olc::S).bPressed)
					s += 's';
				if (GetKey(olc::T).bPressed)
					s += 't';
				if (GetKey(olc::U).bPressed)
					s += 'u';
				if (GetKey(olc::V).bPressed)
					s += 'v';
				if (GetKey(olc::W).bPressed)
					s += 'w';
				if (GetKey(olc::X).bPressed)
					s += 'x';
				if (GetKey(olc::Y).bPressed)
					s += 'y';
				if (GetKey(olc::Z).bPressed)
					s += 'z';
			}

			if (GetKey(olc::SPACE).bPressed)
				s += ' ';
			if (GetKey(olc::K0).bPressed)
				s += '0';
			if (GetKey(olc::K1).bPressed)
				s += '1';
			if (GetKey(olc::K2).bPressed)
				s += '2';
			if (GetKey(olc::K3).bPressed)
				s += '3';
			if (GetKey(olc::K4).bPressed)
				s += '4';
			if (GetKey(olc::K5).bPressed)
				s += '5';
			if (GetKey(olc::K6).bPressed)
				s += '6';
			if (GetKey(olc::K7).bPressed)
				s += '7';
			if (GetKey(olc::K8).bPressed)
				s += '8';
			if (GetKey(olc::K9).bPressed)
				s += '9';
			if (GetKey(olc::BACK).bPressed)
			{
				if (s.size() != 0)
					s.erase(s.end() - 1, s.end());
			}


		}
	}

};


int main()
{
	Editor demo;
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();
	

	return 0;
}