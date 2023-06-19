#include "stm32f10x.h"
#include "./ov7725/bsp_ov7725.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./led/bsp_led.h"   
#include "./usart/bsp_usart.h"
#include "./key/bsp_key.h"  
#include "./systick/bsp_SysTick.h"
#include "./bmp/bsp_bmp.h"
#include "ff.h"


extern uint8_t Ov7725_vsync;

unsigned int Task_Delay[NumOfTask]; 

extern OV7725_MODE_PARAM cam_mode;


//以下
static void LCD_Test(void);    
void Printf_Charater(void);

#define BUFFER_SIZE 100

char usartBuffer[BUFFER_SIZE];  // 存储从串口读取的数据
int bufferIndex = 0;            // 新增一个指针来存储当前接收到的数据的位置

//以上
FATFS fs;													/* FatFs文件系统对象 */
FRESULT res_sd;                /* 文件操作结果 */


/**
  * @brief  主函数
  * @param  无  
  * @retval 无
  */
	
int main(void) 	
{		
	
	float frame_count = 0;
	uint8_t retry = 0;

	/* 液晶初始化 */
	ILI9341_Init();
	ILI9341_GramScan ( 3 );
	
	LCD_SetFont(&Font8x16);
	LCD_SetColors(RED,BLACK);

  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */
	
	/********显示字符串示例*******/
  ILI9341_DispStringLine_EN(LINE(0),"Image Style Migration Project");

	USART_Config();
	LED_GPIO_Config();
	Key_GPIO_Config();
	SysTick_Init();
	
	/*挂载sd文件系统*/
	res_sd = f_mount(&fs,"0:",1);
	if(res_sd != FR_OK)
	{
		printf("\r\n请给开发板插入已格式化成fat格式的SD卡。\r\n");
	}
	
	printf("\r\n ** OV7725摄像头实时液晶显示例程** \r\n"); 
	
	/* ov7725 gpio 初始化 */
	OV7725_GPIO_Config();
	
	LED_BLUE;
	/* ov7725 寄存器默认配置初始化 */
	while(OV7725_Init() != SUCCESS)
	{
		retry++;
		if(retry>5)
		{
			printf("\r\n没有检测到OV7725摄像头\r\n");
			ILI9341_DispStringLine_EN(LINE(2),"No OV7725 module detected!");
			while(1);
		}
	}


	/*根据摄像头参数组配置模式*/
	OV7725_Special_Effect(cam_mode.effect);
	/*光照模式*/
	OV7725_Light_Mode(cam_mode.light_mode);
	/*饱和度*/
	OV7725_Color_Saturation(cam_mode.saturation);
	/*光照度*/
	OV7725_Brightness(cam_mode.brightness);
	/*对比度*/
	OV7725_Contrast(cam_mode.contrast);
	/*特殊效果*/
	OV7725_Special_Effect(cam_mode.effect);
	
	/*设置图像采样及模式大小*/
	OV7725_Window_Set(cam_mode.cam_sx,
														cam_mode.cam_sy,
														cam_mode.cam_width,
														cam_mode.cam_height,
														cam_mode.QVGA_VGA);

	/* 设置液晶扫描模式 */
	ILI9341_GramScan( cam_mode.lcd_scan );
	
	
	
	ILI9341_DispStringLine_EN(LINE(2),"OV7725 initialize success!");
	printf("\r\nOV7725摄像头初始化完成\r\n");
	
	Ov7725_vsync = 0;
	
	while(1)
	{
		/*接收到新图像进行显示*/
		if( Ov7725_vsync == 2 )
		{
			frame_count++;
			FIFO_PREPARE;  			/*FIFO准备*/					
			ImagDisp(cam_mode.lcd_sx,
								cam_mode.lcd_sy,
								cam_mode.cam_width,
								cam_mode.cam_height);			/*采集并显示*/
			
			Ov7725_vsync = 0;			
//			LED1_TOGGLE;

		}
		
		/*检测按键*/
		if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON  )
		{		

			LED_BLUE;

			/*设置图像采样及模式大小*/
			ILI9341_GramScan ( cam_mode.lcd_scan );			
			frame_count++;
      		FIFO_PREPARE;
      		Ov7725_vsync = 0;
			ImagESP(cam_mode.lcd_sx,
								cam_mode.lcd_sy,
								cam_mode.cam_width,
								cam_mode.cam_height);
		
			while(1){
				USART_ReadAndDisplay(USART1, usartBuffer, BUFFER_SIZE);
				Delay_ms(10000);
				break;	
			 }			
		}
		
	}
}

void Delay_ms(uint32_t ms)
{
    SysTick_Config(SystemCoreClock / 1000); // 
    volatile uint32_t delay_count = ms;
    while (delay_count != 0)
    {
        if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) // 
        {
            delay_count--;
        }
    }
    SysTick->CTRL = 0; // 
}

// 从串口接收像素数据流并显示
void USART_ReadAndDisplay(USART_TypeDef* USARTx, char* buffer, int buffer_size) {
    char c;
    while (bufferIndex < buffer_size - 1)
    {
        // Check if data is available to read
        if (USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) != RESET)
        {
			// Read the data from USART and store in the buffer
			c = USART_ReceiveData(USARTx);
			buffer[bufferIndex] = c;
			bufferIndex++;
		}
	}
		{
		uint16_t i, j; 
		uint16_t image_Data;
		
		ILI9341_OpenWindow(cam_mode.lcd_sx,
								cam_mode.lcd_sy,
								cam_mode.cam_width,
								cam_mode.cam_height);
		ILI9341_Write_Cmd ( CMD_SetPixel );	

		for(i = 0; i < cam_mode.cam_width; i++)
		{
			for(j = 0; j < cam_mode.cam_height; j++)
			{
				image_Data = (uint16_t)(buffer[i*cam_mode.cam_height*2+j*2+1] << 8) | (uint16_t)(buffer[i*cam_mode.cam_height*2+j*2]);
				ILI9341_Write_Data(image_Data);
			}
		}
		}
		bufferIndex = 0;
}




/*********************************************END OF FILE**********************/

