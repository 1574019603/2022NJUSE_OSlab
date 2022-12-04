
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PRIVATE void push(CONSOLE* p_con, unsigned int index);
PRIVATE unsigned int pop(CONSOLE* p_con);

/*======================================================================*
			   init_screen
 *======================================================================*/
void search(CONSOLE *pConsole);

void rollback(CONSOLE *p_con);

PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;//当前终端的起始地址
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;
    p_tty->p_console->actionStack.index = 0;
    p_tty->p_console->actionStack.ESCseperator = 0;

    for (int i = 0; i < 80*25; ++i) {
        p_tty->p_console->actionStack.ch[i] = ' ';
    }

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;
    p_tty->p_console->cursor_Stack->now_index = 0;
    p_tty->p_console->lastNormalCursor = p_tty->p_console->cursor;
    p_tty->p_console->cursor_Stack->length = SCREEN_SIZE;


	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
//判断是否是当前的console
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    //屏蔽输入
    if(mode == 2){
        if(ch == '\r'){
            mode = 0;
        } else{
            return;
        }
    }

	switch(ch) {

        case '\r':
            if(mode == 1){
                p_con->lastNormalCursor = p_con->cursor;
            } else{
                if(p_con->cursor > p_con->original_addr){
                    int j = 0;
                    while (p_con->cursor > p_con->lastNormalCursor){
                        unsigned int index = pop(p_con);
                        while (p_con->cursor > index){
                            p_con->cursor--;
                            *(p_vmem-2 - j*2) = ' ';
                            *(p_vmem-1 - j*2) = DEFAULT_CHAR_COLOR;
                            j++;
                        }
                    }
                }
                for(int i=0;i<p_con->cursor;i++){
                    if(*(u8*)(V_MEM_BASE + i * 2+1)==RED){
                        *(u8*)(V_MEM_BASE + i * 2+1)=DEFAULT_CHAR_COLOR;
                    }
                }
            }
            break;
	case '\n':
        if(mode == 0){
            if (p_con->cursor < p_con->original_addr +
                                p_con->v_mem_limit - SCREEN_WIDTH) {
                push(p_con,p_con->cursor);
                p_con->cursor = p_con->original_addr + SCREEN_WIDTH *
                                                       ((p_con->cursor - p_con->original_addr) /
                                                        SCREEN_WIDTH + 1);
            }
        } else{
            mode = 2;
            search(p_con);
        }

		break;
	case '\b':
		if (p_con->cursor > p_con->original_addr) {
            unsigned int index = pop(p_con);
            int j = 0;
            while (p_con->cursor > index){
                p_con->cursor--;
                *(p_vmem-2 - j*2) = ' ';
                *(p_vmem-1 - j*2) = DEFAULT_CHAR_COLOR;
                j++;
            }

		}
		break;
        //新加的输出TAB功能
        case '\t':
            if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 4){
                push(p_con,p_con->cursor);
                for (int i = 0; i < 4; ++i) {
                    *p_vmem++ = ' ';
                    *p_vmem++ = BLUE;
                    p_con->cursor++;
                }
                break;
            }
            case 'z':
        case 'Z':
            if(ctrl){
                rollback(p_con);
                return;
            }
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
            push(p_con,p_con->cursor);
            if(mode == 0 && p_con->cursor>p_con->lastNormalCursor){
                p_con->lastNormalCursor = p_con->cursor;
            }
			*p_vmem++ = ch;
            if(mode == 0 || ch == ' '){
                *p_vmem++ = DEFAULT_CHAR_COLOR;
            } else{
                *p_vmem++ = RED;
            }

			p_con->cursor++;
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

PUBLIC void rollback(CONSOLE *p_con) {
    if(mode == 0){
        //清屏
        disp_pos = 0;
        for (int i = 0; i < SCREEN_SIZE; ++i) {
            disp_str(" ");
        }
        disp_pos = 0;

        p_con->cursor_Stack->now_index = 0;
        p_con->cursor = disp_pos/2;
    }
    if(mode == 1){
        p_con->cursor_Stack->now_index = p_con->actionStack.ESCseperator;
        disp_pos = p_con->lastNormalCursor*2;
        for (int i = 0; i < SCREEN_SIZE; ++i) {
            disp_str(" ");
        }
        disp_pos = p_con->lastNormalCursor*2;
        p_con->cursor = disp_pos/2;
    }
    flush(p_con);

    //下面根据actionSTACK复现
    int start = mode == 1?p_con->actionStack.ESCseperator:0;
    p_con->actionStack.index-=2;
    if(p_con->actionStack.index<=0){
        p_con->actionStack.index=0;
        return;
        //已经清空
    }
    for (int i = start; i < p_con->actionStack.index; ++i) {
        out_char(p_con,p_con->actionStack.ch[i]);
    }


}

//搜索函数
void search(CONSOLE *p_con) {
    int length = p_con->cursor - p_con->lastNormalCursor;
    if(length == 0)
        return;

    char* searchedChar;//被搜索的字符
    char* searchedColor;

    char* searchChar;//匹配字符串
    char* searchColor;

    for (int i = 0; i < p_con->lastNormalCursor; ++i) {
        int isfind = 1;
        if(length > p_con->lastNormalCursor){
            isfind = 0;
        }
        for (int j = 0; j < length; ++j) {
            searchedChar = (char*)(V_MEM_BASE+i*2+j*2);
            searchedColor = (char*)(V_MEM_BASE+i*2+j*2+1);

            searchChar = (char*)(V_MEM_BASE+p_con->lastNormalCursor*2+j*2);
            searchColor = (char*)(V_MEM_BASE+p_con->lastNormalCursor*2+j*2+1);

            if(*searchChar != *searchedChar|| (*searchedChar ==' '&&*searchColor!=*searchedColor)){
                isfind = 0;
                break;
            }
        }

        if(isfind == 1){
            for (int j = 0; j < length; ++j) {
                if(*(u8*)(V_MEM_BASE + i * 2+j*2)!=' ')
                    *(u8*)(V_MEM_BASE + i * 2+j*2+1)=RED;
            }
        }

    }
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

PRIVATE void push(CONSOLE* p_con, unsigned int index){
    if(p_con->cursor_Stack->now_index<p_con->cursor_Stack->length){
        p_con->cursor_Stack->array[p_con->cursor_Stack->now_index] = index;
        p_con->cursor_Stack->now_index++;
    }
}


PRIVATE unsigned int pop(CONSOLE* p_con){
    if (p_con->cursor_Stack->now_index>=1){
        unsigned int res = p_con->cursor_Stack->array[p_con->cursor_Stack->now_index-1];
        p_con->cursor_Stack->now_index--;
        return res;
    } else{
        return p_con->cursor_Stack->length+1;
    }
}

