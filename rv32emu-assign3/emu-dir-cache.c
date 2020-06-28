#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include "emu-rv32i.h"
#include "emu-cache.h"

// Address in Direct Mapped Cache
//
//   TAG   |      INDEX            | OFFSET
//   TAG   | log(NUM_CACHE_BLOCKS) |   G
// 24 bits |     6 bits            | 2 bits

#define INDEX_BIT_WIDTH 6
#define TAG_BIT_WIDTH (ADDR_BIT_WIDTH - G_BIT_WIDTH - INDEX_BIT_WIDTH)

struct cache_block
{
  uint32_t tag:TAG_BIT_WIDTH;
  uint32_t valid:1;
  uint32_t data;
};

struct cache_block cache[NUM_CACHE_BLOCKS];

void cache_init(void)
{
  num_cache_hit = 0;
  num_cache_miss = 0;

  for(uint32_t idx=0; idx<NUM_CACHE_BLOCKS; idx++)
    cache[idx].valid = 0;
  return;
}

uint32_t cache_read(uint32_t addr)
{
  // TODO: Assignment #3
  uint32_t value = 0;
	uint32_t idx = (addr >> G_BIT_WIDTH) & ((1<<INDEX_BIT_WIDTH)-1);
	uint32_t tag = addr >> (INDEX_BIT_WIDTH + G_BIT_WIDTH);
	if(cache[idx].valid && cache[idx].tag == tag) {
		value = cache[idx].data;
		num_cache_hit++;
	} else {
		uint8_t *p = ram + addr;
		value = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
		cache[idx].tag = tag;
		cache[idx].valid = 1;
		cache[idx].data = value;
		num_cache_miss++;
	}
  return value;
}

void cache_write(uint32_t addr, uint32_t value)
{
  // TODO: Assignment #3
	uint8_t *p = ram + addr;
	p[0] = value & 0xff;
	p[1] = (value >> 8) & 0xff;
	p[2] = (value >> 16) & 0xff;
	p[3] = (value >> 24) & 0xff;

	uint32_t idx = (addr >> G_BIT_WIDTH) & ((1<<INDEX_BIT_WIDTH)-1);
	uint32_t tag = addr >> (INDEX_BIT_WIDTH + G_BIT_WIDTH);
	cache[idx].tag = tag;
	cache[idx].valid = 1;
	cache[idx].data = value;
}
