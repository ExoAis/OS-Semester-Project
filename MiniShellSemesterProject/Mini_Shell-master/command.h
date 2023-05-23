struct tree {
  enum { NONE = 0, AND, OR, SEMI, PIPE, SUBSHELL } conjunction;
  struct tree *left, *right;
  char **argv;
  char *input;
  char *output;
};

/* Array of strings to match the conjunction enum above.
   print the element using conj[tree->conjunction] */
/* Defined here since we might change the operations. */
   
static const char *conj[] __attribute__((unused)) = { "err", "&&", "||", ";", "|", "()" };
