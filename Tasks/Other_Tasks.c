/** 
* @brief    ��������
* @details  This is the detail description.
* @author   Onion rain
* @date     2018.10.16
* @version  
* @par Copyright (c):
*       Onion rain
* @par ��־
*/

#include "Other_Tasks.h"
#include "cmsis_os.h"
#include "task.h"

void TestTask(void const * argument)
{
  for(;;)
  {
		osDelay(10);
  }
}



