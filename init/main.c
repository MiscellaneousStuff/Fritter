#include <stddef.h>
#include <string.h>

#include <fritter/kernel.h>

#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "timer.h"
#include "rtc.h"

// 219 = block

void *gets(char *s) {
  size_t n = 0;
  for (;;) {
    char c = keyboard_getchar();
    if (c) {
      if (c == '\b' && n > 0) {
        s--;
        n--;
        putc('\b');
      } else if (c == '\n') {
        *s = '\0';
        break;
      } else {
        *s++ = c;
        n++;
        putc(c);
      }
    }
  }
}
 
int kmain() {
  // Init Global/Interrupt Descriptor Tables
  init_gdt();
  init_idt();

  // Init date/time
  read_rtc();

  // Init screen
  init_tty();

  // Welcome message
  printf("Welcome to fritter!\n");

  // Enable Interrupts
  asm volatile("sti");

  // Init timer at 100Hz
  init_timer(100);

  // Init Keyboard Driver
  init_keyboard_driver();

  // Print char from user
  char cmd[100];

  // CMD loop
  for (;;) {
    // Prompt
    printf("> ");

    // Get Command
    gets(cmd);
    printf("\n");

    // Determine command
    if (strncmp(cmd, "help", strlen(cmd)) == 0) {
      printf("clear\t Clears the screen\n");
      printf("help\t Prints this help information");
    } else if (strncmp(cmd, "clear", strlen(cmd)) == 0) {
      clrscr();
    } else if (strncmp(cmd, "echo ", 5) == 0) {
      printf(cmd+5);
    } else if (strncmp(cmd, "time", 4) == 0) {
      printf("%04d/%02d/%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    } else {
      printf("%s is an unknown command. Try again.", cmd);
    }

    // Newline
    printf("\n");
  }

  // Wait for timing events
  for (;;) {
    asm volatile ("hlt");
  }

  return 0xDEADBEEF;
}