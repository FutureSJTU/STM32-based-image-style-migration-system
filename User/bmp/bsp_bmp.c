/**
  ******************************************************************************
  * @file    bsp_key.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   bmp�ļ���������ʾbmpͼƬ�͸�Һ����ͼ
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "ff.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./bmp/bsp_bmp.h"

#define RGB24TORGB16(R,G,B) ((unsigned short int)((((R)>>3)<<11) | (((G)>>2)<<5)	| ((B)>>3)))

BYTE pColorData[960];			/* һ�����ɫ���ݻ��� 320 * 3 = 960 */
FIL bmpfsrc, bmpfdst; 
FRESULT bmpres;

/* �������Ҫ��ӡbmp��ص���ʾ��Ϣ,��printfע�͵�����
 * ��Ҫ��printf()���轫���������ļ���������
 */
#define BMP_DEBUG_PRINTF(FORMAT,...)  printf(FORMAT,##__VA_ARGS__)	 

/* ��ӡBMP�ļ���ͷ��Ϣ�����ڵ��� */
static void showBmpHead(BITMAPFILEHEADER* pBmpHead)
{
    BMP_DEBUG_PRINTF("λͼ�ļ�ͷ:\r\n");
    BMP_DEBUG_PRINTF("�ļ���С:%ld\r\n",(*pBmpHead).bfSize);
    BMP_DEBUG_PRINTF("������:%d\r\n",(*pBmpHead).bfReserved1);
    BMP_DEBUG_PRINTF("������:%d\r\n",(*pBmpHead).bfReserved2);
    BMP_DEBUG_PRINTF("ʵ��λͼ���ݵ�ƫ���ֽ���:%ld\r\n",(*pBmpHead).bfOffBits);
		BMP_DEBUG_PRINTF("\r\n");	
}

/* ��ӡBMP�ļ���ͷ��Ϣ�����ڵ��� */
static void showBmpInforHead(tagBITMAPINFOHEADER* pBmpInforHead)
{
    BMP_DEBUG_PRINTF("λͼ��Ϣͷ:\r\n");
    BMP_DEBUG_PRINTF("�ṹ��ĳ���:%ld\r\n",(*pBmpInforHead).biSize);
    BMP_DEBUG_PRINTF("λͼ��:%ld\r\n",(*pBmpInforHead).biWidth);
    BMP_DEBUG_PRINTF("λͼ��:%ld\r\n",(*pBmpInforHead).biHeight);
    BMP_DEBUG_PRINTF("biPlanesƽ����:%d\r\n",(*pBmpInforHead).biPlanes);
    BMP_DEBUG_PRINTF("biBitCount������ɫλ��:%d\r\n",(*pBmpInforHead).biBitCount);
    BMP_DEBUG_PRINTF("ѹ����ʽ:%ld\r\n",(*pBmpInforHead).biCompression);
    BMP_DEBUG_PRINTF("biSizeImageʵ��λͼ����ռ�õ��ֽ���:%ld\r\n",(*pBmpInforHead).biSizeImage);
    BMP_DEBUG_PRINTF("X����ֱ���:%ld\r\n",(*pBmpInforHead).biXPelsPerMeter);
    BMP_DEBUG_PRINTF("Y����ֱ���:%ld\r\n",(*pBmpInforHead).biYPelsPerMeter);
    BMP_DEBUG_PRINTF("ʹ�õ���ɫ��:%ld\r\n",(*pBmpInforHead).biClrUsed);
    BMP_DEBUG_PRINTF("��Ҫ��ɫ��:%ld\r\n",(*pBmpInforHead).biClrImportant);
		BMP_DEBUG_PRINTF("\r\n");
}





/**
 * @brief  ����ILI9341�Ľ�ȡBMPͼƬ
 * @param  x ����ɨ��ģʽ1�½�ȡ��������X���� 
 * @param  y ����ɨ��ģʽ1�½�ȡ��������Y���� 
 * @param  pic_name ��BMP��ŵ�ȫ·��
 * @retval ��
 */
