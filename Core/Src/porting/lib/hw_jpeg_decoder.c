#include <assert.h>
#include <stdio.h>

#include "hw_jpeg_decoder.h"
#include "main.h"

static JPEG_HandleTypeDef JPEG_Handle = {0};
static DMA2D_HandleTypeDef DMA2D_Handle = {0};

/* LCD resolution */
#define LCD_X_SIZE ((uint32_t)(320))
#define LCD_Y_SIZE ((uint32_t)(240))

/* image coords and rendering */
static uint16_t xPos = 0;
static uint16_t yPos = 0;
static uint8_t ForegroundAlpha = 0xFF;

// Internal buffer between JPEG decoder & DMA2D
// The ideal frame buffer is for YCbCr 4:4:4 = W x H x 3
// considered size YCbCr 4:2:0 320x240 = 320 x 240 x 3/2

static JPEG_ConfTypeDef JPEG_info = {0};

static uint32_t FrameBufferAddress;
static uint32_t JPEGBufferAddress;
static uint32_t JPEGBufferSize;

/* Transfer to a LCD framebuffer ? */
static uint32_t FrameBufferMode = 0;

/* Bypass transfer ? */
static uint32_t disable_transfer = 0;

static void COPY_JpegOutInit();
static void COPY_JpegOut();

static uint32_t JPEG_DecodeInit(uint32_t JPEG_Buffer, uint32_t JPEG_Buffer_Size)
{
    JPEGBufferAddress = JPEG_Buffer;
    JPEGBufferSize = JPEG_Buffer_Size;

    JPEG_Handle.Instance = JPEG;
    return HAL_JPEG_Init(&JPEG_Handle);
}

uint32_t JPEG_DecodeDeInit()
{
    return HAL_JPEG_DeInit(&JPEG_Handle);
}

uint32_t JPEG_DecodeToFrameInit(uint32_t JPEG_Buffer, uint32_t JPEG_Buffer_Size)
{
    FrameBufferMode = 1;

    return JPEG_DecodeInit(JPEG_Buffer, JPEG_Buffer_Size);
}

uint32_t JPEG_DecodeToBufferInit(uint32_t JPEG_Buffer, uint32_t JPEG_Buffer_Size)
{
    FrameBufferMode = 0;

    return JPEG_DecodeInit(JPEG_Buffer, JPEG_Buffer_Size);
}

uint32_t JPEG_DecodeGetSize(uint32_t SrcAddress, uint32_t *width, uint32_t *height)
{
    uint32_t ret;
    disable_transfer = 1;

    ret = HAL_JPEG_Decode(&JPEG_Handle, (uint8_t *)SrcAddress, JPEGBufferSize, (uint8_t *)JPEGBufferAddress, JPEGBufferSize, HAL_MAX_DELAY);

    *width = JPEG_info.ImageWidth;
    *height = JPEG_info.ImageHeight;
    return ret;
}

uint32_t JPEG_DecodeToBuffer(uint32_t SrcAddress, uint32_t DestAddress, uint32_t *width, uint32_t *height, uint8_t luma_alpha)
{
    uint32_t ret;
    ForegroundAlpha = luma_alpha;
    
    disable_transfer = 0;
    ret = JPEG_DecodeToFrame(SrcAddress, DestAddress, 0, 0, luma_alpha);

    *width = JPEG_info.ImageWidth;
    *height = JPEG_info.ImageHeight;

    return ret;
}

uint32_t JPEG_DecodeToFrame(uint32_t SrcAddress, uint32_t DestAddress, uint16_t x, uint16_t y, uint8_t luma_alpha)
{

    FrameBufferAddress = DestAddress;
    ForegroundAlpha = luma_alpha;
    disable_transfer = 0;

    xPos = x;
    yPos = y;

    // replace src size by JPEG_Buffer_Size...
    // deliver wrong source size to reduce the number of input parameter
    return HAL_JPEG_Decode(&JPEG_Handle, (uint8_t *)SrcAddress, JPEGBufferSize, (uint8_t *)JPEGBufferAddress, JPEGBufferSize, HAL_MAX_DELAY);
}

void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hJPEG, uint8_t *pDataOut, uint32_t OutDataLength)
{
}

void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hJPEG)
{
    printf("JPEG error\n");
    assert(0);
    Error_Handler();
}

void HAL_JPEG_DecodeCpltCallback(JPEG_HandleTypeDef *hJPEG)
{
    if (disable_transfer == 0)
        COPY_JpegOut();

}

