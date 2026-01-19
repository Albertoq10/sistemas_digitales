/**
  ******************************************************************************
  * @file    Wifi/WiFi_HTTP_Server/src/main.c
  * @author  MCD Application Team
  * @brief   This file provides main program functions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdlib.h>
#include <string.h>
#include "proyecto_persianas.h"
#include "display_oled.h"
#include "ssd1306.h"
#include "i2c.h"
#include "ldr_adc.h"
#include "servomotores_pwm.h"
#include "stm32l4xx_hal_tim.h"

/* Private defines -----------------------------------------------------------*/
/* Update SSID and PASSWORD with own Access point settings */
//#define SSID     "iPhoneBETO"
//#define PASSWORD "12345678"


//casa
#define SSID     "MN2"
#define PASSWORD "B06916969"
#define PORT           80

#define TERMINAL_USE

#define WIFI_WRITE_TIMEOUT 4000
#define WIFI_READ_TIMEOUT  4000
#define SOCKET                 0


#ifdef  TERMINAL_USE
#define LOG(a) printf a
#else
#define LOG(a)
#endif


/* Private typedef------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined (TERMINAL_USE)
extern UART_HandleTypeDef hDiscoUart;
#endif /* TERMINAL_USE */

static  uint8_t http[1024];
static  uint8_t  IP_Addr[4];
static  int     LedState = 0;

/* Private function prototypes -----------------------------------------------*/
#if defined (TERMINAL_USE)
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
#endif /* TERMINAL_USE */

static void SystemClock_Config(void);
//cambio a funcion
static  WIFI_Status_t SendWebPage(uint8_t ledIsOn, uint8_t temperature, modos_persianas_t modo);
static  int wifi_server(void);
static  int wifi_start(void);
static  int wifi_connect(void);
static  bool WebServerProcess(void);
static int parse_int_field(const char *body, const char *key);



//fuuncion para no mandar basura en % de modo manual
static int parse_int_field(const char *body, const char *key)
{
  const char *p = strstr(body, key);
  if (!p) return -1;

  p += strlen(key); // salta "pos=" o "blind="

  char numbuf[8];
  int i = 0;

  while (*p && *p != '&' && *p != ' ' &&
         *p != '\r' && *p != '\n' &&
         i < (int)sizeof(numbuf) - 1)
  {
    numbuf[i++] = *p++;
  }
  numbuf[i] = '\0';

  if (i == 0)
	  return -1;
  return
     atoi(numbuf);
}




int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();//aqui va el de i2c
  PeriphCommonClock_Config();// adc no usa reloj de sistema, usa otro
  MX_I2C1_Init();// i2c para displa

  MX_ADC1_Init();

  HAL_ADC_MspInit(&hadc1);
  HAL_ADC_Start(&hadc1);
  MX_TIM2_Init();
  MX_TIM3_Init();
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

  //ADR D6 y D5 pwm
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);


   //preubas

/*

   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);

  //prueba para leer de adc
  //uint16_t v = HAL_ADC_GetValue(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
  	     {

  	       uint16_t v = HAL_ADC_GetValue(&hadc1);
  	       printf("ADC raw = %lu\r\n", v);
  	     }
//prueba pwm

  for (uint32_t i = 10; i <= 50; i++)
  {
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, i);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, i);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, i);
     // HAL_Delay(200);
  }
*/


  //HAL_I2C_MspInit(&hi2c1);

  ssd1306_Init(&hi2c1);

  //inicial display
  ssd1306_Fill(Black);
  ssd1306_SetCursor(0,0);
  ssd1306_WriteString("Proyecto Persianas", Font_7x10, White);
  ssd1306_SetCursor(0,11);
  ssd1306_WriteString("Sistemas Digitales IoT", Font_7x10, White);
  ssd1306_UpdateScreen(&hi2c1);


  /* Configure LED2 */
  BSP_LED_Init(LED2);

  /*Initialize Temperature sensor */
 // HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED);
  //HAL_ADC_Start(&AdcHandle) ;

  /* WIFI Web Server demonstration */
