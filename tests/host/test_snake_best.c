#include <assert.h>
#include <stdio.h>
#include "best_score.h"

static dada_err_t write_result;
static unsigned writes;

static dada_err_t set_u32(void *ctx, const char *key, uint32_t value)
{
    (void)ctx;
    assert(key[0] != '\0');
    assert(value > 0);
    writes++;
    return write_result;
}

static dada_storage_t storage = {.set_u32 = set_u32};

int main(void)
{
    uint32_t best = 10;
    assert(snake_update_best(&storage, 20, &best) == DADA_OK);
    assert(best == 20 && writes == 1);

    write_result = DADA_ERR_FAIL;
    assert(snake_update_best(&storage, 30, &best) == DADA_ERR_FAIL);
    assert(best == 20 && writes == 2); /* failed durable write cannot advance reported best */

    assert(snake_update_best(NULL, 40, &best) == DADA_ERR_NOT_SUPPORTED);
    assert(best == 40); /* continue in RAM when persistence is not available */
    assert(snake_update_best(&storage, 39, &best) == DADA_OK);
    assert(writes == 2); /* not a new record */
    puts("snake best score: durable success, failure and volatile fallback PASS");
}
