/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "Render/Image.h"
#include "Render/Texture.h"

namespace DAVA 
{
    

Image::Image()
:	data(0)
,   dataSize(0)
,	width(0)
,	height(0)
,	format(FORMAT_RGB565)
{
}

Image::~Image()
{
	SafeDeleteArray(data);
}

Image * Image::Create(uint32 width, uint32 height, PixelFormat format)
{
	Image * image = new Image();
	image->width = width;
	image->height = height;
	image->format = format;
    
    int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);
    if (formatSize ||
		(format >= FORMAT_DXT1 && format <= FORMAT_DXT1A) ||
		(format >= FORMAT_ATC_RGB && format <= FORMAT_ATC_RGBA_INTERPOLATED_ALPHA))
    {
		//workaround, because formatSize is not designed for formats with 4 bits per pixel
		image->dataSize = width * height * formatSize;
		
		if ((format >= FORMAT_DXT1 && format <= FORMAT_DXT5NM) ||
			(format >= FORMAT_ATC_RGB && format <= FORMAT_ATC_RGBA_INTERPOLATED_ALPHA))
		{
			uint32 dSize = formatSize == 0 ? (width * height) / 2 : width * height ;
			if(width < 4 || height < 4)// size lower than  block's size
			{
				uint32 minvalue = width < height ? width : height;
				uint32 maxvalue = width > height ? width : height;
				minvalue = minvalue < 4 ? 4 : minvalue;
				maxvalue = maxvalue < 4 ? 4 : maxvalue;
				dSize = Texture::GetPixelFormatSizeInBits(format) * minvalue * maxvalue;
				dSize /= 8;
			}
			image->dataSize = dSize;
		}
        
        image->data = new uint8[image->dataSize];
    }
    else 
    {
        Logger::Error("Image::Create trying to create image with wrong format");
    }
    
	return image;
}

void Image::ResizeImage(uint32 newWidth, uint32 newHeight)
{
	uint8 * newData = NULL;
	int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);

	if(formatSize>0)
	{
		newData = new uint8[newWidth * newHeight * formatSize];
		memset(newData, 0, newWidth * newHeight * formatSize);

		float32 kx = (float32)width / (float32)newWidth;
		float32 ky = (float32)height / (float32)newHeight;

		float32 xx = 0, yy = 0;
		uint32 offset = 0;
		uint32 offsetOld = 0;
		uint32 posX, posY;
		for (uint32 y = 0; y < newHeight; ++y)
		{
			for (uint32 x = 0; x < newWidth; ++x)
			{
				posX = (uint32)(xx + 0.5f);
				posY = (uint32)(yy + 0.5f);
				if (posX >= width)
					posX = width - 1;

				if (posY >= height)
					posY = height - 1;


				offsetOld = (posY * width + posX) * formatSize;
				memcpy(newData + offset, data + offsetOld, formatSize);

				xx += kx;
				offset += formatSize;
			}
			yy += ky;
			xx = 0;
		}

		// resized data
		width = newWidth;
		height = newHeight;
		SafeDeleteArray(data);
		data = newData;
	}
}

void Image::ResizeCanvas(uint32 newWidth, uint32 newHeight)
{
    uint8 * newData = NULL;
    uint32 newDataSize = 0;
    int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);
        
    if(formatSize>0)
    {
        newDataSize = newWidth * newHeight * formatSize;
        newData = new uint8[newDataSize];
            
        uint32 currentLine = 0;
        uint32 indexOnLine = 0;
        uint32 indexInOldData = 0;
            
        for(uint32 i = 0; i < newDataSize; ++i)
        {
            if((currentLine+1)*newWidth*formatSize<=i)
            {
                currentLine++;
            }
                
            indexOnLine = i - currentLine*newWidth*formatSize;
                
            if(currentLine<(uint32)height)
            {
                // within height of old image
                if(indexOnLine<(uint32)(width*formatSize))
                {
                    // we have data in old image for new image
                    indexInOldData = currentLine*width*formatSize + indexOnLine;
                    newData[i] = data[indexInOldData];
                }
                else
                {
                    newData[i] = 0;
                }
            }
            else
            {
                newData[i] = 0;
            }
        }
            
        // resized data
        width = newWidth;
        height = newHeight;
            
        SafeDeleteArray(data);
        data = newData;
        dataSize = newDataSize;
    }
}

void Image::ResizeToSquare()
{
    float32 newImageSize = Max(width, height);
    ResizeCanvas(newImageSize, newImageSize);
}

};
