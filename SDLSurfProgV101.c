#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <conio.h> // for _getch(), not my favorite, but will do for now.
#include "SDLSurfProg.h"
#include <assert.h>
//#include <errno.h> // Later, homebrew error check for now.


// Globals
SDL_Window* gWindow = NULL;
SDL_Surface* gWindowSurface = NULL;
// I do not like this, but I see it all over the place, so globals they are...
// Post edit: think I am getting the idea why, now. Must confirm.

//****************** MAIN ********************
//--------------------------------------------
int main(int argc, char* argv[])
{
	SDL_Surface* local_SurfImage = NULL; // Some questions arise... regarding local surfaces.
	char input_file_name[BUFFER];
	char output_file_name[BUFFER];
	SDL_memset(input_file_name, '\0', BUFFER);
	SDL_memset(output_file_name, '\0', BUFFER);
	Uint8* pixel_array = NULL; // Required HERE because this array must remmain while a data created surface is active.
	SDL_bool quit = SDL_FALSE;
	int user_command = 0;

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		int flags = IMG_INIT_PNG | IMG_INIT_JPG;
		if (IMG_Init(flags) & flags)
		{
			program_menu();

			while (quit == SDL_FALSE)
			{
				printf("\nPlease select a menu command item by number: ");
				user_command = _getch();
				printf("%c\n", (char)user_command);

				switch (user_command)
				{
				case '1':
					if (local_SurfImage == NULL)
					{
						local_SurfImage = load_base_image(input_file_name, output_file_name);
					}
					else
					{
						printf("A surface image is currently loaded. Please clear it, first (5).\n");
					}
					break;
				case '2':
					if (local_SurfImage != NULL)
					{
						save_image_data(local_SurfImage, output_file_name);
					}
					else
					{
						printf("No image is loaded to surface. Please load an image (1).\n");
					}
					break;
				case '3':
					if (local_SurfImage == NULL)
					{
						local_SurfImage = load_image_data(output_file_name, pixel_array);
					}
					else
					{
						printf("A surface image is currently loaded. Please clear it, first (5).\n");
					}
					break;
				case '4':
					if (local_SurfImage != NULL)
					{
						show_current_image(local_SurfImage);
					}
					else
					{
						printf("No image is loaded to surface. Please load an image (1 or 3).\n");
					}
					break;
				case '5':
					if (local_SurfImage != NULL)
					{
						SDL_FreeSurface(local_SurfImage);
						local_SurfImage = NULL;
					}
					if (pixel_array != NULL) // Code so that this is not necessary...
					{
						SDL_free(pixel_array);
						pixel_array = NULL;
					}
					printf("Surface cleared.\n");
					break;
				case '6':
					system("cls"); // Works on MSVS 2022 and GCC
					program_menu();
					break;
				case '7':
					if (local_SurfImage != NULL)
					{
						SDL_FreeSurface(local_SurfImage);
						local_SurfImage = NULL;
					}
					if (pixel_array != NULL)
					{
						SDL_free(pixel_array);
						pixel_array = NULL;
					}
					printf("Terminating the program.\n");
					quit = SDL_TRUE;
					SDL_Delay(1000);
					break;
				default:
					printf("Not a valid Menu Command.\n");
					break;
				}
			};
		}
		else
		{
			SDL_Log("IMG Init failure: %s", SDL_GetError());
			SDL_Quit();
		}// IMG_Init()
	}
	else
	{
		SDL_Log("SDL Init failure: %s", SDL_GetError());
	}// SDL_Init

	IMG_Quit();
	SDL_Quit();

	return 0;
}

