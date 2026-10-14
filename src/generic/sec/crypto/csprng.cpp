#include <sec/memory.hpp>
#include <sec/hashes.hpp>
#include <sec/random.hpp>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static uint64_t state[8] = {0};
static bool init = false;

bool csprng_init(void) {
	if (init) return false;

	// Quick and dirty entropy
	// CSPRNG assumes that random_u64 is perfect, whilst on real systems it is not, it is often good enough.
	uint64_t dirtystuff[8] = {0};

	for (uint8_t i = 0; i < 8; i++) {
		if (random_u64(&dirtystuff[i]) == false) {
			memset(dirtystuff, 0, sizeof(dirtystuff));
			// State is assumed to be zero, but this is a just in case.
			memset(state, 0, sizeof(state));
			return false;
		}
	}

	sha512_bytes((void*)&dirtystuff, sizeof(dirtystuff), (void*)&state);

	memset(dirtystuff, 0, sizeof(dirtystuff));
	init = true;
	return true;
}

bool csprng_addentropy(void* data, uint64_t size) {
	if (!init) return false;
	if (!range_is_mapped((uintptr_t)data, size)) return false;

	uint64_t hasheddata[8] = {0};
	uint64_t newstate[8] = {0};
	sha512_bytes(data, size, (void*)&hasheddata);

	// Mix the hashed data with the current state
	for (int i = 0; i < 8; i++) {
	uint64_t extra_entropy = 0;
	if (!random_u64(&extra_entropy)) {
		memset(newstate, 0, 64);
		memset(hasheddata, 0, 64);
		extra_entropy = 0;
		return false;
	}
	newstate[i] = state[i] ^ hasheddata[i] ^ extra_entropy;
	extra_entropy = 0;
	}
	memcpy(state, newstate, 64);
	memset(newstate, 0, 64);
	memset(hasheddata, 0, 64);
	return true;
}

bool csprng_getrand(uint8_t *out) {
	uint64_t output[8] = {0};
	uint64_t newstate[8] = {0};
	uint64_t oldstate[8] = {0};

	if (!init) return false;
	if (!range_is_mapped((uintptr_t)out, 64)) return false;
	
	sha512_bytes((void*)&state,64, (void*)&output);
	sha512_bytes((void*)&output, 64, (void*)&newstate);

	// Add some extra entropy
	uint64_t tmpbuf;
	if (!random_u64(&tmpbuf)) {
		memset(&tmpbuf, 0, 8);
		memset(output, 0, 64);
		memset(newstate, 0, 64);
		return false;
	}

	// Copy the new state and then add in entropy
	memcpy(oldstate, state, 64);
	memcpy(state, newstate, 64);
	if (!csprng_addentropy(&tmpbuf, 8)) {
		memcpy(state, oldstate, 64);
		memset(&oldstate, 0, 64);
		memset(&tmpbuf, 0, 8);
		memset(output, 0, 64);
		memset(newstate, 0, 64);
		return false;
	}
	
	// Zero the state from the stack
	memcpy(out, output, 64);
	memset(&oldstate, 0, 64);
	memset(&tmpbuf, 0, 8);
	memset(output, 0, 64);
	memset(newstate, 0, 64);
	return true;
}