#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_GRAPHICS2D
#include "olcPGEX_Graphics2D.h"

#include "Tiles.h"
//#include "Animator.h"
#include "Assets.h"

#include <iostream>
#include <fstream>

constexpr auto LEVEL_PATH = "../Levels/";

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
		MODE_ERROR
	};

	int nMode = MODE_START;
	std::fstream myfile;


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

	// New Level Box Colors
	olc::Pixel p = olc::Pixel(olc::DARK_GREY);
	olc::Pixel pp = olc::Pixel(olc::WHITE);

	std::string s; // New Level Name
	/*=====================================*/

	/*===========Edit Screen===========*/
	std::vector<std::string> vecTileNames; // Acts as a map from ints to strings

	float fScalingFactor = 1.7f; // Adjust this to zoom

	olc::vi2d vGameScreenPos = { 25, 30 };

	int nGameScreenWidth = (int)(22 * fScalingFactor) * 12; // 444 - Hard code the level edit window to be the same aspect ratio as the game even when scaled (and in the editor we will be able to zoom in and out, but default zoom is the game's view)
	int nGameScreenHeight = (int)(22 * fScalingFactor) * 11; // 407

	cTile** vecTiles = nullptr;
	int nWidth, nHeight;

	float fCameraPosX = 5.0f;
	float fCameraPosY = 5.0f;

	// Tile selection in edit mode
	int nTileBoxPosX;
	int nTileBoxPosY;
	int nTileBoxWidth;
	int nTileBoxHeight;
	int nVisibleTileNames;

	int nTilePotentialSelect = 0; // For mouse over
	int nTileSelection = 0;
	int nTileSelectionOffset = 0;
	/*=================================*/

	/*===========Error Screen===========*/
	int nErrorCode = 0; // Default
	std::unique_ptr<olc::Decal> decSkull;
	/*=================================*/

