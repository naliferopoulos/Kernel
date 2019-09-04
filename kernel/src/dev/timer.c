#include <dev/timer.h>
#include <arch/irq.h>
#include <arch/common.h>

volatile unsigned int timer_ticks = 0;

void set_timer_phase(int hz)
{
    int divisor = 1193180 / hz;       /* Calculate our divisor */
    outb(0x43, 0x36);             /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

void timer_handler(struct regs *r)
{
    /* Increment our 'tick count' */
    timer_ticks++;
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
	/* Sets timer phase to 100Hz */
	set_timer_phase(100);
    /* Installs 'timer_handler' to IRQ0 */
    irq_install_handler(0, timer_handler);
}

void timer_wait(int ticks)
{
    unsigned int eticks;

    eticks = timer_ticks + ticks;
    while(timer_ticks < eticks)
    {
        __asm__ __volatile__ ("sti//hlt//cli");
    }
}