//**************** FUNCTIONS ******************
//---------------------------------------------------------------
SDL_Surface* load_base_image(char* inf_name, char* outf_name)
{
	// Before coming here, the local_SurfImage and (if not NULL) the pixel array should be freed.
	// This function does not do any internal checking of them, only filenames.
	SDL_Surface* tempSurf = NULL;

	SDL_memset(inf_name, '\0', BUFFER); // Should not be necessary if surface clearing routine addresses this.
	SDL_memset(outf_name, '\0', BUFFER);

	printf("Enter the name of .bmp, .png or .jpg file to load (include extension):\n-> ");

	//scanf_s("%255[^ \n]s", inf_name, BUFFER); // make sure newline or space ends the string. MSVC 2022
	scanf("%255[^ \n]s", inf_name); // GCC
	//fflush(stdin);
	do {} while (getchar() != '\n');

	if (SDL_strlen(inf_name) != 0)
	{
		char* dot_ext = SDL_strchr(inf_name, '.');
		if (SDL_strcmp(dot_ext, ".png") == 0 || SDL_strcmp(dot_ext, ".PNG") == 0)
		{
			printf("File type is assumed .png\n");
			tempSurf = IMG_LoadPNG_RW(SDL_RWFromFile(inf_name, "rb"));
		}
		else if (SDL_strcmp(dot_ext, ".bmp") == 0 || SDL_strcmp(dot_ext, ".BMP") == 0)
		{
			printf("File type is assumed .bmp\n");
			tempSurf = SDL_LoadBMP_RW(SDL_RWFromFile(inf_name, "rb"), 1);
		}
		else if (SDL_strcmp(dot_ext, ".jpg") == 0 || SDL_strcmp(dot_ext, ".JPG") == 0)
		{
			printf("File type is assumed .jpg\n");
			tempSurf = IMG_LoadJPG_RW(SDL_RWFromFile(inf_name, "rb"));
		}
		else
		{
			printf("File extension | %s | is not known.\n", dot_ext);
			// ----
		}

		if (tempSurf != NULL) // Surface must have loaded with the provided file name.
		{
			SDL_memmove(outf_name, inf_name, (SDL_strlen(inf_name) - 4));
			SDL_strlcat(outf_name, ".bin", BUFFER);
			printf("Image loaded.\n");
		}
		else
		{
			SDL_memset(inf_name, '\0', BUFFER);
			printf("No image was loaded to surface.\n");
		}
	}
	else // User just hit enter
	{
		// Nothing entered at scanf.
	}

	return tempSurf;
}

//---------------------------------------------------------------
SDL_bool save_image_data(SDL_Surface* lSurf, const char* outf_name)
{
	SDL_bool data_saved = SDL_FALSE;
	surface_header local_Surf_head; // Must confirm this is safe in C

	if (lSurf != NULL)
	{
		size_t image_pixel_data_size = (size_t)(lSurf->w * lSurf->h * (int)lSurf->format->BytesPerPixel);
		// Initialize the structure from the active surface to be saved.
		local_Surf_head.Bits_pp = lSurf->format->BitsPerPixel;
		local_Surf_head.Bytes_pp = lSurf->format->BytesPerPixel;
		local_Surf_head.w = lSurf->w;
		local_Surf_head.h = lSurf->h;
		local_Surf_head.pixel_format = lSurf->format->format;
		local_Surf_head.image_data_size = image_pixel_data_size;

		if (local_Surf_head.Bits_pp == 8 ||
			local_Surf_head.Bits_pp == 24 ||
			local_Surf_head.Bits_pp == 32) // Accept only 32, 24 and 8 bit images for saving as bin file.
		{

			if (local_Surf_head.Bits_pp == 8)
			{
				local_Surf_head.ncolors = lSurf->format->palette->ncolors;
			}
			else
			{
				local_Surf_head.ncolors = -1;
			}

			SDL_RWops* save_file = SDL_RWFromFile(outf_name, "wb");
			if (save_file != NULL)
			{
				assert(SDL_RWwrite(save_file, &local_Surf_head, sizeof(surface_header), 1) == 1);
				assert(SDL_RWwrite(save_file, lSurf->pixels, sizeof(Uint8), image_pixel_data_size) == image_pixel_data_size);
				if (local_Surf_head.Bits_pp != 8)
				{
					printf("*** PIXEL DATA SAVED\n");
					data_saved = SDL_TRUE; // If it is not an 8 bit image, we're done.
				}
				else // 8 BIT
				{
					assert(SDL_RWwrite(save_file, lSurf->format->palette->colors, sizeof(SDL_Color), (size_t)local_Surf_head.ncolors)
						== (size_t)local_Surf_head.ncolors);
					printf("*** PIXEL DATA & PALETTE SAVED\n");
					data_saved = SDL_TRUE;
				}
				assert(SDL_RWclose(save_file) == 0);
			} // Open file for writing END
		}
		else
		{
			printf("Bit depth of image is not handled by this program.\n");
		} // Bit Depth valid check END
	}
	else
	{
		printf("No surface is loaded.\n");
	} // Surface exists check END

	return data_saved;
}

