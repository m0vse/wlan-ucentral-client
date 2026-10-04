/* SPDX-License-Identifier: BSD-3-Clause */
#include "../session_status.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	struct session_receipt old, next, saved;
	memset(&native_session, 0, sizeof(native_session));
	/* Loading a persisted UUID must not create remote receipt evidence. */
	saved = session_capture(42, 1);
	session_applied(saved, 42, 1, true);
	assert(!native_session.applied.sequence);
	session_connected();
	assert(native_session.connected && native_session.generation == 1);
	session_applied(session_capture(42, 1), 42, 1, true);
	assert(!native_session.applied.sequence);

	session_received(42, 1);
	old = session_capture(42, 1);
	assert(old.sequence && old.generation == 1);
	session_applied(old, 42, 1, false);
	assert(!native_session.applied.sequence);
	session_applied(old, 43, 1, true);
	session_applied(old, 42, 2, true);
	assert(!native_session.applied.sequence);
	session_applied(old, 42, 1, true);
	assert(native_session.applied.sequence == native_session.received.sequence);

	/* A superseded task cannot acknowledge a newer request, even when the
	 * controller reuses both UUID and request ID. Each task captures a ticket. */
	session_received(42, 1);
	next = session_capture(42, 1);
	assert(next.sequence != old.sequence && !native_session.applied.sequence);
	session_applied(old, 42, 1, true);
	assert(!native_session.applied.sequence);
	session_applied(next, 42, 1, true);
	assert(native_session.applied.sequence == next.sequence);

	/* Disconnect clears evidence. Old-session completion cannot prove a
	 * newly authenticated socket received and applied its configuration. */
	session_disconnected();
	assert(!native_session.connected && !native_session.received.sequence && !native_session.applied.sequence);
	session_applied(next, 42, 1, true);
	assert(!native_session.applied.sequence);
	session_connected();
	session_received(42, 1);
	session_applied(next, 42, 1, true);
	assert(!native_session.applied.sequence);
	next = session_capture(42, 1);
	session_applied(next, 42, 1, true);
	assert(next.generation == 2 && native_session.applied.sequence == next.sequence);

	/* Local task/config loading and invalid UUID have no fresh receipt. */
	session_received(43, 3);
	assert(!session_capture(42, 1).sequence);
	session_received(0, 4);
	assert(!native_session.received.sequence && !native_session.applied.sequence);
	/* Exhaustion must fail closed rather than reuse an old generation/ticket. */
	native_session.counter = UINT32_MAX;
	session_received(42, 1);
	assert(!native_session.received.sequence);
	native_session.generation = UINT32_MAX;
	session_connected();
	assert(!native_session.connected);
	puts("PASS: fresh native session/configuration receipts, task binding, supersession, failure, disconnect, saved-state and overflow refusal");
	return 0;
}
