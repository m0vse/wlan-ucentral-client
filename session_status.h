/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef UCENTRAL_SESSION_STATUS_H
#define UCENTRAL_SESSION_STATUS_H
#include <stdbool.h>
#include <stdint.h>

struct session_receipt {
	uint32_t generation, sequence, request;
	uint64_t uuid;
};
struct native_session_status {
	bool connected;
	uint32_t generation, counter;
	struct session_receipt received, applied;
};
extern struct native_session_status native_session;
void session_connected(void);
void session_disconnected(void);
void session_received(uint64_t uuid, uint32_t request);
struct session_receipt session_capture(uint64_t uuid, uint32_t request);
void session_applied(struct session_receipt receipt, uint64_t uuid,
		     uint32_t request, bool success);
#endif