#if defined (TERMINAL_USE)
  /* Initialize all configured peripherals */
  hDiscoUart.Instance = DISCOVERY_COM1;
  hDiscoUart.Init.BaudRate = 115200;
  hDiscoUart.Init.WordLength = UART_WORDLENGTH_8B;
  hDiscoUart.Init.StopBits = UART_STOPBITS_1;
  hDiscoUart.Init.Parity = UART_PARITY_NONE;
  hDiscoUart.Init.Mode = UART_MODE_TX_RX;
  hDiscoUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hDiscoUart.Init.OverSampling = UART_OVERSAMPLING_16;
  hDiscoUart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hDiscoUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;


  BSP_COM_Init(COM1, &hDiscoUart);
  BSP_TSENSOR_Init();

  printf("****** WIFI Web Server demonstration****** \n\n");

#endif /* TERMINAL_USE */



//////////////////
  wifi_server();
}

/**
  * @brief  Send HTML page
  * @param  None
  * @retval None
  */


static int wifi_start(void)
{
  uint8_t  MAC_Addr[6];

 /*Initialize and use WIFI module */
  if(WIFI_Init() ==  WIFI_STATUS_OK)
  {
    LOG(("ES-WIFI Initialized.\n"));
    if(WIFI_GetMAC_Address(MAC_Addr, sizeof(MAC_Addr)) == WIFI_STATUS_OK)
    {
      LOG(("> eS-WiFi module MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
               MAC_Addr[0],
               MAC_Addr[1],
               MAC_Addr[2],
               MAC_Addr[3],
               MAC_Addr[4],
               MAC_Addr[5]));
    }
    else
    {
      LOG(("> ERROR : CANNOT get MAC address\n"));
      return -1;
    }
  }
  else
  {
    return -1;
  }
  return 0;
}


//intenta conectarse a red
int wifi_connect(void)
{

  wifi_start();

  LOG(("\nConnecting to %s , %s\n",SSID,PASSWORD));
  if( WIFI_Connect(SSID, PASSWORD, WIFI_ECN_WPA2_PSK) == WIFI_STATUS_OK)
  {
    if(WIFI_GetIP_Address(IP_Addr, sizeof(IP_Addr)) == WIFI_STATUS_OK)
    {
      LOG(("> es-wifi module connected: got IP Address : %d.%d.%d.%d\n",
               IP_Addr[0],
               IP_Addr[1],
               IP_Addr[2],
               IP_Addr[3]));
    }
    else
    {
		  LOG((" ERROR : es-wifi module CANNOT get IP address\n"));
      return -1;
    }
  }
  else
  {
		 LOG(("ERROR : es-wifi module NOT connected\n"));
     return -1;
  }
  return 0;
}


//aqui se mantiene vivo el sistema sin depender de GET y POST
int wifi_server(void)
{
  uint8_t temp;
  bool StopServer = false;

  LOG(("\nRunning HTML Server test\n"));
  if (wifi_connect()!=0)
	  return -1;//conexion


  if (WIFI_STATUS_OK!=WIFI_StartServer(SOCKET, WIFI_TCP_PROTOCOL, 1, "", PORT))
  {
    LOG(("ERROR: Cannot start server.\n"));
  }

  LOG(("Server is running and waiting for an HTTP  Client connection to %d.%d.%d.%d\n",IP_Addr[0],IP_Addr[1],IP_Addr[2],IP_Addr[3]));

  do
  {
    uint8_t RemoteIP[4];
    uint16_t RemotePort;

//esto se esta ejecutando aunque no haya conexion para mantener el sistema actualizdo, lo mantiene activo
    while (WIFI_STATUS_OK != WIFI_WaitServerConnection(SOCKET, 300 ,RemoteIP,sizeof(RemoteIP), &RemotePort))
    {
        LOG(("Waiting connection to  %d.%d.%d.%d\n",IP_Addr[0],IP_Addr[1],IP_Addr[2],IP_Addr[3]));
        modos_tarea();//para estar revisando en que estado esta
        temp = (int) BSP_TSENSOR_ReadTemp();
        OLED_StatusUpdate(&hi2c1, temp, leer_estado(), persiana_pct(1), persiana_pct(2));//
    }

    LOG(("Client connected %d.%d.%d.%d:%d\n",RemoteIP[0],RemoteIP[1],RemoteIP[2],RemoteIP[3],RemotePort));

    StopServer=WebServerProcess();

    if(WIFI_CloseServerConnection(SOCKET) != WIFI_STATUS_OK)
    {
      LOG(("ERROR: failed to close current Server connection\n"));
      return -1;
    }
  }
  while(StopServer == false);

  if (WIFI_STATUS_OK!=WIFI_StopServer(SOCKET))
  {
    LOG(("ERROR: Cannot stop server.\n"));
  }

  LOG(("Server is stop\n"));
  return 0;
}


