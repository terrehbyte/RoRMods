#include "D3DHook.h"
//#include "dhFastFont9.h"

void D3DHook::render()
{
	
	// clear the window alpha
	m_d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_d3ddev->BeginScene();    // begins the 3D scene

	// select which vertex format we are using
	m_d3ddev->SetFVF(CUSTOMFVF);

	// select the vertex buffer to display
	m_d3ddev->SetStreamSource(0, m_vbuffer, 0, sizeof(CUSTOMVERTEX));
	// copy the vertex buffer to the back buffer
	

	if (true) //draw howto if...
		DrawTextString(m_width / 2 - 100, m_height / 15, 100, 200, D3DCOLOR_ARGB(255, 255, 255, 255), "F1 to host | F2 to connect", m_pFontDefault, DT_CENTER);
		
	refreshLife(); //Refresh lenght of lifebar according to current health

	if (m_err_life > 0) //Draw error if counter is still relevent
	{
		error();
		m_err_life--;
	}

	if (m_info_life > 0) //Draw info if counter is still relevent
	{
		info();
		m_info_life--;
	}

	if (m_mlife > 0) //Draw HUD only if we're in game and not paused
	{
		m_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_vertices.size() / 3);
		textHud();
	}

	m_d3ddev->EndScene();    // ends the 3D scene

	m_d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}

void D3DHook::initD3D(HWND hWnd)
{
	m_d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;     // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = m_width;    // set the width of the buffer
	d3dpp.BackBufferHeight = m_height;    // set the height of the buffer

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// create a device class using this information and the info from the d3dpp stuct
	m_d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&m_d3ddev);

	vHUD();    // call the function to initialize the lifebar
	
	initFont(); //init font	
	setlmlife();

	//ipBox("");
}

void D3DHook::initFont()
{
	AddFontResourceEx("Resources/RiskofRainSquare.ttf", FR_PRIVATE, 0); //add font to available resources
	AddFontResourceEx("Resources/RiskofRainFont.ttf", FR_PRIVATE, 0); //add font to available resources

	D3DXCreateFont(m_d3ddev, 18, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "RiskofRainSquare", &m_pFont);
	D3DXCreateFont(m_d3ddev, 13, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "RiskofRainSquare", &m_pFontSmall);
	D3DXCreateFont(m_d3ddev, 22, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "RiskofRainFont", &m_pFontStat);

	D3DXCreateFont(m_d3ddev, 13, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_pFontDefaultSmall);
	D3DXCreateFont(m_d3ddev, 17, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_pFontDefault);
}

void D3DHook::vHUD()
{	

	float yoff = WIDTH + 15.0f;

	switch (m_scale) //set rect. pos. depending to scale
	{
		default: //TODO: set pos for each scale

			for (int i=0; i < m_stats.players.size(); i++) //add as many lifebars outlines & backgrounds as needed
			{
				addRect(7.0f, i * yoff + 100.0f, LENGHT, WIDTH, D3DCOLOR_ARGB(255, 41, 43, 60)); //EXTERNAL OUTLINE
				addRect(7.0f + WIDTH / 14.0f, i * yoff + 100.5f + WIDTH / 14.0f, LENGHT - 2 * WIDTH / 14.0f, WIDTH - 2 * WIDTH / 14, D3DCOLOR_ARGB(255, 26, 26 , 26)); //HEALTH BACKGROUND
			}

			for (int i=1; i <= m_stats.players.size(); i++) //add as many lifebars outlines as needed
				addLifeRect(7.0f + WIDTH / 14.0f, i * yoff + 100.0f + WIDTH / 14.0f, WIDTH - 2.0f * WIDTH / 14.0f, D3DCOLOR_ARGB(255, 136, 211, 103), i - 1); //HEALTH

			break;
	}



	// create a vertex buffer interface called m_vbuffer
	m_d3ddev->CreateVertexBuffer(m_vertices.size() * sizeof(CUSTOMVERTEX), NULL, CUSTOMFVF, D3DPOOL_MANAGED, &m_vbuffer, NULL);

	VOID* pVoid;	// a void pointer

	CUSTOMVERTEX array[100];
	std::copy(m_vertices.begin(), m_vertices.end(), array);	//creating array with content of m_vertices to ease cast in memcpy()

	// lock m_vbuffer and load the vertices into it
	m_vbuffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, array, 20*m_vertices.size());
	m_vbuffer->Unlock();
}

