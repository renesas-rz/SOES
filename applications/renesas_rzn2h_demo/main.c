#include "esc_hw.h"
#include "ecat_slv.h"
#include "ecat_options.h"
#include "options.h"
#include "utypes.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdint.h>

/* Application variables */
struct _Objects Obj;

#define MAX_EVENTS      1

#define GPIO_DEV_NAME   "/dev/gpiochip0"

#define LED0_OFFSET     183
#define LED1_OFFSET     184
#define LED2_OFFSET     23
#define LED3_OFFSET     24

#define DIPSW0_OFFSET   218
#define DIPSW1_OFFSET   219
#define DIPSW2_OFFSET   222
#define DIPSW3_OFFSET   251

int epfd;
struct epoll_event events[MAX_EVENTS];
int uio_fd_ecat;
struct gpiohandle_request led0, led1, led2, led3, dipsw0, dipsw1, dipsw2, dipsw3;

static int init_led_dipsw(void)
{
	int fd;

	fd = open(GPIO_DEV_NAME, O_RDWR);
	if (fd < 0) {
		printf("Unabled to open %s\n", GPIO_DEV_NAME);
		return -1;
	}

	led0.lineoffsets[0] = LED0_OFFSET;
	led0.flags = GPIOHANDLE_REQUEST_OUTPUT;
	led0.lines = 1;
	strcpy(led0.consumer_label, "LED0");
	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &led0) < 0) {
		printf("Error setting LED0 as GPIO %d to OUTPUT\n", LED0_OFFSET);
		goto close_fd;
	}

	led1.lineoffsets[0] = LED1_OFFSET;
	led1.flags = GPIOHANDLE_REQUEST_OUTPUT;
	led1.lines = 1;
	strcpy(led1.consumer_label, "LED1");
	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &led1) < 0) {
		printf("Error setting LED1 as GPIO %d to OUTPUT\n", LED1_OFFSET);
		goto close_led0;
	}

	led2.lineoffsets[0] = LED2_OFFSET;
	led2.flags = GPIOHANDLE_REQUEST_OUTPUT;
	led2.lines = 1;
	strcpy(led2.consumer_label, "LED2");
	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &led2) < 0) {
		printf("Error setting LED2 as GPIO %d to OUTPUT\n", LED2_OFFSET);
		goto close_led1;
	}

	led3.lineoffsets[0] = LED3_OFFSET;
	led3.flags = GPIOHANDLE_REQUEST_OUTPUT;
	led3.lines = 1;
	strcpy(led3.consumer_label, "LED3");
	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &led3) < 0) {
		printf("Error setting LED3 as GPIO %d to OUTPUT\n", LED3_OFFSET);
		goto close_led2;
	}

	dipsw0.lineoffsets[0] = DIPSW0_OFFSET;
	dipsw0.flags = GPIOHANDLE_REQUEST_INPUT;
	dipsw0.lines = 1;
	strcpy(dipsw0.consumer_label, "DIPSW0");
	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &dipsw0) < 0) {
		printf("Error setting DIPSW0 as GPIO %d to INPUT\n", DIPSW0_OFFSET);
		goto close_led3;
	}

	dipsw1.lineoffsets[0] = DIPSW1_OFFSET;
	dipsw1.flags = GPIOHANDLE_REQUEST_INPUT;
	dipsw1.lines = 1;
	strcpy(dipsw1.consumer_label, "DIPSW1");
	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &dipsw1) < 0) {
		printf("Error setting DIPSW1 as GPIO %d to INPUT\n", DIPSW1_OFFSET);
		goto close_dipsw0;
	}

	dipsw2.lineoffsets[0] = DIPSW2_OFFSET;
	dipsw2.flags = GPIOHANDLE_REQUEST_INPUT;
	dipsw2.lines = 1;
	strcpy(dipsw2.consumer_label, "DIPSW2");
	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &dipsw2) < 0) {
		printf("Error setting DIPSW2 as GPIO %d to INPUT\n", DIPSW2_OFFSET);
		goto close_dipsw1;
	}

	dipsw3.lineoffsets[0] = DIPSW3_OFFSET;
	dipsw3.flags = GPIOHANDLE_REQUEST_INPUT;
	dipsw3.lines = 1;
	strcpy(dipsw3.consumer_label, "DIPSW3");
	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &dipsw3) < 0) {
		printf("Error setting DIPSW3 as GPIO %d to INPUT\n", DIPSW3_OFFSET);
		goto close_dipsw2;
	}

        return 0;

close_dipsw2:
	close(dipsw2.fd);
close_dipsw1:
	close(dipsw1.fd);
close_dipsw0:
	close(dipsw0.fd);
close_led3:
	close(led2.fd);
close_led2:
	close(led2.fd);
close_led1:
	close(led1.fd);
close_led0:
	close(led0.fd);
close_fd:
	close(fd);

	return -1;
}

static void gpio_write(struct gpiohandle_request rq, uint8_t value)
{
	struct gpiohandle_data data;

	data.values[0] = value;
	if (ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0)
		printf("Unable to write to %s\n", rq.consumer_label);
}

