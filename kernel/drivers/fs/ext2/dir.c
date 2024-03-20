/*
 * Copyright (c) 2007 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <bsd/string.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "ext2_priv.h"

/* read in the dir, look for the entry */
static int ext2_dir_lookup(ext2_t *ext2, struct ext2_inode *dir_inode,
                           const char *name, inodenum_t *inum) {
    uint file_blocknum;
    int err;
    uint8_t *buf;
    size_t namelen = strlen(name);

    if (!S_ISDIR(dir_inode->i_mode)) return ERR_NOT_DIR;

    buf = malloc(EXT2_BLOCK_SIZE(ext2->sb));

    file_blocknum = 0;
    for (;;) {
        /* read in the offset */
        err = ext2_read_inode(ext2, dir_inode, buf,
                              file_blocknum * EXT2_BLOCK_SIZE(ext2->sb),
                              EXT2_BLOCK_SIZE(ext2->sb));
        if (err <= 0) {
            free(buf);
            return err;
        }

        /* walk through the directory entries, looking for the one that matches
         */
        struct ext2_dir_entry_2 *ent;
        uint pos = 0;
        while (pos < EXT2_BLOCK_SIZE(ext2->sb)) {
            ent = (struct ext2_dir_entry_2 *)&buf[pos];

            /* sanity check the record length */
            if (LE16(ent->rec_len) == 0) break;

            if (ent->name_len == namelen &&
                memcmp(name, ent->name, ent->name_len) == 0) {
                // match
                *inum = LE32(ent->inode);
                free(buf);
                return 1;
            }

            pos += ROUNDUP(LE16(ent->rec_len), 4);
        }

        file_blocknum++;

        /* sanity check the directory. 4MB should be enough */
        if (file_blocknum > 1024) {
            free(buf);
            return -1;
        }
    }
}

/* note, trashes path */
static int ext2_walk(ext2_t *ext2, char *path, struct ext2_inode *start_inode,
                     inodenum_t *inum, int recurse) {
    char *ptr;
    struct ext2_inode inode;
    struct ext2_inode dir_inode;
    int err;
    bool done;

    if (recurse > 4) return ERR_NO_ENTRY;

    /* chew up leading slashes */
    ptr = &path[0];
    while (*ptr == '/') ptr++;

    done = false;
    memcpy(&dir_inode, start_inode, sizeof(struct ext2_inode));
    while (!done) {
        /* process the first component */
        char *next_sep = strchr(ptr, '/');
        if (next_sep) {
            /* terminate the next component, giving us a substring */
            *next_sep = 0;
        } else {
            /* this is the last component */
            done = true;
        }

        /* do the lookup on this component */
        err = ext2_dir_lookup(ext2, &dir_inode, ptr, inum);
        if (err < 0) return err;

    nextcomponent:
        /* load the next inode */
        err = ext2_load_inode(ext2, *inum, &inode);
        if (err < 0) return err;

        /* is it a symlink? */
        if (S_ISLNK(inode.i_mode)) {
            char link[512];

            err = ext2_read_link(ext2, &inode, link, sizeof(link));
            if (err < 0) return err;

            /* recurse, parsing the link */
            if (link[0] == '/') {
                /* link starts with '/', so start over again at the rootfs */
                err =
                    ext2_walk(ext2, link, &ext2->root_inode, inum, recurse + 1);
            } else {
                err = ext2_walk(ext2, link, &dir_inode, inum, recurse + 1);
            }

            if (err < 0) return err;

            /* if we weren't done with our path parsing, start again with the
             * result of this recurse */
            if (!done) {
                goto nextcomponent;
            }
        } else if (S_ISDIR(inode.i_mode)) {
            /* for the next cycle, point the dir inode at our new directory */
            memcpy(&dir_inode, &inode, sizeof(struct ext2_inode));
        } else {
            if (!done) {
                /* we aren't done and this walked over a nondir, abort */
                return ERR_NO_ENTRY;
            }
        }

        if (!done) {
            /* move to the next separator */
            ptr = next_sep + 1;

            /* consume multiple separators */
            while (*ptr == '/') ptr++;
        }
    }

    return 0;
}

/* do a path parse, looking up each component */
int ext2_lookup(ext2_t *ext2, const char *_path, inodenum_t *inum) {
    char path[512];
    strlcpy(path, _path, sizeof(path));

    return ext2_walk(ext2, path, &ext2->root_inode, inum, 1);
}
