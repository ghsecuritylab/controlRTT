#include "flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "string.h"
#include <rthw.h>
#include <rtthread.h>
#include <stm32f4xx_hal.h>

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
#if STM32_FLASH_SIZE < 256
  #define STM_SECTOR_SIZE  128 //�ֽ�
#else 
  #define STM_SECTOR_SIZE	 2048
#endif


/* ˽�б��� ------------------------------------------------------------------*/
#if STM32_FLASH_WREN	//���ʹ����д 
static FLASH_TYPE STMFLASH_BUF [ STM_SECTOR_SIZE / FLASH_ALINE +1];//�����2K�ֽ�
#endif

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/

/**
  * ��������: ��ȡָ����ַ�İ���(16λ����)
  * �������: faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
  * �� �� ֵ: ����ֵ:��Ӧ����.
  * ˵    ������
  */
FLASH_TYPE memCTOI(char *data) {FLASH_TYPE ppp; memcpy(&ppp,data,FLASH_ALINE); return ppp;}	
HAL_StatusTypeDef STMFLASH_WriteStruct(uint32_t WriteAddr, void * pBuffer, uint16_t NumToWrite)
{
	int i = 0;
	uint32_t numData = 0;
	FLASH_TYPE Buffer[STM_SECTOR_SIZE / FLASH_ALINE];	
	for(i = 0; i < NumToWrite/FLASH_ALINE ;i ++)
	{
		Buffer[i] = memCTOI((char *)pBuffer);//*data;	
		pBuffer = ((char *)pBuffer+FLASH_ALINE);
		numData ++;		
	}	
		
	return STMFLASH_Write(WriteAddr,Buffer,numData);	
}	

void STMFLASH_ReadStruct(uint32_t ReadAddr, void * pBuffer, uint16_t NumToRead)
{
	FLASH_TYPE Buffer[STM_SECTOR_SIZE/FLASH_ALINE] = {0};
	#if FLASH_ALINE == 2
		STMFLASH_Read(ReadAddr,Buffer,NumToRead/FLASH_ALINE); //һ�������ֽ�
	#else
		STMFLASH_Read_AllWord(ReadAddr,Buffer,NumToRead/FLASH_ALINE); //һ�������ֽ�
	#endif
	memcpy(pBuffer,Buffer,NumToRead);
}	

HAL_StatusTypeDef STMFLASH_writeString(uint32_t WriteAddr, void * pBuffer, uint16_t NumToWrite)
{
	int i = 0;
	uint32_t numData = 0;
	FLASH_TYPE Buffer[STM_SECTOR_SIZE / FLASH_ALINE];	
//	DEBUG_ERROR("define\r\n");
//	HAL_Delay(1000);
	for(i = 0; i < NumToWrite/FLASH_ALINE ;i ++)
	{
		Buffer[i] = memCTOI((char *)pBuffer);//*data; 
		pBuffer = ((char *)pBuffer+FLASH_ALINE);
		numData ++; 	
	} 
	return STMFLASH_Write(WriteAddr,Buffer,numData);	
} 

void STMFLASH_ReadString(uint32_t ReadAddr, char* pBuffer, uint16_t NumToRead)
{
	FLASH_TYPE Buffer[STM_SECTOR_SIZE/FLASH_ALINE] = {0};
	#if FLASH_ALINE == 2
		STMFLASH_Read(ReadAddr,Buffer,NumToRead/FLASH_ALINE); //һ�������ֽ�
	#else
		STMFLASH_Read_AllWord(ReadAddr,Buffer,NumToRead/FLASH_ALINE); //һ���ĸ��ֽ�
	#endif
	memcpy(pBuffer,Buffer,NumToRead);

}	
/**
  * ��������: ��ȡָ����ַ�İ���(32λ����)
  * �������: faddr:����ַ(�˵�ַ����Ϊ4�ı���!!)
  * �� �� ֵ: ����ֵ:��Ӧ����.
  * ˵    ������
  */
uint16_t STMFLASH_ReadHalfWord ( uint16_t faddr )
{
	return *(__IO uint16_t*)faddr;
}

uint32_t STMFLASH_ReadAllWord ( uint32_t faddr )
{
	return *(__IO uint32_t*)faddr;
}
#if STM32_FLASH_WREN	//���ʹ����д   
/**
  * ��������: ������д��
  * �������: WriteAddr:��ʼ��ַ
  *           pBuffer:����ָ��
  *           NumToWrite:����(16λ)��
  * �� �� ֵ: ��
  * ˵    ������
  */
HAL_StatusTypeDef STMFLASH_Write_NoCheck ( uint32_t WriteAddr, FLASH_TYPE * pBuffer, uint32_t NumToWrite )   
{
	uint32_t i;	
	uint32_t *pbuff = (uint32_t *)pBuffer;
	for(i=0;i<NumToWrite;i++)
	{		
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,WriteAddr,pbuff[i]) == HAL_ERROR)
		{
			int j = 0 ;
			for(j = 0 ; j < 10; j ++)
			{
				if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,WriteAddr,pbuff[i]) == HAL_OK)
				{
					break;
				}
			}
			if(j == 10)
			{
				return HAL_ERROR;
			}
			else
			{
				rt_kprintf("index write address : %x\r\n",WriteAddr);		
			}
		}
		WriteAddr+=4;
	}  
	return HAL_OK;
} 
/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;
  
  if (Addr < (FLASH_BASE + 0x100000))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / STM_SECTOR_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + 0x100000)) / STM_SECTOR_SIZE;
  }
  
  return page;
}