//el la tarjeta es el servidor que procesa POST/ GET
//navegador, desde pc o movil
//Aqui se hace el GET y POST
//actualizaciones de pantalla y del estado
static bool WebServerProcess(void)
{
  uint8_t temp;
  uint16_t  respLen;
  static   uint8_t resp[1024];
  bool    stopserver=false;

  if (WIFI_STATUS_OK == WIFI_ReceiveData(SOCKET, resp, 1000, &respLen, WIFI_READ_TIMEOUT))
  {
   LOG(("get %d byte from server\n",respLen));

   if( respLen > 0)
   {
	   //actualiza
      if(strstr((char *)resp, "GET")) /* GET: put web page */
      {
       temp = (int) BSP_TSENSOR_ReadTemp();
        OLED_StatusUpdate(&hi2c1, temp, leer_estado(), persiana_pct(1), persiana_pct(2));//actualizar
        if(SendWebPage(LedState, temp, leer_estado()) != WIFI_STATUS_OK)
        {
          LOG(("> ERROR : Cannot send web page\n"));
        }
        else
        {
          LOG(("Send page after  GET command\n"));
        }
       }
       else if(strstr((char *)resp, "POST"))/* POST: received info */
       {
         LOG(("Post request\n"));

         if(strstr((char *)resp, "radio"))
         {
           if(strstr((char *)resp, "radio=0"))
           {
             LedState = 0;
             BSP_LED_Off(LED2);
           }
           else if(strstr((char *)resp, "radio=1"))
           {
             LedState = 1;
             BSP_LED_On(LED2);
           }
           temp = (int) BSP_TSENSOR_ReadTemp();
         }
         //lo que se manda a pagina y actualiza con los mismos datos
         if (strstr((char *)resp, "mode"))
         {
           if (strstr((char *)resp, "mode=auto")) {
             definir_modo(MODO_automatico);
             LOG(("Modo cambiado a AUTOMATICO\n"));
           }
           else if (strstr((char *)resp, "mode=manual")) {

        	 int blind = parse_int_field((char*)resp, "blind=");
        	 int pos   = parse_int_field((char*)resp, "pos=");


             if ((blind == 1 || blind == 2) && (pos >= 0 && pos <= 100)) {
               manual_set((uint8_t)blind, (uint8_t)pos);
               LOG(("MANUAL: persiana %d -> %d%%\n", blind, pos));
             } else {
               definir_modo(MODO_manual); // igual entra en manual
               LOG(("MANUAL: faltan/invalidos blind/pos\n"));
             }
           }

           else if (strstr((char *)resp, "mode=vac")) {
        	 definir_modo(MODO_vacaciones);
        	 vacaciones_start();
        	 LOG(("Modo cambiado a VACACIONES\n"));
           }
         }

         /////////////////////////////////////////

         if(strstr((char *)resp, "stop_server"))
         {
           if(strstr((char *)resp, "stop_server=0"))
           {
             stopserver = false;
           }
           else if(strstr((char *)resp, "stop_server=1"))
           {
             stopserver = true;
           }
         }
         temp = (int) BSP_TSENSOR_ReadTemp();
         OLED_StatusUpdate(&hi2c1, temp, leer_estado(), persiana_pct(1), persiana_pct(2));//
         if(SendWebPage(LedState, temp, leer_estado()) != WIFI_STATUS_OK)
         {
           LOG(("> ERROR : Cannot send web page\n"));
         }
         else
         {
           LOG(("Send Page after POST command\n"));
         }
       }
     }
  }
  else
  {
    LOG(("Client close connection\n"));
  }
  modos_tarea();
  uint8_t t = (uint8_t)BSP_TSENSOR_ReadTemp();
  OLED_StatusUpdate(&hi2c1, t, leer_estado(), persiana_pct(1), persiana_pct(2));
  return stopserver;

 }

