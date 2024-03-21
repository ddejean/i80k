/*
 * Copyright (c) 2007-2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */

#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blkdev.h"
#include "error.h"
#include "ext2_priv.h"
#include "fs.h"

static void endian_swap_superblock(struct ext2_super_block *sb) {
    LE32SWAP(sb->s_inodes_count);
    LE32SWAP(sb->s_blocks_count);
    LE32SWAP(sb->s_r_blocks_count);
    LE32SWAP(sb->s_free_blocks_count);
    LE32SWAP(sb->s_free_inodes_count);
    LE32SWAP(sb->s_first_data_block);
    LE32SWAP(sb->s_log_block_size);
    LE32SWAP(sb->s_log_frag_size);
    LE32SWAP(sb->s_blocks_per_group);
    LE32SWAP(sb->s_frags_per_group);
    LE32SWAP(sb->s_inodes_per_group);
    LE32SWAP(sb->s_mtime);
    LE32SWAP(sb->s_wtime);
    LE16SWAP(sb->s_mnt_count);
    LE16SWAP(sb->s_max_mnt_count);
    LE16SWAP(sb->s_magic);
    LE16SWAP(sb->s_state);
    LE16SWAP(sb->s_errors);
    LE16SWAP(sb->s_minor_rev_level);
    LE32SWAP(sb->s_lastcheck);
    LE32SWAP(sb->s_checkinterval);
    LE32SWAP(sb->s_creator_os);
    LE32SWAP(sb->s_rev_level);
    LE16SWAP(sb->s_def_resuid);
    LE16SWAP(sb->s_def_resgid);
    LE32SWAP(sb->s_first_ino);
    LE16SWAP(sb->s_inode_size);
    LE16SWAP(sb->s_block_group_nr);
    LE32SWAP(sb->s_feature_compat);
    LE32SWAP(sb->s_feature_incompat);
    LE32SWAP(sb->s_feature_ro_compat);
    LE32SWAP(sb->s_algorithm_usage_bitmap);

    /* ext3 journal stuff */
    LE32SWAP(sb->s_journal_inum);
    LE32SWAP(sb->s_journal_dev);
    LE32SWAP(sb->s_last_orphan);
    LE32SWAP(sb->s_default_mount_opts);
    LE32SWAP(sb->s_first_meta_bg);
}

static void endian_swap_inode(struct ext2_inode *inode) {
    LE16SWAP(inode->i_mode);
    LE16SWAP(inode->i_uid_low);
    LE32SWAP(inode->i_size);
    LE32SWAP(inode->i_atime);
    LE32SWAP(inode->i_ctime);
    LE32SWAP(inode->i_mtime);
    LE32SWAP(inode->i_dtime);
    LE16SWAP(inode->i_gid_low);
    LE16SWAP(inode->i_links_count);
    LE32SWAP(inode->i_blocks);
    LE32SWAP(inode->i_flags);

    // leave block pointers/symlink data alone

    LE32SWAP(inode->i_generation);
    LE32SWAP(inode->i_file_acl);
    LE32SWAP(inode->i_dir_acl);
    LE32SWAP(inode->i_faddr);

    LE16SWAP(inode->i_uid_high);
    LE16SWAP(inode->i_gid_high);
}

static void endian_swap_group_desc(struct ext2_group_desc *gd) {
    LE32SWAP(gd->bg_block_bitmap);
    LE32SWAP(gd->bg_inode_bitmap);
    LE32SWAP(gd->bg_inode_table);
    LE16SWAP(gd->bg_free_blocks_count);
    LE16SWAP(gd->bg_free_inodes_count);
    LE16SWAP(gd->bg_used_dirs_count);
}

