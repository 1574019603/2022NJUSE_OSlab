
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_

typedef struct cursor_stack{
    unsigned int now_index;
    unsigned int length;
    unsigned int array[80 * 25];
}cSTACK;

typedef struct action_stack{
    int index;//当前的下标
    int ESCseperator;//记录/r的下标
    char ch[80*25];//储存每一步的字符
}actionSTACK;

/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;/* 当前光标位置 */
    unsigned int lastNormalCursor;//进入查找模式前的光标位置
    cSTACK* cursor_Stack;//记录光标位置
    actionSTACK actionStack;//用于回滚操作

}CONSOLE;

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */


#endif /* _ORANGES_CONSOLE_H_ */

