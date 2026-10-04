/* SPDX-License-Identifier: BSD-3-Clause */
#include "session_status.h"
#include <string.h>
#include <limits.h>

struct native_session_status native_session;

static void clear_receipts(void)
{
	memset(&native_session.received, 0, sizeof(native_session.received));
	memset(&native_session.applied, 0, sizeof(native_session.applied));
}

void session_connected(void)
{
	clear_receipts();
	/* Do not reuse a generation after overflow. A new client process resets it. */
	native_session.connected = native_session.generation != UINT32_MAX;
	if (native_session.connected) native_session.generation++;
}

void session_disconnected(void)
{
	native_session.connected = false;
	clear_receipts();
}

void session_received(uint64_t uuid, uint32_t request)
{
	clear_receipts();
	if (!native_session.connected || !uuid || native_session.counter == UINT32_MAX)
		return;
	native_session.counter++;
	native_session.received = (struct session_receipt){
		.generation = native_session.generation,
		.sequence = native_session.counter, .request = request, .uuid = uuid
	};
}

struct session_receipt session_capture(uint64_t uuid, uint32_t request)
{
	struct session_receipt receipt = native_session.received;
	if (!native_session.connected || receipt.uuid != uuid || receipt.request != request)
		return (struct session_receipt){0};
	return receipt;
}

void session_applied(struct session_receipt receipt, uint64_t uuid,
		     uint32_t request, bool success)
{
	struct session_receipt current = native_session.received;
	if (!success || !native_session.connected || !receipt.sequence ||
	    receipt.generation != native_session.generation ||
	    receipt.generation != current.generation || receipt.sequence != current.sequence ||
	    receipt.uuid != uuid || receipt.uuid != current.uuid ||
	    receipt.request != request || receipt.request != current.request)
		return;
	native_session.applied = receipt;
}