public:
	bool OnUserCreate() override
	{
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

		decSkull = std::make_unique<olc::Decal>(Assets::get().GetSprite("Error"));
		

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
		case MODE_ERROR:
			return UpdateErrorScreen(fElapsedTime);
		}

		return true;
	}

	bool UpdateStartScreen(float fElapsedTime)
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
				if (LoadLevelData(nSelection))
					nMode = MODE_EDIT;
				else
					nMode = MODE_ERROR;
			}

			// Mouse over New Level Box
			if (GetMouseX() <= ScreenWidth() - 100 && GetMouseX() >= ScreenWidth() - 200
				&& GetMouseY() <= 170 && GetMouseY() >= 100)
			{
				p = olc::YELLOW;
				pp = olc::YELLOW;
			}
			else
			{
				p = olc::Pixel(olc::DARK_GREY);
				pp = olc::Pixel(olc::WHITE);
			}

			// Mouse click on new level box
			if (GetMouse(0).bReleased && GetMouseX() <= ScreenWidth() - 100 && GetMouseX() >= ScreenWidth() - 200
				&&
				GetMouseY() <= 170 && GetMouseY() >= 100)
			{
				//nSelection = vLevelNames.size();
				nMode = MODE_NEW;
			}
		}

		Clear(olc::DARK_GREEN);
		DrawString(olc::vi2d(5, 5), "Choose file to edit, or select \"New Level\"");

		// New Level
		FillRect(olc::vi2d(ScreenWidth() - 200, 100), olc::vi2d(100, 70), olc::CYAN);
		DrawRect(olc::vi2d(ScreenWidth() - 200, 100), olc::vi2d(100, 70), p);
		DrawRect(olc::vi2d(ScreenWidth() - 200 + 1, 100 + 1), olc::vi2d(100 - 2, 70 - 2), p);
		DrawString(olc::vi2d(ScreenWidth() - 187, 120), "New\nLevel", pp, 2);

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


		return true;
	}

	bool UpdateEditScreen(float fElapsedTime)
	{
		
		if (IsFocused())
		{
			if (GetKey(olc::S).bPressed)
			{
				//fCameraPosY - 5.0f * fElapsedTime;

				fScalingFactor -= 0.1f;
				if (fScalingFactor < 0.2f) fScalingFactor = 0.2f; // Prevent divide by 0
			}

			if (GetKey(olc::W).bPressed)
			{
				//fCameraPosY + 5.0f * fElapsedTime;

				fScalingFactor += 0.1f;

			}

			if (GetKey(olc::UP).bHeld)
			{
				fCameraPosY -= 2.0f * fElapsedTime;
			}

			if (GetKey(olc::DOWN).bHeld)
			{
				fCameraPosY += 2.0f * fElapsedTime;

			}
			if (GetKey(olc::LEFT).bHeld)
			{
				fCameraPosX -= 2.0f * fElapsedTime;
			}

			if (GetKey(olc::RIGHT).bHeld)
			{
				fCameraPosX += 2.0f * fElapsedTime;
			}

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
		}
		Clear(olc::VERY_DARK_BLUE);

		DrawString(olc::vi2d(50, 5), std::to_string(nSelection + 1) + ": " + vLevelNames[nSelection]);

		//fCameraPosX = GetMouseX();
		//fCameraPosY = GetMouseY();

		// Determine tile size with scaling factors for fixed window size	
		int nTileWidth = 22 * fScalingFactor; // Redefine width and height for varying zoom (fScalingFactor) levels
		int nTileHeight = 22 * fScalingFactor;
		int nVisibleTilesX = nGameScreenWidth / nTileWidth;
		int nVisibleTilesY = nGameScreenHeight / nTileHeight;

		// Level Window
		FillRect(vGameScreenPos, olc::vi2d((nGameScreenWidth + 1), (nGameScreenHeight + 1)), olc::CYAN);
		DrawRect(vGameScreenPos - olc::vi2d(1, 1), olc::vi2d((nGameScreenWidth) + 2, (nGameScreenHeight) + 2), olc::DARK_GREY);
		DrawRect(vGameScreenPos - olc::vi2d(2, 2), olc::vi2d((nGameScreenWidth) + 4, (nGameScreenHeight) + 4), olc::VERY_DARK_GREY);

		// ========================= Tile selection Window ====================================
		FillRect(olc::vi2d(nTileBoxPosX, nTileBoxPosY), olc::vi2d(nTileBoxWidth, nTileBoxHeight), olc::GREY);
		DrawRect(olc::vi2d(nTileBoxPosX, nTileBoxPosY), olc::vi2d(nTileBoxWidth, nTileBoxHeight), olc::DARK_GREY);
		DrawRect(olc::vi2d(nTileBoxPosX + 1, nTileBoxPosY + 1), olc::vi2d(nTileBoxWidth - 2, nTileBoxHeight - 2), olc::DARK_GREY);

		nTileSelectionOffset = (nTileSelection + 1) - nVisibleTileNames; // Determine which level names to show
		if (nTileSelectionOffset < 0) nTileSelectionOffset = 0;

		if (nTilePotentialSelect != -1)
			FillRect(olc::vi2d(nTileBoxPosX + 3, ((nTilePotentialSelect - nTileSelectionOffset) * 30 + 13 - 10) + nTileBoxPosY), olc::vi2d(nTileBoxWidth - 5, 28), olc::DARK_YELLOW); // Mouse over Selction Box
		FillRect(olc::vi2d(nTileBoxPosX + 3, ((nTileSelection - nTileSelectionOffset) * 30 + 13 - 10) + nTileBoxPosY), olc::vi2d(nTileBoxWidth - 5, 28), olc::DARK_RED); // Selction Box
		for (int i = 0; i < nVisibleTileNames && i < vecTileNames.size(); i++)
		{
			// Sprite before Name
			//SetPixelMode(olc::Pixel::MASK);
			//DrawSprite(olc::vi2d(nTileBoxPosX + 6, (i * 30 + 13/2) + nTileBoxPosY), Assets::get().GetSprite(vecTileNames[i + nTileSelectionOffset]));
			//(olc::Pixel::NORMAL);
			//DrawString(olc::vi2d(nTileBoxPosX + nTileBoxWidth - (vecTileNames[i + nTileSelectionOffset].length() * 8) - 8, (i * 30 + 13) + nTileBoxPosY), vecTileNames[i + nTileSelectionOffset]);

			// Name before Sprite
			SetPixelMode(olc::Pixel::MASK);
			DrawSprite(olc::vi2d(nTileBoxPosX + 6 + (vecTileNames[i + nTileSelectionOffset].length() * 8) + 8, (i * 30 + 13 / 2) + nTileBoxPosY), Assets::get().GetSprite(vecTileNames[i + nTileSelectionOffset]));
			SetPixelMode(olc::Pixel::NORMAL);
			DrawString(olc::vi2d(nTileBoxPosX + 6, (i * 30 + 13) + nTileBoxPosY), vecTileNames[i + nTileSelectionOffset]);
		}
		// =======================================================================================

		

		//Calculate Top-Leftmost visible tile
		float fOffsetX = fCameraPosX - (float)nVisibleTilesX / 2.0f;
		float fOffsetY = fCameraPosY - (float)nVisibleTilesY / 2.0f;

		//Clamp camera to game boundaries
		if (nWidth < nVisibleTilesX)
		{

		}
		if (nHeight < nVisibleTilesY)
		{

		}
		if (nWidth >= nVisibleTilesX)
		{

		}
		if (nHeight >= nVisibleTilesY)
		{

		}
		//if (fOffsetX < 0) fOffsetX = 0;
		//if (fOffsetY < 0) fOffsetY = 0;
		//if (fOffsetX > nWidth - nVisibleTilesX && nWidth > nVisibleTilesX) fOffsetX = nWidth - nVisibleTilesX; // Need "nWidth > nVisibleTilesX" because if for some reason the level is really small, this will set fOffset to negative
		//if (fOffsetY > nHeight - nVisibleTilesY && nHeight > nVisibleTilesY) fOffsetY = nHeight - nVisibleTilesY;
		//^^^ Need more catches here because Visible tiles can change with scaling factor

		if (fOffsetX > nWidth - nVisibleTilesX) fOffsetX = nWidth - nVisibleTilesX;
		if (fOffsetY > nHeight - nVisibleTilesY) fOffsetY = nHeight - nVisibleTilesY;
		if (fOffsetX < 0) fOffsetX = 0;
		if (fOffsetY < 0) fOffsetY = 0;

		// Clamp actual camera to avoid camera not responding instantly
		/*if (fCameraPosX < nVisibleTilesX / 2 * nTileWidth) fCameraPosX = nVisibleTilesX * nTileWidth;
		if (fCameraPosY < nVisibleTilesY / 2 * nTileHeight) fCameraPosY = nVisibleTilesY * nTileHeight;
		if (fCameraPosX > nWidth - nVisibleTilesX / 2) fCameraPosX = nWidth - nVisibleTilesX / 2;
		if (fCameraPosY > nHeight - nVisibleTilesY / 2) fCameraPosY = nHeight - nVisibleTilesY / 2;*/


		// Get offsets for smooth movement
		float fTileOffsetX = (fOffsetX - (int)fOffsetX) * nTileWidth; // Note: this is already scaled by the fScalingFactor (in nTileWidth / nTileHeight)
		float fTileOffsetY = (fOffsetY - (int)fOffsetY) * nTileHeight;

		//Draw visible tile map (overdraw to prevent weird artifacts at screen edges)
		for (int x = 0; x < nVisibleTilesX + 1; x++)
		{
			for (int y = 0; y < nVisibleTilesY + 1 && y < nHeight; y++)
			{
				if (x + fOffsetX < nWidth && y + fOffsetY < nHeight) // Because we're overdrawing, we want to prevent accessing a tile outside of range
				{
					//olc::Sprite sp(GetTile(x + fOffsetX, y + fOffsetY)->sprite->width * fScalingFactor, GetTile(x + fOffsetX, y + fOffsetY)->sprite->height* fScalingFactor);
					//FillRect(x * nTileWidth - fTileOffsetX + vGameScreenPos.x, y * nTileHeight - fTileOffsetY + vGameScreenPos.y, nTileWidth, nTileHeight, olc::BLUE);
					//DrawRect(x* nTileWidth - fTileOffsetX + vGameScreenPos.x, y* nTileHeight - fTileOffsetY + vGameScreenPos.y, nTileWidth - 1, nTileHeight - 1, olc::Pixel(rand() % 256, rand() % 256, rand() % 256));
					//GetTile(x + fOffsetX, y + fOffsetY)->DrawSelf(this, x * nTileWidth - fTileOffsetX + vGameScreenPos.x, y * nTileHeight - fTileOffsetY + vGameScreenPos.y);
					//olc::GFX2D::Transform2D t;
					//t.Scale(fScalingFactor, fScalingFactor);
					//t.Translate(x* nTileWidth - fTileOffsetX + vGameScreenPos.x, y* nTileHeight - fTileOffsetY + vGameScreenPos.y);
					//SetDrawTarget(&sp);
					//olc::GFX2D::DrawSprite(GetTile(x + fOffsetX, y + fOffsetY)->sprite, t);
					//SetDrawTarget(nullptr);

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

					if (x == nVisibleTilesX || x == nWidth)
						DrawPartialDecal(olc::vi2d(x * nTileWidth + vGameScreenPos.x, y * nTileHeight - fTileOffsetY + vGameScreenPos.y), GetTile(x + fOffsetX, y + fOffsetY)->decal, olc::vi2d(0, 0), olc::vi2d(GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->width, GetTile(x + fOffsetX, y + fOffsetY)->decal->sprite->height) - olc::vi2d(fTileOffsetX / fScalingFactor, 0), olc::vf2d(fScalingFactor, fScalingFactor));


					//GetTile(x, y)->DrawSelf(this, x * nTileWidth - fTileOffsetX + vGameScreenPos.x, y * nTileHeight - fTileOffsetY + vGameScreenPos.y);
				}
			}
		}

		//TEST
		/*for (int i = 0; i < nWidth * nHeight; i++)
		{
			vecTiles[i]->DrawSelf(this, i * 30, 30);
		}*/
		

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
			return new cTile("Not_Found", true, false); // I will rationalize doing this, which normally causes a memory leak, because we are just returning SOMETHING only to get to the error screen
			//return nullptr; // Makes it crash if something goes wrong
			// NO -> this makes a memory leak  return new cTile("Not_Found", true, false); // Use the error tile sprite to show an obvious problem if the index is not within range
		}
	}

	void CreateKeyMap()
	{
		std::ifstream inFile("Tiles.key", std::ios::in | std::ios::binary);
		if (inFile.is_open())
		{
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
		std::ifstream inFile(LEVEL_PATH + std::to_string(selection + 1) + ".lvl", std::ios::in | std::ios::binary);
		if (inFile.is_open())
		{
			std::string s; // Already have name, don't do anything with this
			std::getline(myfile, s);

			inFile >> nWidth >> nHeight;
			if (nWidth == 0 || nHeight == 0) // Default for these values is 0, so if not changed there is no level
			{
				std::cerr << "Ensure Level exists, and that Level Width and Level Height are respectively defined in the first 2 lines after the level name." << std::endl;
				nErrorCode = 2;
				return false;
			}

			// File should be good, read
			vecTiles = new cTile * [nWidth * nHeight]; // Create array

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
					if (nTileType >= 0 && nTileType < vecTileNames.size())
						vecTiles[y * nWidth + x] = new cTile(vecTileNames[nTileType], nTileSolid, nTileBreakable);
					else
					{
						std::cerr << "Tile provided at (" << x << ", " << y << ") is not within the range of tiles provided in Tiles.key" << std::endl;
						vecTiles[y * nWidth + x] = new cTile("Not_Found", true, false); // Will create a tile with a "tile not found" sprite, or something similar as defined in Assets class, to be obvious to the developer/designer that something is wrong
					}
				}
			}
		}
		else
		{
			std::cerr << "Could not open " << LEVEL_PATH + std::to_string(selection + 1) + ".lvl" << std::endl;
			nErrorCode = 1;
			return false;
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
		// Enter Level Name
		Clear(olc::DARK_BLUE);
		DrawString(olc::vi2d((ScreenWidth() / 2) - (17 * 8), 30), "Enter Level Name:", olc::WHITE, 2);
		FillRect(olc::vi2d(ScreenWidth() / 5, 100), olc::vi2d(400, 50), olc::GREY);
		DrawRect(olc::vi2d(ScreenWidth() / 5, 100), olc::vi2d(400, 50), olc::DARK_GREY);
		DrawRect(olc::vi2d(ScreenWidth() / 5 + 1, 100 + 1), olc::vi2d(400 - 2, 50 - 2), olc::DARK_GREY);

		CharInput(s);
		DrawString(olc::vi2d(ScreenWidth() / 5 + 5, 115), s, olc::WHITE, 2);

		if (bReady)
		{
			myfile.open(LEVEL_PATH + std::to_string(vLevelNames.size() + 1) + ".lvl", std::ios::out); // Open file to create it and save level name
			if (myfile.is_open())
			{
				std::cout << LEVEL_PATH + std::to_string(vLevelNames.size() + 1) + ".lvl" + " - File Successfully Created." << std::endl;
				myfile << s;
			}
			else
				std::cout << "Unable to create file: " << LEVEL_PATH + std::to_string(vLevelNames.size() + 1) + ".lvl" << std::endl;

			myfile.close();

			nSelection = vLevelNames.size();
			vLevelNames.push_back(s);
			nMode = MODE_EDIT;
		}

		return true;
	}

	bool UpdateErrorScreen(float fElapsedTime)
	{
		Clear(olc::VERY_DARK_RED);
		DrawString(olc::vi2d(5, 10), "OH NO, SOMETHING BAD HAPPENED...", olc::DARK_GREY, 2);

		SetPixelMode(olc::Pixel::MASK);
		DrawDecal(olc::vi2d(ScreenWidth() / 2 - 47, 30), decSkull.get(), { 0.15f, 0.15f });
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
			DrawString(olc::vi2d(5, 170), "File opened to read level data,\nbut something went wrong reading it.", olc::DARK_GREY, 2);
			break;
		case 3:
			DrawString(olc::vi2d(5, 170), "Tried indexing into an illegal spot\nin vecTiles.\n\nEnsure things that modify\nthe index (like fOffset) are correct.", olc::DARK_GREY, 2);
			break;
		}
		return true;
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
	/*
	//This all happens before the editor launches - before Editor class is instantiated
	//Confirms that level file exits, and prompts user to create it now if not
	bool launch = false;
	std::cout << "Attempting to read \"level.lvl\"" << std::endl;
	std::fstream myfile;
	myfile.open("level.lvl");
	if (myfile.is_open())
	{
		std::cout << "Successfully opened \"level.lvl\"" << std::endl;
		launch = true;
	}
	else
	{
		char ans;
		std::cout << "File not found - Would you like to create it now?" << std::endl
			<< "Type y or n (Yes/No): ";
		std::cin >> ans;
		if (ans == 'y' || ans == 'Y')
		{
			std::cout << "Creating file \"level.lvl\"..." << std::endl;
			myfile.open("level.lvl", std::ios::out);
			if (!myfile.is_open())
				std::cout << "File Successfully Created." << std::endl;
			myfile.close();

			launch = true;
		}
		else if (ans == 'n' || ans == 'N')
		{
			std::cout << "File not created." << std::endl;
			system("pause");
		}
		else
		{
			std::cout << "Response not understood. File not created." << std::endl;
			system("pause");
		}

	}
	*/

	//if (launch)
	//{
	Editor demo;
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();
	//}
	

	return 0;
}