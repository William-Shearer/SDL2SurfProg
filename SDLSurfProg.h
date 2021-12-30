#ifndef _SDLSURFPROG_H
#define _SDLSURFPROG_H

// Macros / Constants
#define	BUFFER	256

/*// Enums - supressed - cumbersome and does not help.
enum {
	IMAGE_TYPE_NONE = 0,
	IMAGE_TYPE_BMP = 1,
	IMAGE_TYPE_PNG = 2,
	IMAGE_TYPE_JPG = 3
};

enum {
	BIT_BEPTH_UNDEFINED = 0,
	BIT_DEPTH_32 = 1,
	BIT_DEPTH_24 = 2,
	BIT_DEPTH_8 = 3
};*/

// Structures
typedef struct image_header
{
	int w;
	int h;
	Uint32 pixel_format;
	Uint8 Bytes_pp;
	Uint8 Bits_pp;
	size_t image_data_size;
	int ncolors;
} surface_header;

//PROTOTYPES
SDL_Surface* load_base_image(char*, char*);
SDL_bool save_image_data(SDL_Surface*, const char*);
SDL_Surface* load_image_data(char*, Uint8*);
void show_current_image(SDL_Surface*);
void program_menu(void);

//void test_data_load(char*); // function for reference stored at Vestigial
//void bin_file_name_manual_entry(char*);
//void clear_current_surface(SDL_Surface*, Uint8*, SDL_bool);

#endif // _SDLSURFPROG_H
