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


//����
static void LCD_Test(void);    
void Printf_Charater(void);

#define BUFFER_SIZE 100

char usartBuffer[BUFFER_SIZE];  // �洢�Ӵ��ڶ�ȡ������
int bufferIndex = 0;            // ����һ��ָ�����洢��ǰ���յ������ݵ�λ��

//����
FATFS fs;													/* FatFs�ļ�ϵͳ���� */
FRESULT res_sd;                /* �ļ�������� */


/**
  * @brief  ������
  * @param  ��  
  * @retval ��
  */
	
int main(void) 	
{		
	
	float frame_count = 0;
	uint8_t retry = 0;

	/* Һ����ʼ�� */
	ILI9341_Init();
	ILI9341_GramScan ( 3 );
	
	LCD_SetFont(&Font8x16);
	LCD_SetColors(RED,BLACK);

  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */
	
	/********��ʾ�ַ���ʾ��*******/
  ILI9341_DispStringLine_EN(LINE(0),"Image Style Migration Project");

	USART_Config();
	LED_GPIO_Config();
	Key_GPIO_Config();
	SysTick_Init();
	
	/*����sd�ļ�ϵͳ*/
	res_sd = f_mount(&fs,"0:",1);
	if(res_sd != FR_OK)
	{
		printf("\r\n�������������Ѹ�ʽ����fat��ʽ��SD����\r\n");
	}
	
	printf("\r\n ** OV7725����ͷʵʱҺ����ʾ����** \r\n"); 
	
	/* ov7725 gpio ��ʼ�� */
	OV7725_GPIO_Config();
	
	LED_BLUE;
	/* ov7725 �Ĵ���Ĭ�����ó�ʼ�� */
	while(OV7725_Init() != SUCCESS)
	{
		retry++;
		if(retry>5)
		{
			printf("\r\nû�м�⵽OV7725����ͷ\r\n");
			ILI9341_DispStringLine_EN(LINE(2),"No OV7725 module detected!");
			while(1);
		}
	}


	/*��������ͷ����������ģʽ*/
	OV7725_Special_Effect(cam_mode.effect);
	/*����ģʽ*/
	OV7725_Light_Mode(cam_mode.light_mode);
	/*���Ͷ�*/
	OV7725_Color_Saturation(cam_mode.saturation);
	/*���ն�*/
	OV7725_Brightness(cam_mode.brightness);
	/*�Աȶ�*/
	OV7725_Contrast(cam_mode.contrast);
	/*����Ч��*/
	OV7725_Special_Effect(cam_mode.effect);
	
	/*����ͼ�������ģʽ��С*/
	OV7725_Window_Set(cam_mode.cam_sx,
														cam_mode.cam_sy,
														cam_mode.cam_width,
														cam_mode.cam_height,
														cam_mode.QVGA_VGA);

	/* ����Һ��ɨ��ģʽ */
	ILI9341_GramScan( cam_mode.lcd_scan );
	
	
	
	ILI9341_DispStringLine_EN(LINE(2),"OV7725 initialize success!");
	printf("\r\nOV7725����ͷ��ʼ�����\r\n");
	
	Ov7725_vsync = 0;
	
	while(1)
	{
		/*���յ���ͼ�������ʾ*/
		if( Ov7725_vsync == 2 )
		{
			frame_count++;
			FIFO_PREPARE;  			/*FIFO׼��*/					
			ImagDisp(cam_mode.lcd_sx,
								cam_mode.lcd_sy,
								cam_mode.cam_width,
								cam_mode.cam_height);			/*�ɼ�����ʾ*/
			
			Ov7725_vsync = 0;			
//			LED1_TOGGLE;

		}
		
		/*��ⰴ��*/
		if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON  )
		{		

			LED_BLUE;

			/*����ͼ�������ģʽ��С*/
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

// �Ӵ��ڽ�����������������ʾ
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

