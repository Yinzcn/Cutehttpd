
struct chain_t {
    void           *dptr;
    struct chain_t *prev;
    struct chain_t *next;
};



struct mblk_t {
    int    size;
    int    used;
    void  *mptr;
    struct mblk_t *prev;
    struct mblk_t *next;
    struct mblk_t *attrprev;
    struct mblk_t *attrnext;
};


struct pool_t {
    int    size;
    int    used;
    struct mblk_t *mblk;
    struct pool_t *next;
    struct mblk_t *free;
    struct mblk_t *unfr;
};


struct pool_t *
pool_new(int size)
{
    struct pool_t *pool;
    pool = calloc(1, sizeof(struct pool_t) + size);
    if (pool) {
        struct mblk_t *mblk;
        mblk       = (void *)pool + sizeof(struct pool_t);
        mblk->mptr = (void *)mblk + sizeof(struct mblk_t);
        mblk->size =         size - sizeof(struct mblk_t);
        mblk->prev = mblk;
        mblk->next = mblk;
        mblk->attrprev = mblk;
        mblk->attrnext = mblk;
        pool->mblk = mblk;
        pool->free = mblk;
        pool->size = mblk->size;
        pool->used = 0;
    }
    return pool;
}


void *
palloc(struct pool_t *pool, int size)
{
    if (pool == 0) {
        return 0;
    }
    struct mblk_t *mblk;
    struct mblk_t *curr;
    struct mblk_t *last;
    curr = pool->free;
    last = curr->attrprev;
    while (1) {
        if (curr->size > size) {
            mblk = curr;
            int left = curr->size - size;
            if (left > sizeof(struct mblk_t)) {
                mblk = (void *)curr + sizeof(struct mblk_t) + curr->size;
                mblk->attrprev = curr->attrprev;
                mblk->attrnext = curr->attrnext;
                curr->attrprev->attrnext = mblk;
                curr->attrnext->attrprev = mblk;
            } else {
                curr->attrprev->attrnext = curr->attrnext;
                curr->attrnext->attrprev = curr->attrprev;
            }
            return curr->mptr;
        }
        if (curr == last) {
            break;
        }
    }
    return 0;
}


void
pfree(void *mptr)
{
    struct mblk_t *mblk;
    mblk = mptr - sizeof(struct mblk_t);

}


int
main(int argc, char **argv)
{
    printf("%s %s\n", argv[0], argv[1]);
    int size = atoi(argv[1]);
    int sizx = 8;
    while (sizx < size) {
        sizx *= 2;
    }
    int sz58 = sizx*5/8;
    if (sz58 >= size) {
        sizx = sz58;
    } else {
        int sz34 = sizx*3/4;
        if (sz34 >= size) {
            sizx = sz34;
        }
    }

    printf("%d %d", size, sizx);

    struct pool_t *pool;
    pool = pool_new(1024 * 1024);
    void *p = palloc(pool, 64);
}