//---------------------------------------------------------------
SDL_Surface* load_image_data(char* outf_name, Uint8* pixel_data)
{
	SDL_Surface* tempSurf = NULL;
	SDL_Palette* local_surface_palette = NULL;
	SDL_Color* palette_colors = NULL;
	surface_header local_Surf_head; // Must confirm this is safe in C

	SDL_bool lock_surface = SDL_FALSE;

	SDL_memset(outf_name, '\0', BUFFER);
	printf("Enter the name of .bin file to load (include extension).\n-> ");
	//scanf_s("%255[^ \n]s", outf_name, BUFFER); // MSVC 2022
	scanf("%255[^ \n]s", outf_name); // GCC
	do {} while (getchar() != '\n');

	char* dot_ext = SDL_strchr(outf_name, '.');
	if (SDL_strlen(outf_name) == 0 || SDL_strcmp(".bin", dot_ext) != 0) // covers the user just hitting enter to abort...
	{
		printf("No .bin file specified.\n");
		return NULL;
	}

	SDL_RWops* load_file = SDL_RWFromFile(outf_name, "rb");
	if (load_file != NULL)
	{
		assert(SDL_RWread(load_file, &local_Surf_head, sizeof(surface_header), 1) == 1);
		// A major part of the function to keep an eye on...
		pixel_data = SDL_calloc(local_Surf_head.image_data_size, sizeof(Uint8));
		if (pixel_data != NULL)
		{
			assert(SDL_RWread(load_file, pixel_data, sizeof(Uint8), local_Surf_head.image_data_size) == local_Surf_head.image_data_size);
			printf("*** PIXEL DATA LOADED\n");
			// Pixel data read correctly, create the surface...
			tempSurf = SDL_CreateRGBSurfaceWithFormatFrom(pixel_data, local_Surf_head.w, local_Surf_head.h,
				local_Surf_head.Bits_pp, (int)(local_Surf_head.w * (int)local_Surf_head.Bytes_pp), local_Surf_head.pixel_format);

			if (SDL_MUSTLOCK(tempSurf) == SDL_TRUE)
				lock_surface = SDL_TRUE;

			if (tempSurf != NULL)
			{
				printf("*** SURFACE CREATED FROM LOADED DATA\n");
				// Now check if 8 bit
				if (local_Surf_head.Bits_pp == 8)
				{
					if (lock_surface == SDL_TRUE)
						SDL_LockSurface(tempSurf);

					local_surface_palette = SDL_AllocPalette(local_Surf_head.ncolors);
					if (local_surface_palette != NULL)
					{
						palette_colors = SDL_calloc(local_Surf_head.ncolors, sizeof(SDL_Color));
						if (palette_colors != NULL)
						{
							assert(SDL_RWread(load_file, palette_colors, sizeof(SDL_Color), (size_t)local_Surf_head.ncolors) ==
								(size_t)local_Surf_head.ncolors);

							if (SDL_SetPaletteColors(local_surface_palette, palette_colors, 0, local_Surf_head.ncolors) == 0)
							{
								SDL_free(palette_colors); // Palette colors can be freed once colors are transferred to palette.
								palette_colors = NULL;
								printf("*** PALETTE DATA LOADED.\n");

								if (SDL_SetSurfacePalette(tempSurf, local_surface_palette) == 0)
								{
									printf("*** PALETTE SET TO SURFACE\n");

								}
								else
								{
									printf("!!! PALETTE NOT SET TO SURFACE\n");
									SDL_Log("%s", SDL_GetError());
								}
							}
							else
							{
								printf("!!! PALETTE DATA NOT LOADED\n");
								SDL_Log("%s", SDL_GetError());
							}
						}
						SDL_FreePalette(local_surface_palette); // Palette may be freed after it is set to the 8 bit image.
						local_surface_palette = NULL;
					}
					if (lock_surface == SDL_TRUE)
						SDL_UnlockSurface(tempSurf);
				}
			}
		}
		assert(SDL_RWclose(load_file) == 0);
	}
	else
	{
		printf("No file to load.\n");
	}

	return tempSurf; // returns NULL if something failed and we are back where we started.
}