void D3DHook::addRect(float x, float y, float l, float w, D3DCOLOR color)
{
	m_vertices.push_back({ x, y, 0.0f, 0.0f, color });	// top left
	m_vertices.push_back({ x + l, y, 0.0f, 0.0f, color });	//top right
	m_vertices.push_back({ x, y + w, 0.0f, 0.0f, color });	//bottom right

	m_vertices.push_back({ x + l, y, 0.0f, 0.0f, color });	// top right
	m_vertices.push_back({ x + l, y + w, 0.0f, 0.0f, color }); //bottom right
	m_vertices.push_back({ x, y + w, 0.0f, 0.0f, color });	//bottom left
}

void D3DHook::addLifeRect(float x, float y, float w, D3DCOLOR color, int player)
{
	m_vertices.push_back({ x, y, 0.0f, 0.0f, color });	//top left
	m_vertices.push_back({ x + m_llives[player], y, 0.0f, 0.0f, color });	//top right
	m_vertices.push_back({ x + m_llives[player], y + w, 0.0f, 0.0f, color });	//bottom right

	m_vertices.push_back({ x + m_llives[player], y + w, 0.0f, 0.0f, color });	//bottom right
	m_vertices.push_back({ x, y + w, 0.0f, 0.0f, color }); //bottom left
	m_vertices.push_back({ x, y, 0.0f, 0.0f, color }); //top left
}

void D3DHook::refreshLife()
{
	int yoff = WIDTH + 15.0f; //y-axis offset

	m_llife = m_life * m_lmlife / m_mlife; //calc lenght of lifebar


	for (int i=0; i < m_stats.players.size(); i++) //calc all lenghts of player's lifebars
		m_llives[i] = m_stats.players[i].health * m_lmlife / m_stats.players[i].maxHealth;

	for (int i = 0; i < 6*m_stats.players.size(); i++) //remove 6 vertices for each players (=> remove all lifebars)
		m_vertices.pop_back();

	//TODO: dependence on m_scale
	for (int i=0; i < m_stats.players.size(); i++) //add as many lifebars outlines as needed
		addLifeRect(7.0f + WIDTH / 14.0f, i * yoff + 100.0f + WIDTH / 14.0f, WIDTH - 2.0f * WIDTH / 14.0f, D3DCOLOR_ARGB(255, 136, 211, 103), i - 1); //HEALTH



	// create a vertex buffer interface called m_vbuffer
	m_d3ddev->CreateVertexBuffer(m_vertices.size() * sizeof(CUSTOMVERTEX), NULL, CUSTOMFVF, D3DPOOL_MANAGED, &m_vbuffer, NULL);

	VOID* pVoid;    // a void pointer

	CUSTOMVERTEX array[100];
	std::copy(m_vertices.begin(), m_vertices.end(), array);
	// lock m_vbuffer and load the vertices into it
	m_vbuffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, array, 20 * m_vertices.size());
	m_vbuffer->Unlock();
}