static int gpio_read(struct gpiohandle_request rq)
{
	struct gpiohandle_data data;

	if (ioctl(rq.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
		printf("Unable to read from %s\n", rq.consumer_label);
		return 0;
	}

	return data.values[0];
}

void setled(uint32_t value)
{
	if (value & 1)
		gpio_write(led0, 1);
	else
		gpio_write(led0, 0);

	if (value & 2)
		gpio_write(led1, 1);
	else
		gpio_write(led1, 0);

	if (value & 4)
		gpio_write(led2, 1);
	else
		gpio_write(led2, 0);

	if (value & 8)
		gpio_write(led3, 1);
	else
		gpio_write(led3, 0);
}

uint8_t getdipsw(void)
{
	uint8_t u8DipSw;

	u8DipSw = 0;

	if (gpio_read(dipsw0))
		u8DipSw |= 0x01;

	if (gpio_read(dipsw1))
		u8DipSw |= 0x02;

	if (gpio_read(dipsw2))
		u8DipSw |= 0x04;

	if (gpio_read(dipsw3))
		u8DipSw |= 0x08;

	return u8DipSw;
}

void cb_get_inputs(void)
{
	volatile uint8_t io_input;

	io_input = getdipsw();
	Obj.BUTTON = io_input;
}

void cb_set_outputs(void)
{
	volatile uint32_t io_output;

	io_output = Obj.LED;
	setled(io_output);
}

void cb_state_change(uint8_t *as, uint8_t *an)
{
	if (*as == SAFEOP_TO_OP) {
		/* Enable watchdog interrupt */
		ESC_ALeventmaskwrite(ESC_ALeventmaskread() | ESCREG_ALEVENT_WD);
	}
}

/* Called from stack when stopping outputs */
void user_safeoutput(void)
{
	memset(&Obj.LED, 0, (sizeof(Obj.LED)));
}

/* PDI ISR handler */
void PDI_Isr(void)
{
	uint32_t alevent;
	uint16_t wd;

	ESC_read(ESCREG_ALEVENT, &alevent, sizeof(alevent));
	CC_ATOMIC_SET(ESCvar.ALevent, etohs(alevent));

	if (ESCvar.ALevent & (ESCREG_ALEVENT_SM2 | ESCREG_ALEVENT_SM3)) {
		DIG_process(DIG_PROCESS_OUTPUTS_FLAG |
			    DIG_PROCESS_APP_HOOK_FLAG |
			    DIG_PROCESS_INPUTS_FLAG);
	}

	/* SM watchdog */
	if (ESCvar.ALevent & ESCREG_ALEVENT_WD) {
		/* Ack the WD IRQ */
		ESC_read(ESCREG_WDSTATUS, &wd, sizeof(wd));
		/* Check if the WD have expired and if we're in OP */
		if (((wd & 0x1) == 0) && ((CC_ATOMIC_GET(ESCvar.App.state) & APPSTATE_OUTPUT) > 0)) {
			ESC_ALstatusgotoerror((ESCsafeop | ESCerror), ALERR_WATCHDOG);
			ESC_ALeventmaskwrite(ESC_ALeventmaskread() & (uint32_t)(~ESCREG_ALEVENT_WD));
		}
	}
}

// The function to be executed by threads
void* myThreadFun(void* arg)
{
	int val, i;

	val = 1;

	while (1) {
		int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		for (i = 0; i < nfds; i++)
			if (events[i].data.fd == uio_fd_ecat)
				PDI_Isr();

		/* Re-enable interrupt */
		if (write(uio_fd_ecat, &val, sizeof(val)) < 0)
			printf("Cannot re-enable ecat interrupt\n");
	}
}

int main_run(void * arg)
{
	struct epoll_event ev;
	pthread_t thread_id;
	int ret, val;

	static esc_cfg_t config =
	{
		.user_arg = "/dev/uio0",
		.use_interrupt = 1,
		.watchdog_cnt = S32_MAX,
		.set_defaults_hook = NULL,
		.pre_state_change_hook = NULL,
		.post_state_change_hook = cb_state_change,
		.application_hook = NULL,
		.safeoutput_override = user_safeoutput,
		.pre_object_download_hook = NULL,
		.post_object_download_hook = NULL,
		.rxpdo_override = NULL,
		.txpdo_override = NULL,
		.esc_hw_interrupt_enable = ESC_interrupt_enable,
		.esc_hw_interrupt_disable = ESC_interrupt_disable,
		.esc_hw_eep_handler = NULL,
		.esc_check_dc_handler = NULL,
	};

	/* Init LED, DIPSW */
	if (init_led_dipsw() < 0) {
		printf("Initialize LED, DIPSW failed\n");
		return -1;
	}

	ecat_slv_init(&config);

	uio_fd_ecat = open("/dev/uio0", O_RDWR);
	if (uio_fd_ecat < 0) {
		printf("Open /dev/uio0 error\n");
		return uio_fd_ecat;
	}

	epfd = epoll_create1(0);

	ev.events = EPOLLIN;
	ev.data.fd = uio_fd_ecat;
	epoll_ctl(epfd, EPOLL_CTL_ADD, uio_fd_ecat, &ev);

	/* Enable interrupt */
	val = 1;
	if (write(uio_fd_ecat, &val, sizeof(val)) < 0)
	{
		printf("Cannot enable ecat interrupt\n");
		return -1;
	}

	ret = pthread_create(&thread_id, NULL, &myThreadFun, NULL);
	if (ret != 0) {
		printf("Thread can't be created : [%d]\n", ret);
		return ret;
	}

	while (1)
	{
		ecat_slv_poll();
	}

	pthread_join(thread_id, NULL);

	return 0;
}

int main (void)
{
	printf ("Running ESC Slave\n");
	main_run (NULL);
	return 0;
}
