#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
  int cur;
  uint8_t total;
  uint8_t nums[27];
} comb_t;

typedef struct {
  uint8_t count; // subtree leaf count
  uint8_t parent;
  uint8_t lhs;
  uint8_t rhs;
} astnode_t;

typedef struct {
  astnode_t buf[48];
  uint8_t free[48];
  uint8_t stack[48];
  uint8_t freep;
  uint8_t sp;
} astenum_t;

enum {
  OP_ADD = 0,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_END
};

struct hash_node {
  struct hash_node *next;
  uint64_t res;
  uint8_t n;
  char str[111];
};

#define HASH_BINCOUNT 65536

struct hash_node *hash_bins[HASH_BINCOUNT];

void comb_first(comb_t *comb, uint8_t total) {
  comb->cur = 0;
  comb->total = total;
  comb->nums[0] = total;
  comb->nums[1] = 0;
}

int comb_next(comb_t *comb) {
  int i, j;
  uint8_t left;

  i = comb->cur;
  while (i>=0 && comb->nums[i]<=1)
    i--;
  if (i<0) return 0;
  comb->nums[i]--;
  left = comb->total;
  for (j=0; j<=i; j++)
    left -= comb->nums[j];
  i++;
  comb->nums[i] = left;
  comb->nums[i+1] = 0;
  comb->cur = i;
  return 1;
}

void astenum_first(astenum_t *ast, uint8_t size) {
  int i;
  uint8_t tstack[48];
  uint8_t *free, *stack, tsp, freep, sp, p;
  astnode_t *buf;

  buf = ast->buf;
  free = ast->free;
  stack = ast->stack;

  memset(ast, 0, sizeof(*ast));
  for (i=0; i<47; i++)
    free[i] = 47 - i;
  freep = 47;
  sp = 0;
  tsp = 0;

  buf[0].count = size;
  buf[0].parent = 0;
  stack[sp++] = 255;
  tstack[tsp++] = 0;
  while (tsp > 0) {
    p = tstack[--tsp];
    if (buf[p].count > 1) {
      if (!buf[p].lhs) {
        buf[p].lhs = free[--freep];
        buf[buf[p].lhs].count = buf[p].count - 1;
        buf[buf[p].lhs].parent = p;
      }
      if (!buf[p].rhs) {
        buf[p].rhs = free[--freep];
        buf[buf[p].rhs].count = 1;
        buf[buf[p].rhs].parent = p;
      }
      tstack[tsp++] = buf[p].rhs;
      tstack[tsp++] = buf[p].lhs;
      stack[sp++] = p;
    }
  }

  ast->sp = sp;
  ast->freep = freep;
}

int astenum_next(astenum_t *ast) {
  int i;
  uint8_t tstack[48];
  uint8_t *free, *stack, tsp, freep, sp, p;
  astnode_t *buf;

  buf = ast->buf;
  free = ast->free;
  stack = ast->stack;
  freep = ast->freep;
  sp = ast->sp;

  tsp = 0;
  for (;;) {
    p = stack[--sp];
    if (p == 255 ||
        buf[p].count > 2 && buf[buf[p].lhs].count > 1)
      break;
    free[freep++] = buf[p].lhs;
    free[freep++] = buf[p].rhs;
    buf[p].lhs = 0;
    buf[p].rhs = 0;
  }
  if (p == 255) return 0;
  sp = 1;
  buf[buf[p].lhs].count--;
  buf[buf[p].rhs].count++;
  tstack[tsp++] = 0;
  while (tsp > 0) {
    p = tstack[--tsp];
    if (buf[p].count > 1) {
      if (!buf[p].lhs) {
        buf[p].lhs = free[--freep];
        buf[buf[p].lhs].count = buf[p].count - 1;
        buf[buf[p].lhs].parent = p;
      }
      if (!buf[p].rhs) {
        buf[p].rhs = free[--freep];
        buf[buf[p].rhs].count = 1;
        buf[buf[p].rhs].parent = p;
      }
      tstack[tsp++] = buf[p].rhs;
      tstack[tsp++] = buf[p].lhs;
      stack[sp++] = p;
    }
  }

  ast->sp = sp;
  ast->freep = freep;
  return 1;
}

void printast(astenum_t *ast) {
  int i;
  uint8_t tstack[48];
  uint8_t tsp, p;
  astnode_t *buf;

  buf = ast->buf;

  tsp = 0;
  tstack[tsp++] = 0;
  while (tsp > 0) {
    p = tstack[--tsp];
    if (buf[p].count > 1) {
      tstack[tsp++] = buf[p].rhs;
      tstack[tsp++] = buf[p].lhs;
    }
    printf("%d ", buf[p].count);
  }
  printf("\n");
}

