static int ext2_allocate_in_bg(struct super_block *sb, int group,
                               struct buffer_head *bitmap_bh, unsigned long *count)
{
	ext2_fsblk_t group_first_block = ext2_group_first_block_no(sb, group);
	ext2_fsblk_t group_last_block = ext2_group_last_block_no(sb, group);
	ext2_grpblk_t nblocks = group_last_block - group_first_block + 1;
	ext2_grpblk_t first_free_bit;
	struct ext2_group_desc *gdp;
	unsigned long num;
	
	ext2_debug("Allocating in group: %d\n", group);
    	ext2_debug("Group first block: %llu, Group last block: %llu, Total blocks: %llu\n",
               (unsigned long long)group_first_block,
               (unsigned long long)group_last_block,
               (unsigned long long)nblocks);

    	/* Find the first free bit in the bitmap */
    	first_free_bit = find_next_zero_bit_le(bitmap_bh->b_data, nblocks, 0);
    	ext2_debug("First free bit: %llu\n", (unsigned long long)first_free_bit);

    	if (first_free_bit >= nblocks) {
        	/* No available blocks */
        	ext2_debug("No available blocks in group: %d\n", group);
        	return -ENOSPC;
    	}

    	/* Allocate the block(s) */
    	for (num = 0; num < *count && first_free_bit + num < nblocks; num++) {
		if(*(bitmap_bh->b_data + first_free_bit + num) == 1)
			break;
        	ext2_set_bit_atomic(sb_bgl_lock(EXT2_SB(sb), group), first_free_bit + num, bitmap_bh->b_data);
        	ext2_debug("Allocated block: %llu\n", (unsigned long long)(first_free_bit + num));
    	}

    	if (num < *count) {
        	/* Unable to fully allocate */
        	ext2_debug("Partial allocation: only %lu out of %lu blocks allocated\n", num, *count);
        	*count = num;
        	return -ENOSPC;
    	}

    	*count = num;
    	ext2_debug("Successfully allocated %lu blocks starting from block %llu\n", num, 
               (unsigned long long)(group_first_block + first_free_bit));
    	return group_first_block + first_free_bit; /* Return the starting block address */
			
}
