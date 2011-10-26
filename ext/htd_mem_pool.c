
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


struct lvl_group_t;

void free_lvl_groups(struct lvl_group_t *);


struct mem_index_t { // 内存索引
    int size; // 申请大小
    struct mem_block_t *mem_block;
    struct mem_index_t *prev;
    struct mem_index_t *next;
};


struct memp_t {
    struct htdx_t *htdx;
    struct mem_index_t *mem_indexes;
    struct lvl_group_t *lvl_groups;
};


struct mem_index_t *
mem_indexes_get(struct memp_t *memp, void *mem_block)
{
    if (!memp->mem_indexes) {
        htd_cry(memp->htdx, "error: called mem_index_get(memp=%d, mem_block=%d), memp->mem_indexes is NULL.", memp, mem_block);
        return NULL;
    }
    struct mem_index_t *curr, *last;
    curr = memp->mem_indexes;
    last = curr->prev;
    while (1) {
        if (curr->mem_block == mem_block) {
            return curr;
        }
        if (curr == last) {
            return NULL;
        }
        curr = curr->next;
    }
    return NULL;
}


void
mem_indexes_add(struct memp_t *memp, struct mem_block_t *mem_block)
{
    struct mem_index_t *mem_index;
    mem_index = calloc(1, sizeof(struct mem_index_t));
    mem_index->mem_block = mem_block;
    if (memp->mem_indexes) {
        mem_index->prev = memp->mem_indexes->prev;
        mem_index->next = memp->mem_indexes;
        mem_index->prev->next = mem_index;
        mem_index->next->prev = mem_index;
    } else {
        mem_index->prev   = mem_index;
        mem_index->next   = mem_index;
        memp->mem_indexes = mem_index;
    }
}


void
mem_indexes_del(struct memp_t *memp, struct mem_index_t *mem_index)
{
    if (mem_index->next == mem_index) { // only one
        memp->mem_indexes = NULL;
    } else {
        if (mem_index == memp->mem_indexes) { // the first
            memp->mem_indexes = mem_index->next;
        }
        mem_index->prev->next = mem_index->next;
        mem_index->next->prev = mem_index->prev;
    }
    free(mem_index);
}


void
mem_indexes_free(struct memp_t *memp)
{
    struct mem_index_t *curr, *next, *last;
    curr = memp->mem_indexes;
    if (!curr) {
        return;
    }
    last = curr->prev;
    while (1) {
        next = curr->next;
        free(curr);
        if (curr == last) {
            break;
        }
        curr = next;
    };
    memp->mem_indexes = NULL;
}


void
memp_destroy(struct memp_t *memp)
{
    if (!memp) {
        return;
    }

    if (memp->mem_indexes) {
        mem_indexes_free(memp);
    }

    if (memp->lvl_groups) {
        free_lvl_groups(memp->lvl_groups);
    }

    free(memp);
}


struct memp_t *
memp_new(struct htdx_t *htdx)
{
    struct memp_t *memp;
    memp = calloc(1, sizeof(struct memp_t));
    if (!memp) {
        debug_trace("alloc memp failed!");
        return NULL;
    }
    memp->htdx = htdx;
    return memp;
}
