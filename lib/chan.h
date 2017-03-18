#ifndef __CHAN_H__
#define __CHAN_H__

#include <sys/types.h>

struct chan_msg
{
	void* data;
	size_t len;
};
struct chan;

/**
 * Create a uni directonal channel.
 * @return the channel.
 */
extern struct chan* chan_create(void);

/**
 * Destroy the channel.
 * Any behavior of subsequent read/write is undefined.
 */
extern void chan_destroy(struct chan*);

/**
 * Closes the write end of the channel.
 */
extern void chan_close(struct chan*);

/**
 * Reads from the channel.
 * @param the channel.
 * @param a pointer to chan_msg struct to store the message in.
 * @param timeout in milliseconds. 0 for polling, -1 for blocking.
 * @return 0 on success.
 *         EBADF if the channel is closed.
 *         EAGAIN if the timeout was reached.
 *         -1 on other error, see errno for more information.
 */
extern int chan_read(struct chan*, struct chan_msg*, int);

/**
 * Read a single message from a multiple set of channels.
 * At most one message is read. If multiple channels are ready
 * the first read channel will be read from.
 * @param pointer to channels.
 * @param the number of channels to wait for.
 * @param a pointer to chan_msg struct to store the message in.
 * @param timeout in milliseconds.
 * @return 0 on success.
 *         EBADF if the channel is closed.
 *         EAGAIN if the timeout was reached.
 *         -1 on other error, see errno for more information.
 */
extern int chan_select(struct chan**, unsigned int, struct chan_msg*, int);

/**
 * Write a message onto a channel.
 * When writing a message, the data referenced by the message is *not* copied.
 * It is up to the client to defined the ownership of any referenced data.
 * If channel is full, this call blocks until the channel is drained.
 * @param the channel.
 * @param the message to write.
 * @return 0 if successful.
 *         -1 otherwise, se errno for more information.
 */
extern int chan_write(struct chan*, struct chan_msg*);

/**
 * Read from multiple channels and re-transmit to a single (merge).
 * This method creates a pthread to manage the operation.
 * To stop the operation, close the returned channel. The provided source
 * channels will not be affected by this.
 * @param pointer to channels to read from.
 * @param the number of channels.
 * @return a new channel to read messages from.
 */
extern struct chan* chan_fan_in(struct chan**, unsigned int);

/**
 * Read from a single channel and duplicate each msg on a collection of
 * provided channels.
 * This method creates a pthread that manages the operation. To stop 
 * the operation, close the source channel. The target channels will not be 
 * closed.
 * @param pointer to channels to write to.
 * @param the number of channels.
 * @param the source channel to read messages from.
 * @return 0 if fan out was successfully created.
 */
extern int chan_fan_out(struct chan**, unsigned int, struct chan*);

/**
 * Read one msg from a channel.
 * This function is for internal use, do not call.
 * Performs a non blocking read of one mesage.
 * @return
 *     0: Message successfully read.
 *     -1: Error occured.
 *     EBADF: Channel is closed.
 *     EAGAIN: Nothing to read.
 */
extern int chan_read_msg(struct chan*, struct chan_msg*);

#endif /* __CHAN_H__ */
