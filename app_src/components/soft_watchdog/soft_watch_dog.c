/*
 * soft_watch_dog.c
 *
 *  Created on: Nov 10, 2021
 *      Author: Ben
 */
#include "soft_watch_dog.h"
#include "watchdog.h"
#ifdef SOFT_WACTH_DOG_ENABLE

//С����
volatile static uint32_t g_little_dog_group_1 = 0;
//С��ע����Ϣ
volatile static uint32_t g_little_dog_adopt_mask = 0x2;


//ע��С��
void little_dog_adopt(DOG_INDEX dog_site)
{
	g_little_dog_adopt_mask |= dog_site;
}
//ע��С��
void little_dog_deadopt(DOG_INDEX dog_site)
{
	g_little_dog_adopt_mask &= ~dog_site;
}

//ιС��
void little_dog_feed(DOG_INDEX dog_num)//uint32_t* little_dog_group,
{
	g_little_dog_group_1 |= dog_num;
}

//���С����������
//DOG_MASK �����������dog site
bool big_dog_check(uint32_t dog_mask)//uint32_t* little_dog_group,
{
	if((g_little_dog_group_1 & dog_mask) != dog_mask)
	{
		//DBG("Miss Little WatchDog: %x\n",DOG_MASK);
		return 0;
	}else
	{
		g_little_dog_group_1 = 0x0;
		return 1;
	}
}

//ι��
//�����Ƿ��������ʱ���У�����֧��ĳЩ����ʱ������ι����
void big_dog_feed(void)
{
	//static uint32_t feed_freq_count = 0;//example1
	//feed_freq_count ++;
	if(!big_dog_check(g_little_dog_adopt_mask))
	{
		return;
	}
	//if((feed_freq_count%100 == 0)&&(!big_dog_check(&g_little_dog_group_2,0x0000FFFF)))
//	if(!big_dog_check(&g_little_dog_group_2,0x0000FFFF))
//	{
//		return;
//	}

	WDG_Feed();
}

#endif //SOFT_WACTH_DOG_ENABLE