void D3DHook::textHud()
{
	int lifeyoff = WIDTH +15.0f;

	RECT container = { 0, 0, 0, 0 }; //container for outline

	std::ostringstream life[4], lvl[4], item;

	for (int i = 0; i < m_stats.players.size(); i++) //create array containing life strings to draw
	{
		life[i] << std::fixed << std::setprecision(0) << m_stats.players[i].health << "/" << m_stats.players[i].maxHealth;
		lvl[i] << "LV. " << std::fixed << std::setprecision(0) << m_stats.players[i].level;
	}
	item << std::fixed << m_item << " ITEMS";

	//LIFE & OUTLINE
	for (int i = 0; i < m_stats.players.size(); i++) //draw life and level of each existing player
	{
		DrawOutline(7.0f, 100.0f + i * lifeyoff + WIDTH / 14, WIDTH, LENGHT, D3DCOLOR_ARGB(255, 64, 64, 64), life[i].str().c_str(), m_pFont, DT_CENTER, container);
		DrawTextString(7.0f, 100.0f + i * lifeyoff + WIDTH / 14, WIDTH, LENGHT, D3DCOLOR_ARGB(255, 255, 255, 255), life[i].str().c_str(), m_pFont, DT_CENTER);

		DrawOutline(8, 100 + i * lifeyoff + WIDTH, WIDTH, LENGHT, D3DCOLOR_ARGB(255, 26, 26, 26), lvl[i].str().c_str(), m_pFontSmall, DT_LEFT, container);
		DrawTextString(8, 100 + i * lifeyoff + WIDTH, WIDTH, LENGHT, D3DCOLOR_ARGB(255, 255, 255, 255), lvl[i].str().c_str(), m_pFontSmall, DT_LEFT);
	}

	//ITEMS & OUTLINE
	DrawOutline(39 * m_width / 100, 90.3*m_height / 100, WIDTH, 2 * LENGHT, D3DCOLOR_ARGB(255, 26, 26, 26), item.str().c_str(), m_pFontSmall, DT_LEFT, container);
	DrawTextString(39 * m_width / 100, 90.3*m_height / 100, WIDTH, 2 * LENGHT, D3DCOLOR_ARGB(255, 192, 192, 192), item.str().c_str(), m_pFontSmall, DT_LEFT);

	int yoff = 1; //index of stat
	float height = 18; //space between two stats

	std::ostringstream dmg, rate, crit, regen, strength;

	dmg << "DMG:  " << std::setprecision(2) << m_stats.players[m_pSel].dmg;
	rate << "FIRERATE:  " << std::setprecision(2) << m_stats.players[m_pSel].rate;
	crit << "CRIT:  " << std::setprecision(2) << m_stats.players[m_pSel].crit;
	regen << "REGEN:  " << std::setprecision(2) << m_stats.players[m_pSel].regen;
	strength << "STRENGTH:  " << std::setprecision(2) << m_stats.players[m_pSel].strength;

	DrawTextString(75.75 * m_width/100, 14.0f*m_height / 100  + yoff * height, height, m_width/5, D3DCOLOR_ARGB(255, 192, 192, 192), dmg.str().c_str(),  m_pFontStat, DT_RIGHT);
	yoff++; //index++
	DrawTextString(75.75 * m_width / 100, 14.0f*m_height / 100 + yoff * height, height, m_width / 5, D3DCOLOR_ARGB(255, 192, 192, 192), rate.str().c_str(), m_pFontStat, DT_RIGHT);
	yoff++;
	DrawTextString(75.75 * m_width / 100, 14.0f*m_height / 100 + yoff * height, height, m_width / 5, D3DCOLOR_ARGB(255, 192, 192, 192), crit.str().c_str(), m_pFontStat, DT_RIGHT);
	yoff++;
	DrawTextString(75.75 * m_width / 100, 14.0f*m_height / 100 + yoff * height, height, m_width / 5, D3DCOLOR_ARGB(255, 192, 192, 192), regen.str().c_str(), m_pFontStat, DT_RIGHT);
	yoff++;
	DrawTextString(75.75 * m_width / 100, 14.0f*m_height / 100 + yoff * height, height, m_width / 5, D3DCOLOR_ARGB(255, 192, 192, 192), strength.str().c_str(), m_pFontStat, DT_RIGHT);
}

void D3DHook::DrawTextString(int x, int y, int h, int w, DWORD color, const char *str, LPD3DXFONT pfont, int align)
{
	// set container of text
	RECT container = { x, y, x + w, y + h };
	// Output the text, center aligned
	pfont->DrawText(NULL, str, -1, &container, align, color);
}

void D3DHook::DrawOutline(int x, int y, int h, int w, DWORD color, const char *str, LPD3DXFONT pfont, int align, RECT container)
{	
	container = { x - 2, y, x + w, y + h }; // set container of text
	pfont->DrawText(NULL, str, -1, &container, align, color); //left outline

	container = { x + 2, y, x + w, y + h };	// set container of text
	pfont->DrawText(NULL, str, -1, &container, align, color);	//right outline

	container = { x, y - 2, x + w, y + h };	// set container of text
	pfont->DrawText(NULL, str, -1, &container, align, color);	//top outline

	container = { x, y + 2, x + w, y + h };	// set container of text
	pfont->DrawText(NULL, str, -1, &container, align, color); //bottom outline
}

void D3DHook::error()
{
	RECT Rect = { 10, m_height - 20,0,0 }; //set container pos

	m_pFontDefault->DrawText(NULL, m_error, -1, &Rect, DT_CALCRECT, 0); //calc container size based on text
	m_pFontDefault->DrawText(NULL, m_error, -1, &Rect, DT_LEFT, D3DCOLOR_ARGB(255, 255, 0, 0)); //draw text
}

void D3DHook::info()
{
	RECT Rect = { 10, m_height - 20,0,0 }; //set container pos

	m_pFontDefault->DrawText(NULL, m_info, -1, &Rect, DT_CALCRECT, 0);	//calc container size based on text
	m_pFontDefault->DrawText(NULL, m_info, -1, &Rect, DT_LEFT, D3DCOLOR_ARGB(255, 0, 255, 255));	//draw text
}

/*
void D3DHook::drawString(int x, int y, DWORD color, LPD3DXFONT g_pFont, const char * fmt)
{
	RECT FontPos = { x, y, x + 120, y + 16 };
	char buf[1024] = { '\0' };
	va_list va_alist;

	va_start(va_alist, fmt);
	vsprintf_s(buf, fmt, va_alist);
	va_end(va_alist);
	g_pFont->DrawText(NULL, buf, -1, &FontPos, DT_NOCLIP, color);
}
*/

