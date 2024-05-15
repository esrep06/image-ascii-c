// IMAGE TO ASCII ART IN C 
// PROCESS:
// 1) LOAD THE ORIGINAL IMAGE 
// 2) RESIZE THE IMAGE
// 3) CONVERT IT TO GRAYSCALE 
// 4) GET AVERAGE BRIGHTNESS OF EACH PIXEL AND MAP ASCII CHARACTERS TO THE BRIGHTNESS
// 5) PRINT THE CHARACTER THAT GOES WITH THE AVERAGE BRIGHTNESS 
// 6) ENJOY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "include/stb_image.h"
#include "include/stb_image_write.h"
#include "include/stb_image_resize.h"

#define VERSION 0.1

#define RED_WEIGHT 0.21
#define GREEN_WEIGHT 0.72
#define BLUE_WEIGHT 0.07

struct image
{
	unsigned char* data;
	char* path;
	int width, height, channels;
	size_t size;
};

int image_load(struct image* image)
{
	image->data = stbi_load(image->path, &image->width, &image->height, &image->channels, 0);
	image->size = image->width * image->height * image->channels;

	return (image->data != NULL);
}

int image_resize(struct image input, struct image* output)
{
	output->data = (unsigned char*)malloc(output->size);
	output->channels = input.channels;
	int res = stbir_resize_uint8(input.data, input.width, input.height, 0, output->data, output->width, output->height, 0, input.channels);

	if (!res) 
		printf("Failure resizing image\n");

	return res;
}

int image_to_gray_scale(struct image input, struct image* output)
{
	// Grayscale images only have one channel
	output->channels = 1;
	output->data = (unsigned char*)malloc(output->size);

	for (unsigned char *p = input.data, *pg = output->data; p != input.data + input.size; p += input.channels, pg += output->channels)
	{
		// We use different weigths to calculate the gray scale of each color 
		// We can also just get the average of each pixel but eyes dont see all red, green and blue colors the same 
		// So we use weights instead 
		*pg = (uint8_t)((*p * RED_WEIGHT) + (*(p + 1) * GREEN_WEIGHT) + (*(p + 2) * BLUE_WEIGHT)); 

		if (input.channels == 4)
			*(pg + 1) = *(p + 3);
	}

	output->width = input.width;
	output->height = input.height;

	if (!output->data)
	{
		printf("Failure converting image to grayscale");
		return 0;
	}

	return 1;
}

void image_to_ascii(struct image gray_image, int make_text)
{
	// We open the file if make_text is true
	FILE* file;

	if (make_text)
	{
		file = fopen("ascii.txt", "w");
		if (file == NULL) return;
	}

	// Ascii characters arranged from darkest to lightest
	char* gray_ramp = "^\",:;Il!i~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0oZmwqpdbkhao*#MW&8%B@S ";
	int ramp_length = strlen(gray_ramp);

	for (int y = 0; y < gray_image.height; y++)
	{
		for (int x = 0; x < gray_image.width; x++)
		{
			// We get the rgb values for the current pixel
			int current_pixel = y * gray_image.width + x;
			int r = (int)(gray_image.data[current_pixel]);
			int g = (int)(gray_image.data[current_pixel + 1]);
			int b = (int)(gray_image.data[current_pixel + 2]);

			// Get the average brightness of the pixel, this is why we convert it to grayscale
			float average = (r + g + b) / 3.0f;

			// Map the character with the brightness, this is simple division and multiplication
			char mapped_char = gray_ramp[(int)(ramp_length * (average / 255))];

			if (make_text) fputc(mapped_char, file);

			putchar(mapped_char);
		}

		if (make_text) fputc('\n', file);

		printf("\n");
	}

	if (make_text) fclose(file);
}

int main(int argc, char** argv)
{
	// we just parse arguments here
	
	char* help_message = "\n  cascii help\n"
						 "  --version : prints version\n"
						 "  image_path, width, height\n"
						 "  --file || --f : generates ascii.txt with ascii art"
						 "  --imdata : prits image data {width, height, channels}\n\n";

	if (argc < 2) { printf("%s", help_message); return 0; }

	if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v")){ printf("cascii version: %g\n", VERSION); }

	struct image original_image;
	original_image.path = argv[1];

	if (!image_load(&original_image)) { printf("%s", help_message); return 0; }

	if (argc > 2)
	{
		if (!strcmp(argv[2], "--imdata")) { 
			printf("%s: w:%d, h:%d, c:%d\n", original_image.path, original_image.width, original_image.height, original_image.channels); 
			return 0;
		}
	}

	if (argc < 4) { printf("%s", help_message); return 0; }

	// we can maintain aspect ratio this way
	int aspect_ratio = original_image.width / original_image.height;

	// this occurs with images that are longer than they are wider
	if (aspect_ratio == 0)
		aspect_ratio = 1;

	// Resolution of resized image
	int res_w = (int)(atoi(argv[2]));
	int res_h = (int)(atoi(argv[3]));

	// Create the resized image with the width and height selected by the user
	// The height is basically set with this logic
	// If the aspect_ratio is 1, we use the height specified by the user, if its not, we divide the resolution by the aspect ratio
	struct image resized_image = { .width = res_w, .height = aspect_ratio == 1 ? res_h : (int)(res_h / aspect_ratio)};
	resized_image.size = resized_image.width * resized_image.height * original_image.channels;

	if (!image_resize(original_image, &resized_image)) { printf("Failure resizing image:\n%s", help_message); return 0; }

	struct image gray_image;
	if (!image_to_gray_scale(resized_image, &gray_image)) return 0;
	
	int make_file = 0;

	// If there is a fifth argument then we check if its equals to --file or -f
	// If it is make file is then set to 1, so we create a file with the ascii art
	// If it isn't we stop and print help,
	// If there is no fifth argument we proceed without generating an ascii art file
	if (argc == 5)
	{
		// Returns 1 if its either of them
		make_file = !strcmp(argv[4], "--file") ||! strcmp(argv[4], "--f");
		if (!make_file) { printf("%s", help_message); return 0; }
	}


	// Finally print the data
	image_to_ascii(gray_image, make_file);

	// We free the data 
	free(gray_image.data);
	stbi_image_free(original_image.data);
	free(resized_image.data);

	return 0;
}

