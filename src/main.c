#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "ven/stbi_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ven/stbi_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ven/stb_image.h"

// Converts a given image into one supported by minecraft maps
// Should:
/// Downscale to 128x128
/// Dither the resulting image to the colour pallette of the maps

// Bayer's Dither model - https://rayferric.xyz/posts/dithering-&-shaders/
#define BAYER_SIZE 4
const float BAYER_MATRIX[BAYER_SIZE * BAYER_SIZE] = {
	 0.0 / 16.0, 12.0 / 16.0,  3.0 / 16.0, 15.0 / 16.0,
	 8.0 / 16.0,  4.0 / 16.0, 11.0 / 16.0,  7.0 / 16.0,
	 2.0 / 16.0, 14.0 / 16.0,  1.0 / 16.0, 13.0 / 16.0,
	10.0 / 16.0,  6.0 / 16.0,  9.0 / 16.0,  5.0 / 16.0
};

float step(float edge, float x) {
	return (x < edge ? 0.0f : 1.0f);
}

float bayer_dither(float val, int x, int y, int q) {
	val *= (float)q;
	
	float stepbystep = floor(val); // val without the decimal place

	float delta = val - stepbystep; // delta now leaves us with only the decimal place

	int x_on_matrix = x % BAYER_SIZE;
	int y_on_matrix = y % BAYER_SIZE;
	float dither_multiplier = BAYER_MATRIX[y_on_matrix * BAYER_SIZE + x_on_matrix];

	return (stepbystep + step(dither_multiplier, delta)) / (float)q;
}

int main(int argc, char** argv) {
	const char* input_filepath = "in.png";
	const char* output_filepath = "out.png";
	
	if(argc > 1) {
		input_filepath = argv[1];
	}
	if(argc > 2) {
		output_filepath = argv[2];
	}

	// for now assume given image to be 128x128
	int input_width, input_height, input_channels;
	uint8_t* input_image_data = stbi_load(input_filepath, &input_width, &input_height, &input_channels, 4);

	if(input_image_data == NULL) {
		printf("Error: Input file doesn't exist.\n");
		return 1;
	}

	if(input_width != input_height) {
		printf("Warning: Aspect ratio of the input image isn't 1:1.\n");
	}

	int width = 128, 
		height = 128, 
		channels = 4;

	uint8_t* resized_image_data = malloc(width * height * channels);
	stbir_resize_uint8(
		input_image_data, input_width, input_height, input_channels*input_width,
		resized_image_data, width, height, channels*width, channels
	);

	uint8_t* output_image_data = malloc(width * height * channels);
	memset(output_image_data, 0, width * height * channels);

	// just dither
	int colour_count_per_channel = 5;

	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {

			float r = (float)resized_image_data[(y*width+x)*channels + 0] / 255.0f;
			float g = (float)resized_image_data[(y*width+x)*channels + 1] / 255.0f;
			float b = (float)resized_image_data[(y*width+x)*channels + 2] / 255.0f;

			r = bayer_dither(r, x, y, colour_count_per_channel);
			g = bayer_dither(g, x, y, colour_count_per_channel);
			b = bayer_dither(b, x, y, colour_count_per_channel);

			output_image_data[(y*width+x)*channels + 0] = fmin(fmax(r, 0.0f), 1.0f) * 255.0f;
			output_image_data[(y*width+x)*channels + 1] = fmin(fmax(g, 0.0f), 1.0f) * 255.0f;
			output_image_data[(y*width+x)*channels + 2] = fmin(fmax(b, 0.0f), 1.0f) * 255.0f;
			output_image_data[(y*width+x)*channels + 3] = 255;

		}
	}

	stbi_write_png(output_filepath, width, height, channels, output_image_data, width*channels);

	free(output_image_data);
	free(resized_image_data);
	free(input_image_data);

	return 0;
}