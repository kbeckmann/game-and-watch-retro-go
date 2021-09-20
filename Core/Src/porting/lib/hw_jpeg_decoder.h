#ifndef __HW_JPEG_DECODER_H
#define __HW_JPEG_DECODER_H

/* Buffer mode (linear memory) */
uint32_t JPEG_DecodeToBufferInit(uint32_t JPEG_Buffer, uint32_t JPEG_Buffer_Size);
uint32_t JPEG_DecodeToBuffer(uint32_t SrcAddress, uint32_t DestAddress, uint32_t* width, uint32_t* height, uint8_t luma_alpha);

/* LCD Frame buffer mode */
// x,y are coordinates in the frame
uint32_t JPEG_DecodeToFrameInit(uint32_t JPEG_Buffer, uint32_t JPEG_Buffer_Size);
uint32_t JPEG_DecodeToFrame(uint32_t SrcAddress, uint32_t DestAddress, uint16_t x, uint16_t y, uint8_t luma_alpha);

/* Get JPEG Image size */
uint32_t JPEG_DecodeGetSize (uint32_t SrcAddress,uint32_t* width, uint32_t* height);

uint32_t JPEG_DecodeDeInit();

#endif /* __HW_JPEG_DECODER_H */
