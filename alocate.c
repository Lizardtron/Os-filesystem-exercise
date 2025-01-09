static int ext2_allocate_in_bg(struct super_block *sb, int group,
                               struct buffer_head *bitmap_bh, unsigned long *count)
{
	ext2_fsblk_t group_first_block = ext2_group_first_block_no(sb, group);
	ext2_fsblk_t group_last_block = ext2_group_last_block_no(sb, group);
	ext2_grpblk_t nblocks = group_last_block - group_first_block + 1;
	ext2_grpblk_t first_free_bit;
	struct ext2_group_desc *gdp;
	unsigned long num;
	
	if((first_free_bit = find_next_zero_bit_le(bitmap_bh->b_data, nblocks, 0)) > nblocks)
	{
		ext2_debug("No available blocks in group: %d\n", group);
	        return -ENOSPC;
	}
	ext2_debug("First free bit: %llu\n", (unsigned long long)first_free_bit);

	num = le32_to_cpu(first_free_bit);
	ext2_set_bit_atomic(sb_bgl_lock(EXT2_SB(sb), group), num, bitmap_bh->b_data);

	for (int i = 0; i < *count && i < nblocks; i++)
	{
		if(*(bitmap_bh->b_data + num + i))
		{
			*count = i - 1;
		       //ext2_set_bit_atomic(sb_bgl_lock(EXT2_SB(sb), group), first_free_bit + i, bitmap_bh->data);	
		       break;
		}
		ext2_set_bit_atomic(sb_bgl_lock(EXT2_SB(sb), group), num + i, bitmap_bh->b_data);
		ext2_debug("Allocated block: %llu\n", (unsigned long long)(num + i));
	}

	ext2_debug("Successfully allocated %lu blocks starting from block %llu\n", num,
               (unsigned long long)(group_first_block + first_free_bit));
    	return group_first_block + first_free_bit; /* Return the starting block address */
 				
			
}
