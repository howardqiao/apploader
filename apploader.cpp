#include "apploader.h"

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 480;

SDL_Renderer* gRenderer = NULL;
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
LTexture img_splash;
LTexture img_x;
LTexture img_es;
LTexture img_kodi;
LTexture img_terminal;

SDL_Rect outlinelt = { 0, 0, 400, 240 };
SDL_Rect outlinert = { 400, 0, 400, 240 };
SDL_Rect outlineld = { 0, 240, 400, 240 };
SDL_Rect outlinerd = { 400, 240, 400, 240 };
SDL_Point cover_center = {0, 0};
int pos = 0;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	mPixels = NULL;
	mPitch = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Convert surface to display format
		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat( loadedSurface, SDL_PIXELFORMAT_RGBA8888, 0 );
		if( formattedSurface == NULL )
		{
			printf( "Unable to convert loaded surface to display format! %s\n", SDL_GetError() );
		}
		else
		{
			//Create blank streamable texture
			newTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h );
			if( newTexture == NULL )
			{
				printf( "Unable to create blank texture! SDL Error: %s\n", SDL_GetError() );
			}
			else
			{
				//Enable blending on texture
				SDL_SetTextureBlendMode( newTexture, SDL_BLENDMODE_BLEND );

				//Lock texture for manipulation
				SDL_LockTexture( newTexture, &formattedSurface->clip_rect, &mPixels, &mPitch );

				//Copy loaded/formatted surface pixels
				memcpy( mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h );

				//Get image dimensions
				mWidth = formattedSurface->w;
				mHeight = formattedSurface->h;

				//Get pixel data in editable format
				Uint32* pixels = (Uint32*)mPixels;
				int pixelCount = ( mPitch / 4 ) * mHeight;

				//Map colors				
				Uint32 colorKey = SDL_MapRGB( formattedSurface->format, 0, 0xFF, 0xFF );
				Uint32 transparent = SDL_MapRGBA( formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00 );

				//Color key pixels
				for( int i = 0; i < pixelCount; ++i )
				{
					if( pixels[ i ] == colorKey )
					{
						pixels[ i ] = transparent;
					}
				}

				//Unlock texture to update
				SDL_UnlockTexture( newTexture );
				mPixels = NULL;
			}

			//Get rid of old formatted surface
			SDL_FreeSurface( formattedSurface );
		}	
		
		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}
void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		mPixels = NULL;
		mPitch = 0;
	}
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip,bool zoom )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	if (zoom)
	{
		SDL_RenderCopyEx( gRenderer, mTexture, NULL, &renderQuad, angle, center, flip );
	} else {
		SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
	}
	
}