void LCD_Show_BMP ( uint16_t x, uint16_t y, char * pic_name )
{
	int i, j, k;
	int width, height, l_width;

	BYTE red,green,blue;
	BITMAPFILEHEADER bitHead;
	BITMAPINFOHEADER bitInfoHead;
	WORD fileType;

	unsigned int read_num;

	bmpres = f_open( &bmpfsrc , (char *)pic_name, FA_OPEN_EXISTING | FA_READ);	
/*-------------------------------------------------------------------------------------------------------*/
	if(bmpres == FR_OK)
	{
		BMP_DEBUG_PRINTF("���ļ��ɹ�\r\n");

		/* ��ȡ�ļ�ͷ��Ϣ  �����ֽ�*/         
		f_read(&bmpfsrc,&fileType,sizeof(WORD),&read_num);     

		/* �ж��ǲ���bmp�ļ� "BM"*/
		if(fileType != 0x4d42)
		{
			BMP_DEBUG_PRINTF("�ⲻ��һ�� .bmp �ļ�!\r\n");
			return;
		}
		else
		{
			BMP_DEBUG_PRINTF("����һ�� .bmp �ļ�\r\n");	
		}        

		/* ��ȡBMP�ļ�ͷ��Ϣ*/
		f_read(&bmpfsrc,&bitHead,sizeof(tagBITMAPFILEHEADER),&read_num);        
		showBmpHead(&bitHead);

		/* ��ȡλͼ��Ϣͷ��Ϣ */
		f_read(&bmpfsrc,&bitInfoHead,sizeof(BITMAPINFOHEADER),&read_num);        
		showBmpInforHead(&bitInfoHead);
	}    
	else
	{
		BMP_DEBUG_PRINTF("���ļ�ʧ��!������룺bmpres = %d \r\n",bmpres);
		return;
	}    
/*-------------------------------------------------------------------------------------------------------*/
	width = bitInfoHead.biWidth;
	height = bitInfoHead.biHeight;

	/* ����λͼ��ʵ�ʿ��Ȳ�ȷ����Ϊ32�ı���	*/
	l_width = WIDTHBYTES(width* bitInfoHead.biBitCount);	

	if(l_width > 960)
	{
		BMP_DEBUG_PRINTF("\n ��ͼƬ̫���޷���Һ��������ʾ (<=320)\n");
		return;
	}
	
	
	/* ��һ��ͼƬ��С�Ĵ���*/
	ILI9341_OpenWindow(x, y, width, height);
	ILI9341_Write_Cmd (CMD_SetPixel ); 

	
	/* �ж��Ƿ���24bit���ɫͼ */
	if( bitInfoHead.biBitCount >= 24 )
	{
		for ( i = 0; i < height; i ++ )
		{
			/*���ļ��ĺ������BMP�ļ���ԭʼͼ����Ϊ���½ǵ����Ͻ�*/
      f_lseek ( & bmpfsrc, bitHead .bfOffBits + ( height - i - 1 ) * l_width );	
			
			/* ��ȡһ��bmp�����ݵ�����pColorData���� */
			f_read ( & bmpfsrc, pColorData, l_width, & read_num );				

			for(j=0; j<width; j++) 											   //һ����Ч��Ϣ
			{
				k = j*3;																	 //һ���е�K�����ص����
				red = pColorData[k+2];
				green = pColorData[k+1];
				blue = 	pColorData[k];
				ILI9341_Write_Data ( RGB24TORGB16 ( red, green, blue ) ); //д��LCD-GRAM
			}            			
		}        		
	}    	
	else 
	{        
		BMP_DEBUG_PRINTF("�ⲻ��һ��24λ���ɫBMP�ļ���");
		return ;
	}
	
	f_close(&bmpfsrc);  
  
}




/**
 * @brief  ����ILI9341�Ľ�ȡBMPͼƬ
 * @param  x ����ȡ��������X���� 
 * @param  y ����ȡ��������Y���� 
 * @param  Width ���������
 * @param  Height ������߶� 
 * @retval ��
  *   �ò���Ϊ����ֵ֮һ��
  *     @arg 0 :��ͼ�ɹ�
  *     @arg -1 :��ͼʧ��
 */
int Screen_Shot( uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, char * filename)
{
	/* bmp  �ļ�ͷ 54���ֽ� */
	unsigned char header[54] =
	{
		0x42, 0x4d, 0, 0, 0, 0, 
		0, 0, 0, 0, 54, 0, 
		0, 0, 40,0, 0, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 1, 0, 24, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 
		0, 0, 0
	};
	
	int i;
	int j;
	long file_size;     
	long width;
	long height;
	unsigned char r,g,b;	
	unsigned int mybw;
	unsigned int read_data;
	char kk[4]={0,0,0,0};
	
	uint8_t ucAlign;//
	
	
	/* ��*�� +������ֽ� + ͷ����Ϣ */
	file_size = (long)Width * (long)Height * 3 + Height*(Width%4) + 54;		

	/* �ļ���С 4���ֽ� */
	header[2] = (unsigned char)(file_size &0x000000ff);
	header[3] = (file_size >> 8) & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	width=Width;	
	header[18] = width & 0x000000ff;
	header[19] = (width >> 8) &0x000000ff;
	header[20] = (width >> 16) &0x000000ff;
	header[21] = (width >> 24) &0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	height = Height;
	header[22] = height &0x000000ff;
	header[23] = (height >> 8) &0x000000ff;
	header[24] = (height >> 16) &0x000000ff;
	header[25] = (height >> 24) &0x000000ff;
		
	/* �½�һ���ļ� */
	bmpres = f_open( &bmpfsrc , (char*)filename, FA_CREATE_ALWAYS | FA_WRITE );
	
	/* �½��ļ�֮��Ҫ�ȹر��ٴ򿪲���д�� */
	f_close(&bmpfsrc);
		
	bmpres = f_open( &bmpfsrc , (char*)filename,  FA_OPEN_EXISTING | FA_WRITE);

	if ( bmpres == FR_OK )
	{    
		/* ��Ԥ�ȶ���õ�bmpͷ����Ϣд���ļ����� */
		bmpres = f_write(&bmpfsrc, header,sizeof(unsigned char)*54, &mybw);		
			
		ucAlign = Width % 4;
		
		for(i=0; i<Height; i++)					
		{
			for(j=0; j<Width; j++)  
			{					
				read_data = ILI9341_GetPointPixel ( x + j, y + Height - 1 - i );					
				
				r =  GETR_FROM_RGB16(read_data);
				g =  GETG_FROM_RGB16(read_data);
				b =  GETB_FROM_RGB16(read_data);

				bmpres = f_write(&bmpfsrc, &b,sizeof(unsigned char), &mybw);
				bmpres = f_write(&bmpfsrc, &g,sizeof(unsigned char), &mybw);
				bmpres = f_write(&bmpfsrc, &r,sizeof(unsigned char), &mybw);
			}
				
			if( ucAlign )				/* �������4�ֽڶ��� */
				bmpres = f_write ( & bmpfsrc, kk, sizeof(unsigned char) * ( ucAlign ), & mybw );

		}/* ������� */

		f_close(&bmpfsrc); 
		
		return 0;
		
	}	
	else/* ����ʧ�� */
		return -1;

}