void HAL_JPEG_InfoReadyCallback(JPEG_HandleTypeDef *hJPEG, JPEG_ConfTypeDef *pInfo)
{
    if (HAL_OK != HAL_JPEG_GetInfo(hJPEG, &JPEG_info))
        assert(0);

    uint32_t ImgSize = JPEG_info.ImageWidth * JPEG_info.ImageHeight;

    if (JPEG_info.ChromaSubsampling == JPEG_420_SUBSAMPLING)
    {
        ImgSize = ImgSize * 3 / 2;
        //printf("JPEG %lux%lu 4:2:0\n", JPEG_info.ImageWidth, JPEG_info.ImageHeight);
    }

    if (JPEG_info.ChromaSubsampling == JPEG_422_SUBSAMPLING)
    {
        ImgSize = ImgSize * 2;
        //printf("JPEG %lux%lu 4:2:2\n", JPEG_info.ImageWidth, JPEG_info.ImageHeight);
    }

    if (JPEG_info.ChromaSubsampling == JPEG_444_SUBSAMPLING)
    {
        ImgSize = ImgSize * 3;
        //printf("JPEG %lux%lu 4:4:4\n", JPEG_info.ImageWidth, JPEG_info.ImageHeight);
    }
    
    if (ImgSize > JPEGBufferSize)
    {
        printf("JPEG %lux%lu TOO LARGE:%lu > %lu \n", JPEG_info.ImageWidth, JPEG_info.ImageHeight, ImgSize, JPEGBufferSize);
        assert(0);
    }

}

void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hJPEG, uint32_t NbDecodedData)
{
}

static uint32_t cssMode = DMA2D_CSS_420, inputLineOffset = 0;

static void COPY_JpegOutInit()
{
    DMA2D_Handle.Instance = DMA2D;

    /* DMA2D Initialization */

    if (JPEG_info.ChromaSubsampling == JPEG_420_SUBSAMPLING)
    {
        cssMode = DMA2D_CSS_420;

        inputLineOffset = JPEG_info.ImageWidth % 16;
        if (inputLineOffset != 0)
        {
            inputLineOffset = 16 - inputLineOffset;
        }
    }
    else if (JPEG_info.ChromaSubsampling == JPEG_444_SUBSAMPLING)
    {
        cssMode = DMA2D_NO_CSS;

        inputLineOffset = JPEG_info.ImageWidth % 8;
        if (inputLineOffset != 0)
        {
            inputLineOffset = 8 - inputLineOffset;
        }
    }
    else if (JPEG_info.ChromaSubsampling == JPEG_422_SUBSAMPLING)
    {
        cssMode = DMA2D_CSS_422;

        inputLineOffset = JPEG_info.ImageWidth % 16;
        if (inputLineOffset != 0)
        {
            inputLineOffset = 16 - inputLineOffset;
        }
    }
    
    /* Configure  DMA2D Mode, Color Mode and output offset */
    DMA2D_Handle.Init.Mode = DMA2D_M2M_BLEND_BG; //DMA2D_M2M_PFC;
    DMA2D_Handle.Init.ColorMode = DMA2D_OUTPUT_RGB565;

    if (FrameBufferMode == 1)
        DMA2D_Handle.Init.OutputOffset = LCD_X_SIZE - JPEG_info.ImageWidth;
    else
        DMA2D_Handle.Init.OutputOffset = 0;

    DMA2D_Handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA; /* No Output Alpha Inversion */
    DMA2D_Handle.Init.RedBlueSwap = DMA2D_RB_REGULAR;      /* No Output Red & Blue swap */
    DMA2D_Handle.Init.LineOffsetMode = DMA2D_LOM_PIXELS;

    DMA2D_Handle.XferCpltCallback = NULL;

    assert(HAL_OK == HAL_DMA2D_Init(&DMA2D_Handle));
}
static void COPY_JpegOutConfigLayers()
{

    /* DMAD2D Foreground Configuration */
    DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    DMA2D_Handle.LayerCfg[1].InputAlpha = ForegroundAlpha;
    DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_YCBCR;
    DMA2D_Handle.LayerCfg[1].ChromaSubSampling = cssMode;
    DMA2D_Handle.LayerCfg[1].InputOffset = inputLineOffset;

    DMA2D_Handle.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;      /* No ForeGround Red/Blue swap */
    DMA2D_Handle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */

    assert(HAL_OK == HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1));

    /* DMAD2D Background Configuration */
    DMA2D_Handle.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
    DMA2D_Handle.LayerCfg[0].InputAlpha = 0xFF000000;
    DMA2D_Handle.LayerCfg[0].InputColorMode = DMA2D_INPUT_A8;
    DMA2D_Handle.LayerCfg[0].InputOffset = 0;

    DMA2D_Handle.LayerCfg[0].RedBlueSwap = DMA2D_RB_REGULAR;      /* No Red/Blue swap */
    DMA2D_Handle.LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No Alpha inversion */

    assert(HAL_OK == HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 0));
}

static void COPY_JpegOut()
{
    static uint32_t destination = 0;

    COPY_JpegOutInit();
    COPY_JpegOutConfigLayers();

    /*copy the new decoded frame to the Frame buffer */
    /* considering RGB565 16bits data format */
    destination = (uint32_t)FrameBufferAddress + ((yPos * LCD_X_SIZE) + xPos) * 2;

    SCB_CleanDCache();
    
    /* start transfer */
    assert(HAL_OK == HAL_DMA2D_Start(&DMA2D_Handle, JPEGBufferAddress, destination, JPEG_info.ImageWidth, JPEG_info.ImageHeight));

    /* wait for the previous DMA2D transfer to ends */
    assert(HAL_OK == HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 50));

    SCB_CleanInvalidateDCache();

    HAL_DMA2D_DeInit(&DMA2D_Handle);
}