int ext2_mount(const struct blkdev *dev, fscookie **cookie) {
    int err;

    if (!dev) return ERR_INVAL;

    ext2_t *ext2 = calloc(1, sizeof(ext2_t));
    if (!ext2) {
        printf("ext2: failed to allocate superblock\n");
        return ERR_NO_MEM;
    }
    ext2->dev = dev;

    err = blk_read(dev, &ext2->sb, 1024, sizeof(struct ext2_super_block));
    if (err < 0) {
        printf("ext2: failed to read superblock (error %d)\n", err);
        goto err;
    }

    endian_swap_superblock(&ext2->sb);

    /* see if the superblock is good */
    if (ext2->sb.s_magic != EXT2_SUPER_MAGIC) {
        printf("ext2: invalid superblock magic %x\n", ext2->sb.s_magic);
        err = -1;
        goto err;
    }

    /* calculate group count, rounded up */
    ext2->s_group_count =
        (ext2->sb.s_blocks_count + ext2->sb.s_blocks_per_group - 1) /
        ext2->sb.s_blocks_per_group;

    /* we only support dynamic revs */
    if (ext2->sb.s_rev_level > EXT2_DYNAMIC_REV) {
        printf("ext2: implementation only supports dynamic revisions\n");
        err = ERR_NOT_SUPP;
        goto err;
    }

    /* make sure it doesn't have any ro features we don't support */
    if (ext2->sb.s_feature_ro_compat & ~(EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER)) {
        // Add EXT2_FEATURE_RO_COMPAT_LARGE_FILE once it is supported.
        err = ERR_NOT_SUPP;
        goto err;
    }

    /* read in all the group descriptors */
    ext2->gd = calloc(ext2->s_group_count, sizeof(struct ext2_group_desc));
    if (!ext2->gd) {
        err = ERR_NO_MEM;
        goto err;
    }
    err = blk_read(ext2->dev, (void *)ext2->gd,
                   (EXT2_BLOCK_SIZE(ext2->sb) == 4096) ? 4096 : 2048,
                   sizeof(struct ext2_group_desc) * ext2->s_group_count);
    if (err < 0) {
        err = -4;
        goto err;
    }

    int i;
    for (i = 0; i < ext2->s_group_count; i++) {
        endian_swap_group_desc(&ext2->gd[i]);
    }

    /* load the first inode */
    err = ext2_load_inode(ext2, EXT2_ROOT_INO, &ext2->root_inode);
    if (err < 0) goto err;

    *cookie = (fscookie *)ext2;

    return 0;

err:
    free(ext2->gd);
    free(ext2);
    return err;
}

int ext2_unmount(fscookie *cookie) {
    // free it up
    ext2_t *ext2 = (ext2_t *)cookie;

    free(ext2->gd);
    free(ext2);

    return 0;
}

static void get_inode_addr(ext2_t *ext2, inodenum_t num, blocknum_t *block,
                           size_t *block_offset) {
    num--;

    uint32_t group = num / ext2->sb.s_inodes_per_group;

    // calculate the start of the inode table for the group it's in
    *block = ext2->gd[group].bg_inode_table;

    // add the offset of the inode within the group
    size_t offset =
        (num % EXT2_INODES_PER_GROUP(ext2->sb)) * EXT2_INODE_SIZE(ext2->sb);
    *block_offset = offset % EXT2_BLOCK_SIZE(ext2->sb);
    *block += offset / EXT2_BLOCK_SIZE(ext2->sb);
}

int ext2_load_inode(ext2_t *ext2, inodenum_t num, struct ext2_inode *inode) {
    int err;

    blocknum_t bnum;
    size_t block_offset;
    get_inode_addr(ext2, num, &bnum, &block_offset);

    err = blk_read(ext2->dev, inode,
                   (bnum * EXT2_BLOCK_SIZE(ext2->sb)) + block_offset,
                   sizeof(struct ext2_inode));
    if (err < 0) return err;

    /* endian swap it */
    endian_swap_inode(inode);

    return 0;
}

static const struct fs_api ext2_api = {
    .mount = ext2_mount,
    .unmount = ext2_unmount,
    .open = ext2_open_file,
    .stat = ext2_stat_file,
    .read = ext2_read_file,
    .close = ext2_close_file,
};

FS(ext2, &ext2_api);
