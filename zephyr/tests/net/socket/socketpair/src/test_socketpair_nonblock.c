/*
 * Copyright (c) 2020 Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(net_test, CONFIG_NET_SOCKETS_LOG_LEVEL);

#include <stdio.h>
#include <string.h>
#include <net/socket.h>
#include <sys/util.h>
#include <posix/unistd.h>

#include <ztest_assert.h>

#undef read
#define read(fd, buf, len) zsock_recv(fd, buf, len, 0)

#undef write
#define write(fd, buf, len) zsock_send(fd, buf, len, 0)

void test_socketpair_write_nonblock(void)
{
	int res;
	int sv[2] = {-1, -1};

	res = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	zassert_equal(res, 0, "socketpair(2) failed: %d", errno);

	for (size_t i = 0; i < 2; ++i) {
		/* first, fill up the buffer */
		for (size_t k = 0; k < CONFIG_NET_SOCKETPAIR_BUFFER_SIZE;
			++k) {
			res = write(sv[i], "x", 1);
			zassert_equal(res, 1, "write(2) failed: %d", errno);
		}

		/* then set the O_NONBLOCK flag */
		res = fcntl(sv[i], F_GETFL, 0);
		zassert_not_equal(res, -1, "fcntl() failed: %d", i, errno);

		res = fcntl(sv[i], F_SETFL, res | O_NONBLOCK);
		zassert_not_equal(res, -1, "fcntl() failed: %d", i, errno);

		/* then, try to write one more byte */
		res = write(sv[i], "x", 1);
		zassert_equal(res, -1, "expected write to fail");
		zassert_equal(errno, EAGAIN, "errno: exected: EAGAIN "
			"actual: %d", errno);
	}

	close(sv[0]);
	close(sv[1]);
}

void test_socketpair_read_nonblock(void)
{
	int res;
	int sv[2] = {-1, -1};
	char c;

	res = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	zassert_equal(res, 0, "socketpair(2) failed: %d", errno);

	for (size_t i = 0; i < 2; ++i) {
		/* set the O_NONBLOCK flag */
		res = fcntl(sv[i], F_GETFL, 0);
		zassert_not_equal(res, -1, "fcntl() failed: %d", i, errno);

		res = fcntl(sv[i], F_SETFL, res | O_NONBLOCK);
		zassert_not_equal(res, -1, "fcntl() failed: %d", i, errno);

		/* then, try to read one byte */
		res = read(sv[i], &c, 1);
		zassert_equal(res, -1, "expected read to fail");
		zassert_equal(errno, EAGAIN, "errno: exected: EAGAIN "
			"actual: %d", errno);
	}

	close(sv[0]);
	close(sv[1]);
}