/*
CUSTOMVERTEX vertices[] =
{
	//FIRST OUTLINE
	{ 10.0f, 100.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f + LENGHT, 100.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f, 100.0f + WIDTH, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f + LENGHT, 100.0f + WIDTH, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },

	{ 10.0f, 100.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f + LENGHT, 100.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f, 100.0f + WIDTH, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f + LENGHT, 100.0f + WIDTH, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },

	{ 10.0f, 100.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f + LENGHT, 100.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f, 100.0f + WIDTH, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f + LENGHT, 100.0f + WIDTH, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },

	{ 10.0f + LENGHT, 100.0f , 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f + LENGHT, 100.0f + WIDTH, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },
	{ 10.0f, 100.0f + WIDTH, 0.0f, 0.0f, D3DCOLOR_XRGB(51, 43, 60) },

	//SECOND OUTLINE
	{ 10.0f + WIDTH / 16 , 100.0f + WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(64, 65, 87) },
	{ 10.0f + LENGHT - WIDTH / 16, 100.0f + WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(64, 65, 87) },
	{ 10.0f + WIDTH / 16, 100.0f + WIDTH - WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(64, 65, 87) },

	{ 10.0f + LENGHT - WIDTH / 16 , 100.0f + WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(64, 65, 87) },
	{ 10.0f + LENGHT - WIDTH / 16, 100.0f + WIDTH - WIDTH / 16 , 0.0f, 0.0f, D3DCOLOR_XRGB(64, 65, 87) },
	{ 10.0f + WIDTH / 16, 100.0f + WIDTH - WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(64, 65, 87) },

	//BACKGROUND (NO-LIFE)
	{ 10.0f + 2 * WIDTH / 16 , 100.0f + 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(26, 26, 26) },
	{ 10.0f + LENGHT - 2 * WIDTH / 16, 100.0f + 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(26, 26, 26) },
	{ 10.0f + 2 * WIDTH / 16, 100.0f + WIDTH - 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(26, 26, 26) },

	{ 10.0f + LENGHT - 2 * WIDTH / 16 , 100.0f + 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(26, 26, 26) },
	{ 10.0f + LENGHT - 2 * WIDTH / 16, 100.0f + WIDTH - 2 * WIDTH / 16 , 0.0f, 0.0f, D3DCOLOR_XRGB(26, 26, 26) },
	{ 10.0f + 2 * WIDTH / 16, 100.0f + WIDTH - 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(26, 26, 26) },

	//HEALTH (GREEN)
	{ 10.0f + 2 * WIDTH / 16 , 100.0f + 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(136, 211, 103) },
	{ 10.0f + m_llife, 100.0f + 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(136, 211, 103) },
	{ 10.0f + 2 * WIDTH / 16, 100.0f + WIDTH - 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(136, 211, 103) },

	{ 10.0f + m_llife , 100.0f + 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(136, 211, 103) },
	{ 10.0f + m_llife, 100.0f + WIDTH - 2 * WIDTH / 16 , 0.0f, 0.0f, D3DCOLOR_XRGB(136, 211, 103) },
	{ 10.0f + 2 * WIDTH / 16, 100.0f + WIDTH - 2 * WIDTH / 16, 0.0f, 0.0f, D3DCOLOR_XRGB(136, 211, 103) },

};
*/

/*
void D3DHook::ipBox(const char *str)
{

float x = m_width/2, y = m_height / 15, l = 100, w = 13;
x -= l / 2;
D3DCOLOR color = D3DCOLOR_ARGB(255, 0, 0, 0);

m_ipvertices.push_back({ x, y, 0.0f, 0.0f, color });
m_ipvertices.push_back({ x + l, y, 0.0f, 0.0f, color });
m_ipvertices.push_back({ x, y + w, 0.0f, 0.0f, color });

m_ipvertices.push_back({ x + l, y, 0.0f, 0.0f, color });
m_ipvertices.push_back({ x + l, y + w, 0.0f, 0.0f, color });
m_ipvertices.push_back({ x, y + w, 0.0f, 0.0f, color });


// create a vertex buffer interface called m_vbuffer
m_d3ddev->CreateVertexBuffer(m_ipvertices.size() * sizeof(CUSTOMVERTEX), NULL, CUSTOMFVF, D3DPOOL_MANAGED, &m_ipvbuffer, NULL);

VOID* pipVoid;    // a void pointer

CUSTOMVERTEX iparray[100];
std::copy(m_ipvertices.begin(), m_ipvertices.end(), iparray);
// lock m_vbuffer and load the vertices into it

m_ipvbuffer->Lock(0, 0, (void**)&pipVoid, 0);
memcpy(pipVoid, iparray, 20 * m_ipvertices.size());
m_ipvbuffer->Unlock();
}
*/
