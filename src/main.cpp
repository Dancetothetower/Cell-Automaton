#include "raylib.h"
//#include "resource_dir.h"	
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <cstdint>
#include <bitset>
#include <random>
#include <algorithm>
struct buttonBox {
	Rectangle box;
	bool curState;
};
struct dialogeBox {
	Rectangle box;
	const char* name;
};
const Color HALFSHADE = { 0,0,0,127 };
uint16_t winHeight = 800;
uint16_t winHeightButton = 100;
uint16_t winHeightText = 50;
uint16_t winHeightTrue = winHeight + winHeightButton + winHeightText;
uint16_t winWidth = 1600;
const uint16_t animHeight = 400;
const uint16_t animWidth = 799;
std::bitset<animWidth> stateBuffer[animHeight];
uint8_t colorBuffer[animHeight][animWidth];
uint8_t sineTable[765];
std::mt19937 rng(std::random_device{}());
uint8_t ruleSelect = 30;
const char* stateName[2] = { "0", "1" };
const char* ruleNamePreset = "Rule %d";
buttonBox buttons[8];
buttonBox ranButton[4];
dialogeBox ruleBox = { {0, winHeight, winWidth, winHeightText}, "Rule 30" };

void initButtons()
{
	for (uint8_t i = 0; i < 8; i++)
	{
		buttons[i].box.x = winWidth - ((i+1) * winWidth >> 3);
		buttons[i].box.y = winHeight + winHeightText;
		buttons[i].box.width = winWidth >> 3;
		buttons[i].box.height = winHeightButton;
		buttons[i].curState = false;
	}
	for (uint8_t i = 0; i < 4; i++)
	{
		ranButton[i].box.x = winWidth - ((i + 1) * winWidth >> 3);
		ranButton[i].box.y = winHeight;
		ranButton[i].box.width = winWidth >> 3;
		ranButton[i].box.height = winHeightText;
		ranButton[i].curState = false;
	}
}
void initStateBuffer()
{
	for (uint16_t y = 0; y < animHeight; y++)
	{
		for (uint16_t x = 0; x < animWidth; x++)
		{
			colorBuffer[y][x] = 0;
			stateBuffer[y][x] = false;
			//if (y == 0 && x % 2 == 0) stateBuffer[y][x] = true;
		}
	}
	float piOffset = 2. * PI / 765.;
	for (uint16_t i = 0; i < 765; i++)
	{
		sineTable[i] = (uint8_t)(sin((float)i * piOffset) * 127.0f + 127.0f);
	}
	stateBuffer[0][(animWidth - 1 ) >> 1] = true;
}
Color getColor(uint16_t input)
{
	uint8_t offset = 255;
	uint8_t r = std::max(sineTable[(input * 3 )             % 765], uint8_t{ 0 });
	uint8_t g = std::max(sineTable[(input * 3 + offset)     % 765], uint8_t{ 0 });
	uint8_t b = std::max(sineTable[(input * 3 + offset * 2) % 765], uint8_t{ 0 });
	if(ranButton[3].curState){
		r = (r >> 1) + 127;
		g = (g >> 1) + 127;
		b = (b >> 1) + 127;
	}
	Color color = {r, g, b, 255 };
	return color;
}
void updateStateBuffer()
{
	uint8_t index = 0;
	bool left = false;
	bool center = false;
	bool right = false;
	for (uint16_t y = 0; y < animHeight - 1; y++)
	{
		for (uint16_t x = 0; x < animWidth; x++)
		{
			left = (x == 0) ? false : stateBuffer[y][(x - 1)];
			center = stateBuffer[y][x];
			right = (x == animWidth - 1) ? false : stateBuffer[y][(x + 1)];
			index = (left << 2) | (center << 1) | right;
			stateBuffer[y + 1][x] = (ruleSelect >> index) & 1;
			colorBuffer[y + 1][x] = index << 5;
		}
	}
}
bool genRandomBuffer(bool clearBuffer = false)
{
	std::uniform_int_distribution<uint16_t> dist(0, 1);
	for (uint16_t x = 0; x < animWidth; x++)
	{
			stateBuffer[0][x] = (clearBuffer)? 0: dist(rng);
	}
	if (clearBuffer) stateBuffer[0][animWidth >> 1] = true;
	return true;
}
//draws the current state of the cellular automaton to the screen
// each cell is drawn as a rectangle with a color based on its state (white for 1, black for 0)
void drawBackground()
{
	for (uint16_t y = 0; y < animHeight; y++)
	{
		for (uint16_t x = 0; x < animWidth; x++)
		{
			Color cellColor = getColor(colorBuffer[y][x] + y * ranButton[3].curState);
			cellColor =  stateBuffer[y][x] ? (ranButton[2].curState ? cellColor : WHITE) : BLACK;
			DrawRectangle(x * (winWidth / animWidth) + 1,
				y * (winHeight / animHeight),
				winWidth / animWidth,
				winHeight / animHeight,
				cellColor);
		}
	}
}
// Draws the buttons and updates the ruleSelect variable based on their states
// Returns true if any button was pressed, false otherwise
// Encodes the state of the buttons into a single byte, where each bit represents the state of a button (0 for off, 1 for on)
bool checkButtons()
{
	Rectangle offsetBox = { 10, 10, 0, 0 };
	//Rectangle tempBox = { 0, -50, 0, 0 };
	uint8_t rule = 0;
	bool work = false;
	for (uint8_t i = 0; i < 8; i++) {
        GuiToggle(buttons[i].box, stateName[buttons[i].curState], &buttons[i].curState);
		rule |= (buttons[i].curState << i);
	}
	if (rule != ruleSelect) work = true;
	ruleSelect = rule;
	char buffer[16];
	snprintf(buffer, sizeof(buffer), ruleNamePreset, ruleSelect);
	ruleBox.name = buffer;
	DrawText(ruleBox.name, ruleBox.box.x, ruleBox.box.y, 50, WHITE);
	if(GuiButton(ranButton[0].box, "")) work = genRandomBuffer(true);
	if(GuiButton(ranButton[1].box, "")) work = genRandomBuffer(false);
	GuiToggle(ranButton[2].box, "", &ranButton[2].curState);
	GuiToggle(ranButton[3].box, "", &ranButton[3].curState);
	DrawText("Clear", ranButton[0].box.x + offsetBox.x, ranButton[0].box.y + offsetBox.y, 30, HALFSHADE);
	DrawText("Random", ranButton[1].box.x + offsetBox.x, ranButton[1].box.y + offsetBox.y, 30, HALFSHADE);
	DrawText("Color", ranButton[2].box.x + offsetBox.x, ranButton[2].box.y + offsetBox.y, 30, HALFSHADE);
	DrawText("Gradient", ranButton[3].box.x + offsetBox.x, ranButton[3].box.y + offsetBox.y, 30, HALFSHADE);
	return work;
}

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	InitWindow(winWidth, winHeightTrue, "Cellular Automaton");
	//SearchAndSetResourceDir("resources");
	initButtons();
	initStateBuffer();
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		BeginDrawing();
		ClearBackground(BLACK);
		if (checkButtons())
		{
			updateStateBuffer();
		}
		drawBackground();
		EndDrawing();
	}
	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