/**
  * ��������: ��ָ����ַ��ʼд��ָ�����ȵ�����
  * �������: WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
  *           pBuffer:����ָ��
  *           NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
  * �� �� ֵ: ��
  * ˵    ������
  */
HAL_StatusTypeDef STMFLASH_Write ( uint32_t WriteAddr, FLASH_TYPE * pBuffer, uint32_t NumToWrite )	
{	
//	uint32_t SECTORError = 0;
	uint32_t secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	uint32_t secremain; //������ʣ���ַ(16λ�ּ���)	   
	uint32_t i;    
	uint32_t secpos;	   //������ַ
	uint32_t offaddr;   //ȥ��0X08000000��ĵ�ַ
	
	if(WriteAddr<FLASH_BASE||(WriteAddr>=(FLASH_BASE+1024*STM32_FLASH_SIZE)))
		return HAL_ERROR;
	rt_kprintf("flash write num = %d\r\n",NumToWrite);
	HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR |FLASH_FLAG_OPERR |FLASH_FLAG_PGPERR |FLASH_FLAG_PGSERR);	
	//FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	offaddr=WriteAddr-FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/FLASH_ALINE;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/FLASH_ALINE-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)
		secremain=NumToWrite;//�����ڸ�������Χ	
	while(1)
	{
		#if FLASH_ALINE == 2		
			STMFLASH_Read(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/FLASH_ALINE);//������������������
//			printf("read data for half word\r\n");
			HAL_Delay(2000);
		#else
			STMFLASH_Read_AllWord(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/FLASH_ALINE);//������������������
		#endif
		uint32_t countData =  STM_SECTOR_SIZE/FLASH_ALINE;
		for(i=0;i<countData;i++)//У������
		{
			#if FLASH_ALINE == 2
				if(STMFLASH_BUF[i]!=0XFFFF)
			#else
				if(STMFLASH_BUF[i]!=0XFFFFFFFF)
			#endif
				break;
		}
		if(i<STM_SECTOR_SIZE/FLASH_ALINE)//��Ҫ����
		{
			/* Fill EraseInit structure*/
			uint32_t FirstPage = GetPage(WriteAddr);
			uint32_t NbOfPages = GetPage(WriteAddr + NumToWrite*4) - FirstPage + 1;
			FLASH_Erase_Sector(FirstPage*0x1000 + 0x08000000, FLASH_VOLTAGE_RANGE_3);
			for(i=0;i<secremain;i++)
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}			
			if(STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/FLASH_ALINE) == HAL_ERROR)
			{
				rt_kprintf("Write_NoCheck error:%x\r\n",HAL_FLASH_GetError());
				HAL_FLASH_Lock();
				goto WF;
				//return HAL_ERROR;
			}
		}
		else
		{
			rt_kprintf("start:%x,%x\r\n",WriteAddr,secremain);
			if(STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain) == HAL_ERROR)
			{
				rt_kprintf("Write_NoCheck error\r\n");
				HAL_FLASH_Lock();
				goto WF;
				//return HAL_ERROR;
			}
		}
		if(NumToWrite==secremain)
			break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
			pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain*FLASH_ALINE;	//д��ַƫ��	   
			NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/FLASH_ALINE))
				secremain=STM_SECTOR_SIZE/FLASH_ALINE;//��һ����������д����
			else 
				secremain=NumToWrite;//��һ����������д����
		}	 
	}
	
	if(HAL_FLASH_Lock() == HAL_ERROR)
	{
		rt_kprintf("HAL_FLASH_Lock error\r\n");
		goto WF;
		//return HAL_ERROR;
	}
	rt_kprintf("flash write success");
	return HAL_OK;
	WF:	
	HAL_FLASH_Unlock();		
	if(STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/FLASH_ALINE) == HAL_ERROR)
	{
		rt_kprintf("write error\r\n");
	}
	HAL_FLASH_Lock();
	return HAL_OK;
}
#endif

/**
  * ��������: ��ָ����ַ��ʼ����ָ�����ȵ�����
  * �������: ReadAddr:��ʼ��ַ
  *           pBuffer:����ָ��
  *           NumToRead:����(16λ)��
  * �� �� ֵ: ��
  * ˵    ������
  */
void STMFLASH_Read ( uint32_t ReadAddr, uint16_t *pBuffer, uint32_t NumToRead )
{
	uint16_t i;
	
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr += 2;//ƫ��2���ֽ�.	
	}
}


void STMFLASH_Read_AllWord( uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead )   	
{
	uint16_t i;
	
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i] = STMFLASH_ReadAllWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr += 4;//ƫ��4���ֽ�.	
	}
}

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/





