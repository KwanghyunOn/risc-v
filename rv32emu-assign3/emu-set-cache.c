#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#include "emu-rv32i.h"
#include "emu-cache.h"

// Address in 4-Way Set-associative Cache
//
//   TAG   |      INDEX                         | OFFSET
//         | log(NUM_CACHE_BLOCKS)-log(NUM_WAY) | G
// 26 bits |     4 bits                         | 2 bits

#define NUM_WAY 4
#define INDEX_BIT_WIDTH 4
#define TAG_BIT_WIDTH (ADDR_BIT_WIDTH - G_BIT_WIDTH - INDEX_BIT_WIDTH)

struct cache_block
{
  uint32_t tag:TAG_BIT_WIDTH;
  uint32_t valid:1;
  uint32_t data;
  uint32_t counter:2;
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
	uint32_t lru_cid = idx << 2;
	uint32_t hit = 0;
	for(int i = 0; i < 4; i++) {
		uint32_t cid = (idx << 2) | i;
		if(cache[cid].valid && cache[cid].tag == tag) {
			value = cache[cid].data;
			cache[cid].counter = 0;
			num_cache_hit++;
			hit = 1;
			return value;
		} else {
			if(cache[cid].valid && cache[cid].counter < 3)
				cache[cid].counter++;
		}
		if(!cache[cid].valid || (cache[lru_cid].valid && cache[lru_cid].counter < cache[cid].counter))
			lru_cid = cid;
	}
	// lru_cid = idx << 2;
	if(!hit) {
		num_cache_miss++;
		uint8_t *p = ram + addr;
		value = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
		cache[lru_cid].tag = tag;
		cache[lru_cid].valid = 1;
		cache[lru_cid].data = value;
		cache[lru_cid].counter = 0;
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
	uint32_t lru_cid = idx << 2;
	for(int i = 0; i < 4; i++) {
		uint32_t cid = (idx << 2) | i;
		if(!cache[cid].valid || (cache[lru_cid].valid && cache[lru_cid].counter < cache[cid].counter))
			lru_cid = cid;
	}
	// lru_cid = idx << 2;
	cache[lru_cid].tag = tag;
	cache[lru_cid].valid = 1;
	cache[lru_cid].data = value;
	cache[lru_cid].counter = 0;
}