/**
  * @brief  Send HTML page
  * @param  None
  * @retval None
  */

//cambio a funcion, parametros, para que pueda recibir el modo de las persianas
//aqui se enfoca en darle formato a la pagina web
static WIFI_Status_t SendWebPage(uint8_t ledIsOn, uint8_t temperature, modos_persianas_t modo)
{
  uint8_t  temp[50];
  uint16_t SentDataLength;
  WIFI_Status_t ret;

  /* construct web page content */
  strcpy((char *)http, (char *)"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n");
  strcat((char *)http, (char *)"<html>\r\n<body>\r\n");
  strcat((char *)http, (char *)"<title>STM32 Web Server</title>\r\n");
  strcat((char *)http, (char *)"<h2>Persianas : Web Server using Es-Wifi with STM32</h2>\r\n");
  strcat((char *)http, (char *)"<br /><hr>\r\n");


  strcat((char *)http, (char *)"<p><form method=\"POST\"><strong>Temp: <input type=\"text\" value=\"");
  sprintf((char *)temp, "%d", temperature);
  strcat((char *)http, (char *)temp);
  strcat((char *)http, (char *)"\"> <sup>O</sup>C");

  //seleccion de modo de las persianas
  strcat((char *)http, (char *)"<strong>:</strong><br>\r\n");

  if (modo == MODO_automatico)
   {
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"auto\" checked> Automatico<br>\r\n");
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"manual\"> Manual<br>\r\n");
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"vac\"> Vacaciones<br>\r\n");
  }
  else if (modo == MODO_manual)
  {
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"auto\"> Automatico<br>\r\n");
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"manual\" checked> Manual<br>\r\n");
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"vac\"> Vacaciones<br>\r\n");
  }
  else
  {
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"auto\"> Automatico<br>\r\n");
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"manual\"> Manual<br>\r\n");
    strcat((char *)http, (char *)"<input type=\"radio\" name=\"mode\" value=\"vac\" checked> Vacaciones<br>\r\n");
  }

  //para mostrar modo manual, simepre se esta mandando, por lo que siempre se actualiza cada vez que se seleeciona
  strcat((char *)http, (char *)"<br><strong>Manual:</strong><br>\r\n");

  strcat((char *)http, (char *)"Persiana: <select name=\"blind\">");
  strcat((char *)http, (char *)"<option value=\"1\">1</option>");
  strcat((char *)http, (char *)"<option value=\"2\">2</option>");
  strcat((char *)http, (char *)"</select><br>\r\n");

  strcat((char *)http, (char *)"Apertura (%): <input type=\"number\" name=\"pos\" min=\"0\" max=\"100\" step=\"1\" value=\"50\"><br>\r\n");

  strcat((char *)http, (char *)"</strong><p><input type=\"submit\"></form></span>");
  strcat((char *)http, (char *)"</body>\r\n</html>\r\n");

  ret = WIFI_SendData(0, (uint8_t *)http, strlen((char *)http), &SentDataLength, WIFI_WRITE_TIMEOUT);

  if((ret == WIFI_STATUS_OK) && (SentDataLength != strlen((char *)http)))
  {
    ret = WIFI_STATUS_ERROR;
  }

  return ret;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

#if defined (TERMINAL_USE)
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&hDiscoUart, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
#endif /* TERMINAL_USE */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif


/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
    case (GPIO_PIN_1):
    {
      SPI_WIFI_ISR();
      break;
    }
    default:
    {
      break;
    }
  }
}

/**
  * @brief  SPI3 line detection callback.
  * @param  None
  * @retval None
  */
void SPI3_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&hspi);
}


/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */


