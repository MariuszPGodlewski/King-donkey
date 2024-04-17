#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH		960
#define SCREEN_HEIGHT		720
#define MAXSTRLN			128
#define ZEROPLATFORM		20
#define MARGINRIGHT			95
#define MARGINLEFT			140
#define MAP1LEVELHEIGHT		90
#define LADDERMARGINLEFT	144
#define LADDERMARGINRIGHT	130
#define PLATFORMWIDTH		52
#define PLATFORMHEIGHT		20
#define MARIOHEIGHT			50
#define LADDERHEIGHT		90
#define LADDERWIDTH			28
#define LADDERWIDTHRANGE	22
#define LADDERHEIGHTHRANGE	51
#define LADDERMVBOT			130
#define MARIOMARGIN			25
#define	SLOAP				2
#define NROFFLORS			4
#define NROFFLORSM2			4
#define NROFPLATFORMLV		20
#define MVSPEED				2
#define LADDERPLACE			13
#define LEFTMVTC			1
#define SPEEDMULTI			3
#define JUMPHEIGHT			11
#define GRAVITY				SLOAP
#define TREASUREHEIGHT		50  
#define TREASUREY			154
#define	TREASUREX			6* PLATFORMWIDTH - 40
#define DIMONDSIDE			25
#define DIAMONDNR			2
#define DIAMONDPOSIT		16 //wchich platform
#define POINTFORLEVEL		1500
#define POINTFORDIAMOND		800
const double FPS = 50;
const double FPSANIMATION = FPS / 2;
const double TIMEANIDELTA = 1 / FPS;

typedef struct var {
	char text[MAXSTRLN];
	int t1, t2, quit = 0, frames, rc, direction = 0, selectedmap = 1, won = 0, points = 0;
	int black, blue, red, green;
	double delta, worldTime = 0, accumulatedTime = 0, accumulatedTimeanimation = 0, targetFrameTime = 1 / FPS, jumptime = 0, diamondtime=0;
	int  diamondcolected[DIAMONDNR] = { 1, 1 };//1 represent that dimond was not colected
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface * platform1 = NULL, * platform2 = NULL, * diamond = NULL, * ladder = NULL, * bigplatform = NULL, * bigplatform2 = NULL, * marioR = NULL, * marioRmv = NULL, * marioL = NULL, * marioLmv = NULL, * treasure = NULL;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Rect botrect, marioHB, ladderHB, platformHB, treasureHB, diamondHB;
	SDL_Texture * bottexture, * bottexture1, * bottexture2, * mariotexture, * Rmariotexture, * mvRmariotexture, * Lmariotexture, * mvLmariotexture, * laddertexture, * platformtexture, * platform1texture, * platform2texture, * treasuretexture, * diamondtexture;
};

// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};
// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};
// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};
// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};
// rysowanie prostokπta o d≥ugoúci bokÛw l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void MarioPosit(var& g) {
	g.marioHB.x = 0;
	g.marioHB.y = SCREEN_HEIGHT - ZEROPLATFORM - MARIOHEIGHT;
	g.marioHB.w = MARIOHEIGHT;
	g.marioHB.h = MARIOHEIGHT;
}

void Exit(var& g) {
	SDL_FreeSurface(g.charset);
	SDL_FreeSurface(g.diamond);
	SDL_FreeSurface(g.platform1);
	SDL_FreeSurface(g.platform2);
	SDL_FreeSurface(g.ladder);
	SDL_FreeSurface(g.treasure);
	SDL_FreeSurface(g.bigplatform);
	SDL_FreeSurface(g.bigplatform2);
	SDL_FreeSurface(g.marioR);
	SDL_FreeSurface(g.marioRmv);
	SDL_FreeSurface(g.marioL);
	SDL_FreeSurface(g.marioLmv);
	SDL_DestroyTexture(g.scrtex);
	SDL_DestroyWindow(g.window);
	SDL_DestroyRenderer(g.renderer);
	SDL_Quit();
}