uint64_t evalast(astenum_t *ast, uint8_t *nums, uint8_t *ops, char *pstr) {
  int i;
  uint8_t tstack[72];
  uint64_t nstack[24];
  uint64_t num, num2, res, mod;
  uint8_t tsp, nsp, p, n, op;
  astnode_t *buf;

  buf = ast->buf;

  tsp = 0;
  nsp = 0;
  tstack[tsp++] = 0;
  while (tsp > 0) {
    p = tstack[--tsp];
    if (p & 0x80) {
      op = p & 0x7f;
      num2 = nstack[--nsp];
      num = nstack[--nsp];
      switch (op) {
        case OP_ADD:
          *pstr++ = '+';
          *pstr++ = ' ';
          res = num + num2;
          nstack[nsp++] = res;
          break;
        case OP_SUB:
          *pstr++ = '-';
          *pstr++ = ' ';
          if (num < num2) return 0;
          res = num - num2;
          nstack[nsp++] = res;
          break;
        case OP_MUL:
          *pstr++ = '*';
          *pstr++ = ' ';
          res = num * num2;
          nstack[nsp++] = res;
          break;
        case OP_DIV:
          *pstr++ = '/';
          *pstr++ = ' ';
          if (!num2) return 0;
          res = num / num2;
          mod = num % num2;
          if (mod) return 0;
          nstack[nsp++] = res;
          break;
      }
    }
    else if (buf[p].count > 1) {
      tstack[tsp++] = *ops++ | 0x80;
      tstack[tsp++] = buf[p].rhs;
      tstack[tsp++] = buf[p].lhs;
    }
    else {
      n = *nums++;
      num = 0;
      for (i=0; i<n; i++) {
        *pstr++ = '9';
        num *= 10;
        num += 9;
      }
      *pstr++ = ' ';
      nstack[nsp++] = num;
    }
  }
  res = nstack[--nsp];
  *--pstr = 0;
  return res;
}

void enumops_first(uint8_t *ops, uint8_t n) {
  int i;
  for (i=0; i<n; i++)
    ops[i] = OP_ADD;
}

int enumops_next(uint8_t *ops, uint8_t n) {
  int i;
  for (i=0; i<n; i++) {
    ops[i]++;
    if (ops[i] < OP_END)
      return 1;
    ops[i] = OP_ADD;
  }
  return 0;
}

struct hash_node *hash_lookup(uint64_t res) {
  uint32_t hash;
  struct hash_node *node;

  hash = res % HASH_BINCOUNT;
  for (node = hash_bins[hash]; node; node = node->next)
    if (node->res == res)
      return node;
  return NULL;
}

void hash_add(struct hash_node *node) {
  uint32_t hash;

  hash = node->res % HASH_BINCOUNT;
  if (!hash_bins[hash]) {
    node->next = NULL;
    hash_bins[hash] = node;
  }
  else {
    node->next = hash_bins[hash];
    hash_bins[hash] = node;
  }
}

void hash_dump() {
  uint32_t i;
  struct hash_node *p;

  for (i=0; i<HASH_BINCOUNT; i++)
    for (p=hash_bins[i]; p; p=p->next)
      printf("%llu = %s\n", p->res, p->str);
}

void hash_free() {
  uint32_t i;
  struct hash_node *p, *q;

  for (i=0; i<HASH_BINCOUNT; i++) {
    p = hash_bins[i];
    while (p) {
      q = p->next;
      free(p);
      p = q;
    }
  }
}

int main(int argc, char **argv) {
  int i, j;
  int x, n;
  uint64_t res;
  uint8_t ops[24];
  comb_t comb;
  astenum_t ast;
  char str[128];

  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  if (argc != 2)
    return 1;
  x = atoi(argv[1]);

  for (i=1; i<=x; i++) {
    fprintf(stderr, "N=%d\n", i);
    comb_first(&comb, i);
    j = 1;
    do {
      fprintf(stderr, "%d/%d\r", j++, 1<<(i-1));
      for (n=0; comb.nums[n]; n++);
      astenum_first(&ast, n);
      do {
        enumops_first(ops, n-1);
        do {
          res = evalast(&ast, comb.nums, ops, str);
          if (res) {
            struct hash_node *node;
            node = hash_lookup(res);
            if (!node) {
              node = malloc(sizeof(*node));
              node->res = res;
              node->n = 255;
              hash_add(node);
            }
            if (node->n > n) {
              node->n = n;
              strncpy(node->str, str, sizeof(node->str)-1);
            }
          }
        } while (enumops_next(ops, n-1));
      } while (astenum_next(&ast));
    } while (comb_next(&comb));
    fprintf(stderr, "\n");
  }
  
  hash_dump();
  hash_free();

  return 0;
}