//---------------------------------------------------------------
void show_current_image(SDL_Surface* lSurf)
{
	gWindow = SDL_CreateWindow("View Surface", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		lSurf->w, lSurf->h, SDL_WINDOW_SHOWN);

	if (gWindow != NULL)
	{
		gWindowSurface = SDL_GetWindowSurface(gWindow);
		if (gWindowSurface != NULL)
		{
			SDL_BlitSurface(lSurf, NULL, gWindowSurface, NULL);
			SDL_UpdateWindowSurface(gWindow);
			printf("Showing image for 3 seconds...\n");
			SDL_Delay(3000); // Show image for 3 seconds.
		}
		else
		{
			SDL_Log("%s", SDL_GetError());
		}
	}
	else
	{
		SDL_Log("%s", SDL_GetError());
	}

	SDL_FreeSurface(gWindowSurface);
	gWindowSurface = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	printf("Window closed.\n");

}

//---------------------------------------------------------------
void program_menu(void)
{
	printf("SDL2 Surface Saver Program V 1.0.1\n\n");
	printf("*** MENU ***\n");
	printf("1. Load a .bmp, .png, or .jpg file to the Surface.\n");
	printf("2. Save the Surface to .bin file.\n");
	printf("3. Load a .bin file to the Surface.\n");
	printf("4. Show the currently active Surface.\n");
	printf("5. Clear currently active Surface.\n");
	printf("6. Refresh console.\n");
	printf("7. Exit the program.\n\n");
	printf("WARNING: Please use 7 to exit, do not simply close the console.\n");
}


// VESTIGIAL FAILURES - Reference only -
//---------------------------------------------------------------
/*
void clear_current_surface(SDL_Surface* lSurf, Uint8* pixel_data, SDL_bool quit_flag)
{
	if (lSurf != NULL)
	{
		SDL_FreeSurface(lSurf); // <-It does not complain, but does not seem to like this...
		lSurf = NULL;           // <-or this.
		// Freeing and setting to NULL the surface that resides locally in main()
		// causes program crashes when an attempt is made to load a new image to the
		// apparently freed and NULLed surface.
	}
	//SDL_memset(inf_name, '\0', BUFFER);
	//SDL_memset(outf_name, '\0', BUFFER);
	if (pixel_data != NULL)
	{
		SDL_free(pixel_data);
		pixel_data = NULL;
	}
	printf("CLEAR: Surface cleared.\n");
	if (quit_flag == SDL_TRUE)
	{
		printf("Terminating Program.\n");
		IMG_Quit();
		SDL_Quit();
	}
}
*/
//---------------------------------------------------------------
/*
void bin_file_name_manual_entry(char* outf_name)
{
	printf("Would you like to enter a .bin file name to load manually ? (y / n)\n");
	int y_n_prompt = '0';
	while (y_n_prompt != 'y' && y_n_prompt != 'Y' && y_n_prompt != 'n' && y_n_prompt != 'N')
	{
		y_n_prompt = _getch();
	};

	if (y_n_prompt == 'Y' || y_n_prompt == 'y')
	{
		printf("Please enter a .bin file name (include .bin extension):\n->");
		scanf_s("%255[^ \n]s", outf_name, BUFFER);
		do {} while (getchar() != '\n');
	}
}
*/

/*
	char* dot_ext = SDL_strchr(outf_name, '.');
	if (SDL_strlen(outf_name) == 0 || SDL_strcmp(".bin", dot_ext) != 0)
	{
		printf("No .bin file is specified.\n");
		bin_file_name_manual_entry(outf_name);
		if (SDL_strlen(outf_name) == 0) // covers the user just hitting enter to abort...
		{
			return NULL;
		}
	}
	*/