int Initialization(var& g) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	g.rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&g.window, &g.renderer);
	if (g.rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(g.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(g.renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(g.window, "Szablon do zdania drugiego 2017");


	g.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	g.scrtex = SDL_CreateTexture(g.renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_ShowCursor(SDL_DISABLE);
	return 0;
}

int ReadPhotos(var& g) {
	// wczytanie obrazka cs8x8.bmp
	g.charset = SDL_LoadBMP("./cs8x8.bmp");
	if (g.charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(g.screen);
		SDL_DestroyTexture(g.scrtex);
		SDL_DestroyWindow(g.window);
		SDL_DestroyRenderer(g.renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(g.charset, true, 0x000000);
	//readning diamond image
	g.diamond = SDL_LoadBMP("./dimond.bmp");
	if (g.diamond == NULL) {
		printf("SDL_LoadBMP(diamond.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	//readning platform1 image
	g.platform1 = SDL_LoadBMP("./platforma1.bmp");
	if (g.platform1 == NULL) {
		printf("SDL_LoadBMP(platforma1.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	//readning platform2 image
	g.platform2 = SDL_LoadBMP("./platforma2.bmp");
	if (g.platform2 == NULL) {
		printf("SDL_LoadBMP(platforma2.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	//readning ladder image
	g.ladder = SDL_LoadBMP("./drabina1.bmp");
	if (g.ladder == NULL) {
		printf("SDL_LoadBMP(drabina1.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	//readning treasure image
	g.treasure = SDL_LoadBMP("./treasure.bmp");
	if (g.treasure == NULL) {
		printf("SDL_LoadBMP(treasure.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	//readning bigplatfomr image
	g.bigplatform = SDL_LoadBMP("./bigplatform.bmp");
	if (g.bigplatform == NULL) {
		printf("SDL_LoadBMP(bigplatform.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	//readning bigplatfomr2 image
	g.bigplatform2 = SDL_LoadBMP("./bigplatform2.bmp");
	if (g.bigplatform2 == NULL) {
		printf("SDL_LoadBMP(bigplatform2.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	//readning mario image
	g.marioR = SDL_LoadBMP("./Rmario.bmp");
	if (g.marioR == NULL) {
		printf("SDL_LoadBMP(Rmario.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	g.marioRmv = SDL_LoadBMP("./mvRmario.bmp");
	if (g.marioRmv == NULL) {
		printf("SDL_LoadBMP(mvRmario.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	g.marioL = SDL_LoadBMP("./Lmario.bmp");
	if (g.marioL == NULL) {
		printf("SDL_LoadBMP(Lmario.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};
	g.marioLmv = SDL_LoadBMP("./mvLmario.bmp");
	if (g.marioLmv == NULL) {
		printf("SDL_LoadBMP(mvLmario.bmp) error: %s\n", SDL_GetError());
		Exit(g);
		return 1;
	};

	return 0;
}

void TextureDeclaration(var& g) {
	//craeting treasure texture and giving treasure right properties
	g.treasuretexture = SDL_CreateTextureFromSurface(g.renderer, g.treasure);
	SDL_FreeSurface(g.treasure);
	g.treasureHB.w = TREASUREHEIGHT;
	g.treasureHB.h = TREASUREHEIGHT;
	//craeting diamond texture and giving treasure right properties
	g.diamondtexture = SDL_CreateTextureFromSurface(g.renderer, g.diamond);
	SDL_FreeSurface(g.diamond);
	g.diamondHB.w = DIMONDSIDE;
	g.diamondHB.h = DIMONDSIDE;
	//craeting bottom texture and giving treasure right properties
	g.bottexture1 = SDL_CreateTextureFromSurface(g.renderer, g.bigplatform);
	SDL_FreeSurface(g.bigplatform);
	g.bottexture2 = SDL_CreateTextureFromSurface(g.renderer, g.bigplatform2);
	SDL_FreeSurface(g.bigplatform2);
	g.bottexture = g.bottexture1;//setting defoult bottom texture
	g.botrect.x = 0;
	g.botrect.y = SCREEN_HEIGHT - ZEROPLATFORM;
	g.botrect.w = SCREEN_WIDTH;
	g.botrect.h = ZEROPLATFORM;
	//craeting platform texture and giving treasure right properties
	g.platform1texture = SDL_CreateTextureFromSurface(g.renderer, g.platform1);
	SDL_FreeSurface(g.platform1);
	g.platform2texture = SDL_CreateTextureFromSurface(g.renderer, g.platform2);
	SDL_FreeSurface(g.platform2);

	g.platformtexture = g.platform1texture;
	g.platformHB.x = SCREEN_WIDTH - MARGINRIGHT - PLATFORMWIDTH;
	g.platformHB.y = SCREEN_HEIGHT - ZEROPLATFORM - MAP1LEVELHEIGHT;
	g.platformHB.w = PLATFORMWIDTH;
	g.platformHB.h = PLATFORMHEIGHT;
	//craeting ladder texture and giving treasure right properties
	g.laddertexture = SDL_CreateTextureFromSurface(g.renderer, g.ladder);
	SDL_FreeSurface(g.ladder);

	g.ladderHB.x = SCREEN_WIDTH - LADDERMARGINLEFT;
	g.ladderHB.y = SCREEN_HEIGHT - ZEROPLATFORM - LADDERHEIGHT;
	g.ladderHB.w = LADDERWIDTH;
	g.ladderHB.h = LADDERHEIGHT;
	//craeting mario texture and giving treasure right properties
	g.Rmariotexture = SDL_CreateTextureFromSurface(g.renderer, g.marioR);
	SDL_FreeSurface(g.marioR);
	g.mvRmariotexture = SDL_CreateTextureFromSurface(g.renderer, g.marioRmv);
	SDL_FreeSurface(g.marioRmv);
	g.Lmariotexture = SDL_CreateTextureFromSurface(g.renderer, g.marioL);
	SDL_FreeSurface(g.marioL);
	g.mvLmariotexture = SDL_CreateTextureFromSurface(g.renderer, g.marioLmv);
	SDL_FreeSurface(g.marioLmv);
	g.mariotexture = g.Lmariotexture;//settign deafulf mario texture

	MarioPosit(g);
}

void VaribleDeclaration(var& g) {
	//setting colors
	g.black = SDL_MapRGB(g.screen->format, 0x00, 0x00, 0x00);
	g.green = SDL_MapRGB(g.screen->format, 0x00, 0xFF, 0x00);
	g.red = SDL_MapRGB(g.screen->format, 0xFF, 0x00, 0x00);
	g.blue = SDL_MapRGB(g.screen->format, 0x11, 0x11, 0xCC);
	//getting first time tick
	g.t1 = SDL_GetTicks();
}

void InformationGenerator(var& g) {
	//generate top rectange which containn text which we enter 
	DrawRectangle(g.screen, 4, 4, SCREEN_WIDTH - 8, 36, g.red, g.blue);
	sprintf(g.text, "Mariusz Godlewski 197816, czas trwania = %.1lf s, points : %d  ", g.worldTime, g.points);
	DrawString(g.screen, g.screen->w / 2 - strlen(g.text) * 8 / 2, 10, g.text, g.charset);
	sprintf(g.text, "Esc - exit, n - new game, SPACE - jump, Done: A,B,E,F,H");
	DrawString(g.screen, g.screen->w / 2 - strlen(g.text) * 8 / 2, 26, g.text, g.charset);
	if (g.won) {//won message
		sprintf(g.text, "You won, your score is %d", g.points);
		DrawString(g.screen, g.screen->w / 2 - strlen(g.text) * 8 / 2, SCREEN_HEIGHT / 2, g.text, g.charset);
	}
	if (g.diamondtime > 0) {//bonus points above mario character
		g.diamondtime -= g.delta;
		sprintf(g.text, " +%d", POINTFORDIAMOND);
		DrawString(g.screen, g.marioHB.x, g.marioHB.y - 10, g.text, g.charset);
	}
}

void Map1ResetTreasureLocation(var& g) {
	g.treasureHB.x = SCREEN_WIDTH - TREASUREX;
	g.treasureHB.y = TREASUREY;
}

void ResetPlatformLocation(var& g) {
	g.platformHB.x = SCREEN_WIDTH - MARGINRIGHT - PLATFORMWIDTH;
	g.platformHB.y = SCREEN_HEIGHT - ZEROPLATFORM - MAP1LEVELHEIGHT;
}

bool CheckIsPlatformInFront(var& g, int move) {
	if (g.marioHB.x - g.platformHB.x + MARIOHEIGHT / 2 == move || g.marioHB.x - g.platformHB.x + MARIOHEIGHT / 2 == PLATFORMWIDTH + move)
		return true;
	return false;
}

void ReserLadderLocation(var& g) {
	g.ladderHB.x = SCREEN_WIDTH - LADDERMARGINLEFT;
	g.ladderHB.y = SCREEN_HEIGHT - ZEROPLATFORM - LADDERHEIGHT;
}

bool PlatformUPSteap(var& g, int platformwidth, bool generate, bool move) {
	for (int i = 0; i < NROFPLATFORMLV; i++) {
		if (generate)//render platfoem
			SDL_RenderCopy(g.renderer, g.platformtexture, NULL, &g.platformHB);
		else if (abs(g.marioHB.y + MARIOHEIGHT - g.platformHB.y) < 2) {
			if (move) {// check if there is a platfoem in front 
				if (CheckIsPlatformInFront(g, MVSPEED))
					return true;
				if (CheckIsPlatformInFront(g, -LEFTMVTC))
					return true;
			}
			else {// check if there is a platform below
				if (g.marioHB.x - g.platformHB.x + MARIOHEIGHT / 2 >= 0 && g.marioHB.x - g.platformHB.x + MARIOHEIGHT / 2 <= PLATFORMWIDTH)
					return true;
			}
		}
		g.platformHB.x = g.platformHB.x + platformwidth;
		g.platformHB.y -= SLOAP;
	}
	return false;
}

int Map1Diamonds(var& g, bool check) {
	g.diamondHB.x = PLATFORMWIDTH;
	g.diamondHB.y = SCREEN_HEIGHT - MAP1LEVELHEIGHT - ZEROPLATFORM - DIMONDSIDE - DIAMONDPOSIT * SLOAP;
	for (size_t i = 0; i < DIAMONDNR; i++) {
		if (g.diamondcolected[i]) {
			SDL_RenderCopy(g.renderer, g.diamondtexture, NULL, &g.diamondHB);
			if (abs(g.marioHB.y - g.diamondHB.y) <= MARIOHEIGHT / 2 && abs(g.marioHB.x - g.diamondHB.x) <= DIMONDSIDE)
				return i + 1;
		}
		g.diamondHB.x = SCREEN_WIDTH - PLATFORMWIDTH;
		g.diamondHB.y -= MAP1LEVELHEIGHT + NROFPLATFORMLV * SLOAP;
	}
	return 0;
}

void PlatformLeft(var& g, int lvheight) {
	g.platformHB.x = MARGINLEFT;
	g.platformHB.y -= lvheight;
}

void PlatformRight(var& g, int lvheight) {
	g.platformHB.x = SCREEN_WIDTH - MARGINRIGHT - PLATFORMWIDTH;
	g.platformHB.y -= lvheight;
}

void Map1PlatformGenerator(var& g) {
	PlatformUPSteap(g, -PLATFORMWIDTH, true, false);
	PlatformLeft(g, MAP1LEVELHEIGHT);
	PlatformUPSteap(g, PLATFORMWIDTH, true, false);
	PlatformRight(g, MAP1LEVELHEIGHT);
	PlatformUPSteap(g, -PLATFORMWIDTH, true, false);
	PlatformLeft(g, MAP1LEVELHEIGHT);
	PlatformUPSteap(g, PLATFORMWIDTH, true, false);
}

void Map1LadderGenerator(var& g) {
	for (int i = 0; i < NROFFLORS; i++) {
		SDL_RenderCopy(g.renderer, g.laddertexture, NULL, &g.ladderHB);
		if (i % 2 == 0)
			g.ladderHB.x = LADDERMARGINLEFT;
		else
			g.ladderHB.x = SCREEN_WIDTH - LADDERMARGINRIGHT;
		g.ladderHB.y -= MAP1LEVELHEIGHT + SLOAP * LADDERPLACE;
		if (i > 0) {
			g.ladderHB.y -= SLOAP * (NROFPLATFORMLV - LADDERPLACE);
		}
	}
}

void Map1Generator(var& g) {
	Map1ResetTreasureLocation(g);
	ReserLadderLocation(g);
	ResetPlatformLocation(g);
	SDL_RenderCopy(g.renderer, g.bottexture, NULL, &g.botrect);
	Map1LadderGenerator(g);
	SDL_RenderCopy(g.renderer, g.treasuretexture, NULL, &g.treasureHB);
	Map1PlatformGenerator(g);
	Map1Diamonds(g, false);
	SDL_RenderCopy(g.renderer, g.mariotexture, NULL, &g.marioHB);
}

bool Map1IsPlatform(var& g, bool move) {
	ResetPlatformLocation(g);
	if (PlatformUPSteap(g, -PLATFORMWIDTH, false, move))
		return true;
	PlatformLeft(g, MAP1LEVELHEIGHT);
	if (PlatformUPSteap(g, PLATFORMWIDTH, false, move))
		return true;
	PlatformRight(g, MAP1LEVELHEIGHT);
	if (PlatformUPSteap(g, -PLATFORMWIDTH, false, move))
		return true;
	PlatformLeft(g, MAP1LEVELHEIGHT);
	if (PlatformUPSteap(g, PLATFORMWIDTH, false, move))
		return true;
	return false;
}

bool Map1IsLadder(var& g, bool  bottom) {
	ReserLadderLocation(g);
	for (int i = 0; i < NROFFLORS; i++) {
		if (!bottom) {
			if (abs(g.marioHB.y - g.ladderHB.y) < LADDERHEIGHTHRANGE && abs(g.marioHB.x - g.ladderHB.x + MARIOHEIGHT / 2 - LADDERWIDTH / 2) < LADDERWIDTHRANGE) {
				return true;
			}
		}
		else {
			if (abs(g.marioHB.y - g.ladderHB.y) <= LADDERHEIGHTHRANGE && abs(g.marioHB.x - g.ladderHB.x + MARIOHEIGHT / 2 - LADDERWIDTH / 2) < LADDERWIDTHRANGE && !(abs(g.marioHB.y - g.ladderHB.y - MARIOHEIGHT) < 2)) {
				return true;
			}
		}
		if (i % 2 == 0)
			g.ladderHB.x = LADDERMARGINLEFT;
		else
			g.ladderHB.x = SCREEN_WIDTH - LADDERMARGINRIGHT;
		g.ladderHB.y -= MAP1LEVELHEIGHT + SLOAP * LADDERPLACE;
		if (i > 0) {
			g.ladderHB.y -= SLOAP * (NROFPLATFORMLV - LADDERPLACE);
		}
		if (i == 0)
			g.ladderHB.y -= LADDERPLACE;
	}
	return false;
}

int Map3Diamonds(var& g, bool check) {
	g.diamondHB.x = SCREEN_WIDTH - PLATFORMWIDTH;
	g.diamondHB.y = SCREEN_HEIGHT - ZEROPLATFORM - DIMONDSIDE;
	for (size_t i = 0; i < DIAMONDNR; i++) {
		if (g.diamondcolected[i]) {
			SDL_RenderCopy(g.renderer, g.diamondtexture, NULL, &g.diamondHB);
			if (abs(g.marioHB.y - g.diamondHB.y) <= MARIOHEIGHT / 2  && abs(g.marioHB.x - g.diamondHB.x) <= DIMONDSIDE)
				return i + 1;
		}
		g.diamondHB.x = SCREEN_WIDTH - PLATFORMWIDTH;
		g.diamondHB.y -= MAP1LEVELHEIGHT + NROFPLATFORMLV * SLOAP;
	}
	return 0;
}

void Map3ResetTreasureLocation(var& g) {
	g.treasureHB.x = TREASUREX;
	g.treasureHB.y = TREASUREY;
}

void Map3ResetPlatform(var& g) {
	g.platformHB.x = PLATFORMWIDTH;
	g.platformHB.y = SCREEN_HEIGHT - ZEROPLATFORM - MAP1LEVELHEIGHT;
}

void Map3ResetLadder(var& g) {
	g.ladderHB.x = LADDERMARGINRIGHT / 2;
	g.ladderHB.y = SCREEN_HEIGHT - ZEROPLATFORM - LADDERHEIGHT;
}

void Map3PlatformGenerator(var& g) {
	PlatformUPSteap(g, PLATFORMWIDTH, true, false);
	PlatformRight(g, MAP1LEVELHEIGHT);
	PlatformUPSteap(g, -PLATFORMWIDTH, true, false);
	PlatformLeft(g, MAP1LEVELHEIGHT);
	PlatformUPSteap(g, PLATFORMWIDTH, true, false);
	PlatformRight(g, MAP1LEVELHEIGHT);
	PlatformUPSteap(g, -PLATFORMWIDTH, true, false);
}

void Map3LadderGenerator(var& g) {
	for (int i = 0; i < NROFFLORS; i++) {
		SDL_RenderCopy(g.renderer, g.laddertexture, NULL, &g.ladderHB);
		if (i % 2 == 1)
			g.ladderHB.x = LADDERMARGINLEFT;
		else
			g.ladderHB.x = SCREEN_WIDTH - LADDERMARGINRIGHT;
		g.ladderHB.y -= MAP1LEVELHEIGHT + SLOAP * LADDERPLACE;
		if (i > 0) {
			g.ladderHB.y -= SLOAP * (NROFPLATFORMLV - LADDERPLACE);
		}
	}
}

void Map3Generator(var& g) {
	Map3ResetTreasureLocation(g);
	Map3ResetLadder(g);
	Map3ResetPlatform(g);
	SDL_RenderCopy(g.renderer, g.bottexture, NULL, &g.botrect);
	Map3LadderGenerator(g);
	SDL_RenderCopy(g.renderer, g.treasuretexture, NULL, &g.treasureHB);
	Map3PlatformGenerator(g);
	Map3Diamonds(g, false);
	SDL_RenderCopy(g.renderer, g.mariotexture, NULL, &g.marioHB);
}

bool Map3IsPlatform(var& g, bool move) {
	Map3ResetPlatform(g);
	if (PlatformUPSteap(g, PLATFORMWIDTH, false, move))
		return true;
	PlatformRight(g, MAP1LEVELHEIGHT);
	if (PlatformUPSteap(g, -PLATFORMWIDTH, false, move))
		return true;
	PlatformLeft(g, MAP1LEVELHEIGHT);
	if (PlatformUPSteap(g, PLATFORMWIDTH, false, move))
		return true;
	PlatformRight(g, MAP1LEVELHEIGHT);
	if (PlatformUPSteap(g, -PLATFORMWIDTH, false, move))
		return true;
	return false;
}

bool Map3IsLadder(var& g, bool bottom) {
	Map3ResetLadder(g);
	for (int i = 0; i < NROFFLORS; i++) {
		if (!bottom) {
			if (abs(g.marioHB.y - g.ladderHB.y) < LADDERHEIGHTHRANGE && abs(g.marioHB.x - g.ladderHB.x + MARIOHEIGHT / 2 - LADDERWIDTH / 2) < LADDERWIDTHRANGE) {
				return true;
			}
		}
		else {
			if (abs(g.marioHB.y - g.ladderHB.y) <= LADDERHEIGHTHRANGE && abs(g.marioHB.x - g.ladderHB.x + MARIOHEIGHT / 2 - LADDERWIDTH / 2) < LADDERWIDTHRANGE && !(abs(g.marioHB.y - g.ladderHB.y - MARIOHEIGHT) < 2)) {
				return true;
			}
		}
		if (i % 2 == 1)
			g.ladderHB.x = LADDERMARGINLEFT;
		else
			g.ladderHB.x = SCREEN_WIDTH - LADDERMARGINRIGHT;
		g.ladderHB.y -= MAP1LEVELHEIGHT + SLOAP * LADDERPLACE;
		if (i > 0) {
			g.ladderHB.y -= SLOAP * (NROFPLATFORMLV - LADDERPLACE);
		}
		if (i == 0)
			g.ladderHB.y -= LADDERPLACE;
	}
	return false;
}

bool IsPlatform(var& g, bool move) {
	if (g.selectedmap == 1)
		return Map1IsPlatform(g, move);
	else if (g.selectedmap == 3)
		return Map3IsPlatform(g, move);
}

bool IsLadder(var& g, bool move) {
	if (g.selectedmap == 1)
		return Map1IsLadder(g, move);
	else if (g.selectedmap == 3)
		return Map3IsLadder(g, move);
}

void Game(var& g);

void CallMap(var& g) {
	if (g.selectedmap == 1) {
		g.platformtexture = g.platform1texture;
		g.bottexture = g.bottexture1;
	}
	else {
		g.bottexture = g.bottexture2;
		g.platformtexture = g.platform2texture;
	}
	for (int i = 0; i < DIAMONDNR; i++)
		g.diamondcolected[i] = 1;
	g.jumptime = 0;
	MarioPosit(g);
	Game(g);
}

void Gravity(var& g) {
	for (int i = 0; i < SPEEDMULTI ; i++) {
		if (!IsLadder(g, false)) {
			if (IsPlatform(g, false)) {}
			else if (g.marioHB.y < SCREEN_HEIGHT - ZEROPLATFORM - MARIOHEIGHT) {
				g.marioHB.y += GRAVITY;
			}
		}
	}
}

void IsDiamond(var& g) {
	int colectedDimond = 0;
	if (g.selectedmap == 1)
		colectedDimond = Map1Diamonds(g, true);
	else
		colectedDimond = Map3Diamonds(g, true);
	if (colectedDimond) {
		g.points += POINTFORDIAMOND;
		g.diamondcolected[colectedDimond - 1] = 0;
		g.diamondtime = 1;
	}
}

void IsTreasure(var& g) {
	if (g.marioHB.y - g.treasureHB.y <= 0 && abs(g.marioHB.x - g.treasureHB.x) < 25) {
		if (!g.won)
			g.points += POINTFORLEVEL;
		if (g.selectedmap == 1) {
			g.selectedmap += 2;
			MarioPosit(g);
			CallMap(g);
		}
		else if (g.selectedmap == 3) {
			g.won = 1;
		}
	}
}

void Jump(var& g) {
	if (g.jumptime > NULL) {
		g.jumptime -= TIMEANIDELTA;
		g.marioHB.y -= JUMPHEIGHT * g.jumptime;
	}
}

void TimeCalculation(var& g)
{
	g.t2 = SDL_GetTicks();
	g.delta = (g.t2 - g.t1) * 0.001;
	g.t1 = g.t2;
	g.worldTime += g.delta;
	g.accumulatedTime += g.delta;
	g.accumulatedTimeanimation += g.delta;
}

void Events(var& g) {
	while (SDL_PollEvent(&g.event)) {
		switch (g.event.type) {
		case SDL_KEYDOWN:
			if (g.event.key.keysym.sym == SDLK_ESCAPE) g.quit = 1;
			else if (g.event.key.keysym.sym == SDLK_UP) {
				if (g.marioHB.y <= SCREEN_HEIGHT - ZEROPLATFORM - MARIOHEIGHT) {
					for (int i = 0; i < SPEEDMULTI; i++) {
						g.marioHB.y -= MVSPEED;
					}
				}
			}
			else if (g.event.key.keysym.sym == SDLK_DOWN) {
				for (int i = 0; i < SPEEDMULTI; i++) {
					if (g.marioHB.y < SCREEN_HEIGHT - ZEROPLATFORM - MARIOHEIGHT) {
						if (IsLadder(g, true)) {
							g.marioHB.y += MVSPEED;
						}
					}
				}
			}
			else if (g.event.key.keysym.sym == SDLK_LEFT) {
				if (g.marioHB.x > 0) {
					for (int i = 0; i < SPEEDMULTI; i++) {
						if (IsPlatform(g, true))
							g.marioHB.y -= SLOAP;
						g.marioHB.x -= MVSPEED;
					}
					g.mariotexture = g.mvLmariotexture;
					g.direction = 1;
				}
			}
			else if (g.event.key.keysym.sym == SDLK_RIGHT) {
				if (g.marioHB.x < SCREEN_WIDTH - MARIOHEIGHT) {
					for (int i = 0; i < SPEEDMULTI; i++) {
						if (IsPlatform(g, true))
							g.marioHB.y -= SLOAP;
						g.marioHB.x += MVSPEED;
					}
					g.mariotexture = g.mvRmariotexture;
					g.direction = 0;
				}
			}
			else if (g.event.key.keysym.sym == SDLK_SPACE) {
				if (IsPlatform(g, false) || g.marioHB.y >= SCREEN_HEIGHT - ZEROPLATFORM - MARIOHEIGHT)
					g.jumptime = 1;
			}
			else if (g.event.key.keysym.sym == SDLK_1) {
				g.selectedmap = 1;
				CallMap(g);
			}
			else if (g.event.key.keysym.sym == SDLK_2) {
				g.selectedmap = 3;
				CallMap(g);
			}
			else if (g.event.key.keysym.sym == SDLK_3) {
				g.selectedmap = 3;
				CallMap(g);
			}
			else if (g.event.key.keysym.sym == SDLK_n) {
				for (int i = 0; i < DIAMONDNR; i++)
					g.diamondcolected[i] = 1;
				g.selectedmap = 1;
				g.won = 0;
				g.worldTime = 0;
				g.points = 0;
				CallMap(g);
			}
		case SDL_KEYUP:
			;
			break;
		case SDL_QUIT:
			g.quit = 1;
			break;
		};
	};
}

void Game(var& g) {
	VaribleDeclaration(g);
	int k = 0;
	while (!g.quit) {

		SDL_RenderCopy(g.renderer, g.scrtex, NULL, NULL);
		SDL_FillRect(g.screen, NULL, g.black);
		TimeCalculation(g);
		if (g.selectedmap == 1 && !g.won)
			Map1Generator(g);
		else if (g.selectedmap == 3 && !g.won)
			Map3Generator(g);

		InformationGenerator(g);

		SDL_UpdateTexture(g.scrtex, NULL, g.screen->pixels, g.screen->pitch);

		SDL_RenderPresent(g.renderer);
		if (g.accumulatedTimeanimation >= 2 * g.targetFrameTime) {
			g.accumulatedTimeanimation = NULL;
			if (k % 2 == 0) {
				if (g.direction == 0)
					g.mariotexture = g.Rmariotexture;
				else
					g.mariotexture = g.Lmariotexture;
			}
			k++;
		}
		if (g.accumulatedTime >= g.targetFrameTime) {
			IsTreasure(g);
			g.accumulatedTime = NULL;
			Jump(g);
			Events(g);
			IsDiamond(g);
			Gravity(g);
		}

	};
}

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	var g;
	if (Initialization(g))
		return 1;

	if (ReadPhotos(g))
		return 1;

	TextureDeclaration(g);
	Game(g);

	// zwolnienie powierzchni / freeing all surfaces 
	SDL_FreeSurface(g.charset);
	SDL_FreeSurface(g.screen);
	SDL_DestroyTexture(g.bottexture);
	SDL_DestroyTexture(g.bottexture2);
	SDL_DestroyTexture(g.bottexture2);
	SDL_DestroyTexture(g.mariotexture);
	SDL_DestroyTexture(g.Rmariotexture);
	SDL_DestroyTexture(g.mvRmariotexture);
	SDL_DestroyTexture(g.Lmariotexture);
	SDL_DestroyTexture(g.mvLmariotexture);
	SDL_DestroyTexture(g.laddertexture);
	SDL_DestroyTexture(g.platformtexture);
	SDL_DestroyTexture(g.platform1texture);
	SDL_DestroyTexture(g.platform2texture);
	SDL_DestroyTexture(g.treasuretexture);
	SDL_DestroyTexture(g.diamondtexture);
	SDL_DestroyTexture(g.scrtex);
	SDL_DestroyRenderer(g.renderer);
	SDL_DestroyWindow(g.window);

	SDL_Quit();
	return 0;
};