void LTexture::setAsRenderTarget()
{
	//Make self render target
	SDL_SetRenderTarget( gRenderer, mTexture );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

bool sdl_init()
{
    //cout << "init_sdl start" << endl;
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "UGeek Raspi Media Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED |SDL_RENDERER_TARGETTEXTURE );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}
    

    SDL_ShowCursor(0);
    //cout << "init_sdl over" << endl;
    //If everything initialized fine
    return success;
}

SDL_Texture* loadTexture( std::string path )
{
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}

bool load_files()
{
    //Load the background image
	img_splash.loadFromFile("/opt/retropie/configs/all/apploader/images/splash.png" );
    img_es.loadFromFile("/opt/retropie/configs/all/apploader/images/es.jpg" );
	img_kodi.loadFromFile("/opt/retropie/configs/all/apploader/images/kodi.jpg" );
	img_x.loadFromFile("/opt/retropie/configs/all/apploader/images/x.jpg" );
	img_terminal.loadFromFile("/opt/retropie/configs/all/apploader/images/terminal.png" );
    
    //If everything loaded fine
    return true;
}

void sdl_close()
{
    //Free the surfaces
    //SDL_DestroyTexture( cover_current );
	// SDL_DestroyTexture( img_es );
	// SDL_DestroyTexture( img_kodi );
	// SDL_DestroyTexture( img_x );
	// SDL_DestroyTexture( img_terminal );
	img_es.free();
	img_kodi.free();
	img_x.free();
	img_terminal.free();
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
    SDL_Quit();
}

void writeconfig(int p)
{
	ofstream myfile;
	myfile.open ("/home/pi/apploader/selection.conf");
	myfile << "sel=" << p << endl;
	myfile.close();
}

void draw_ui()
{
	img_es.render(0,0,&outlinelt,0,&cover_center,SDL_FLIP_NONE,true);
	img_kodi.render(400,0,&outlinelt,0,&cover_center,SDL_FLIP_NONE,true);
	img_x.render(0,240,&outlinelt,0,&cover_center,SDL_FLIP_NONE,true);
	img_terminal.render(400,240,&outlinelt,0,&cover_center,SDL_FLIP_NONE,true);
}

void draw_selection(int p)
{
	switch (p) 
	{
		case 0:
			SDL_RenderDrawRect(gRenderer , &outlinelt);
			break;
		case 1:
			SDL_RenderDrawRect(gRenderer , &outlinert);
			break;
		case 2:
			SDL_RenderDrawRect(gRenderer , &outlineld);
			break;
		case 3:
			SDL_RenderDrawRect(gRenderer , &outlinerd);
			break;
	}
}

int main( int argc, char* args[] )
{
	bool quit = false;
	SDL_Event e;
	sdl_init();
	load_files();
	SDL_RenderClear(gRenderer);
	img_splash.render(0,0);
	SDL_RenderPresent(gRenderer);
	SDL_Delay(2000);
	SDL_RenderClear(gRenderer);

	//cover_current.render(128,41,&cover_round,degree,&cover_center,SDL_FLIP_NONE,true);
	draw_ui();
	draw_selection(pos);
    SDL_RenderPresent(gRenderer);
	SDL_Point touchLocation;
	while( !quit )
	{
		//Handle events on queue
		while( SDL_PollEvent( &e ) != 0 )
		{
			//User requests quit
			if( e.type == SDL_QUIT )
			{
				quit = true;
			}
			//Touch down
			else if( e.type == SDL_FINGERDOWN )
			{
				SDL_SetRenderDrawColor(gRenderer,255,255,0,0);
				touchLocation.x = e.tfinger.x * SCREEN_WIDTH;
				touchLocation.y = e.tfinger.y * SCREEN_HEIGHT;
				if ((touchLocation.x < 400) && (touchLocation.y < 240))
				{
					pos = 0;
				}
				else if ((touchLocation.x > 400) && (touchLocation.y < 240))
				{
					pos = 1;
				}
				else if ((touchLocation.x < 400) && (touchLocation.y >= 240))
				{
					pos = 2;
				}
				else if ((touchLocation.x >= 400) && (touchLocation.y >= 240))
				{
					pos = 3;
				}
				draw_ui();
				draw_selection(pos);
				SDL_RenderPresent(gRenderer);
			}
			//Touch release
			else if( e.type == SDL_FINGERUP )
			{
				touchLocation.x = e.tfinger.x * SCREEN_WIDTH;
				touchLocation.y = e.tfinger.y * SCREEN_HEIGHT;
				writeconfig(pos);
				quit = true;
			}
			else if( e.type == SDL_KEYDOWN )
			{
				//Select surfaces based on key press
				switch( e.key.keysym.sym )
				{
					case SDLK_UP:
						if (pos > 1) { pos = pos - 2; }
						break;
					case SDLK_DOWN:
						if (pos < 2) { pos = pos + 2; }
						break;
					case SDLK_LEFT:
						if ((pos == 1) || (pos = 3)) { pos-- ; }
						break;
					case SDLK_RIGHT:
						if ((pos == 0) || ( pos = 2)) { pos++ ; }
						break;
					case SDLK_RETURN:
						writeconfig(pos);
						quit = true;
						break;
				}
				draw_ui();
				draw_selection(pos);
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	//SDL_Delay(5000);
	sdl_close();

	return 0;
}
