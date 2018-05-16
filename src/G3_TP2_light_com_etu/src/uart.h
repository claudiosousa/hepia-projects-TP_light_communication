/*
 * Description: UART 0 driver
 * Created on : 30 sept. 2013
 * Author     : VP
 */
#ifndef __UART_H 
#define __UART_H

#define IER_RX      	0x1				// interrupt enable flags
#define IER_TX      	0x2

#define IIR_TX			0x2				// interrupt identification flags
#define IIR_RX			0x4
#define IS_TX_EMPTY 	(1<<5)			// status flags on LSR register
#define IS_RX_NOT_EMPTY (1<<2)

/* Callback function prototype for UART interrupt */
typedef void (*uart_callback_t)(int int_status);

/* Description: UART 0 initialisation. Callbacks can be used, but at most once for TX and once for RX data.
 *              Note that only the first call of uart0_init_ref is setting up the baudrate, which can't be
 *              modified during further calls.
 *
 * Parameters: baudrate [bit/s]
 *             tx_callback: pointer on callback function called by interrupt at the end of TX character
 *             transmission. Note that calling uart0_init_ref() does NOT activate IER_THRE to avoid
 *             continuous calls of tx_callback when the transmission buffer is empty. It is up to
 *             the user to enable it when necessary (LPC_UART0->IER |= IER_THRE) and to disable it at
 *             the end of the transmission (LPC_UART0->IER &= ~IER_THRE). Once IER configured to enable
 *             the interrupt, the first interrupt can be provoked either by sending a character on the UART
 *             or by forcing the VIC to rise it with NVIC_SetPendingIRQ(UART0_IRQn). Note that if this last
 *             method is used, the int_flags of the callback parameter are not significant. In this case,
 *             LPC_UART0->LSR must be read to know if a transmission has been done.
 *             rx_callback: pointer on callback function called by interrupt on RX character
 *                          receiving. if NULL is given, no interrupt is configured. The callback
 *                          will be called each time a character is received.
 */
void uart0_init_ref(uint32_t baudrate, uart_callback_t tx_callback, uart_callback_t rx_callback);

/* Description: UART 0 initialisation
 *
 * Parameters: data: pointer on data to be sent
 *             length: data length [bytes]
 */
void uart0_send_ref(uint8_t *data, uint32_t length);

/* Description: stop UART0 interrupts. The switch off is only managed by the VIC. */
void uart0_stop_interrupt();

#endif
