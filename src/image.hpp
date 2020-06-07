#pragma once

#include <stb_image.h>

#include <string>
#include <memory>

class Image
{

public :
	int width;
	int height;
	int nrChannels;
	unsigned char* data;

	Image(std::string filename)
	{
		stbi_set_flip_vertically_on_load(true);
		data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
	}

	Image(std::string filename, bool flip)
	{
		stbi_set_flip_vertically_on_load(flip);
		data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
	}

	Image() = delete;
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&&) = default;
	Image& operator=(Image&&) = default;

	~Image()
	{
		stbi_image_free(data);
	}
};