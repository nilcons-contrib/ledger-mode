#ifndef _EXPR_H
#define _EXPR_H

#include "journal.h"
#include "value.h"
#include "error.h"

namespace ledger {

class mask_t
{
 public:
  bool        exclude;
  std::string pattern;
  void *      regexp;

  explicit mask_t(const std::string& pattern);
  mask_t(const mask_t&);
  ~mask_t();

  bool match(const std::string& str) const;
};

//////////////////////////////////////////////////////////////////////

struct details_t
{
  const entry_t *	entry;
  const transaction_t * xact;
  const account_t *     account;

  details_t(const entry_t& _entry)
    : entry(&_entry), xact(NULL), account(NULL) {}
  details_t(const transaction_t& _xact)
    : entry(_xact.entry), xact(&_xact), account(_xact.account) {}
  details_t(const account_t& _account)
    : entry(NULL), xact(NULL), account(&_account) {}
};

struct value_expr_t
{
  enum kind_t {
    // Constants
    CONSTANT_I,
    CONSTANT_T,
    CONSTANT_A,

    // Item details
    AMOUNT,
    COST,
    DATE,
    CLEARED,
    REAL,
    INDEX,
    DEPTH,

    // Item totals
    COUNT,
    TOTAL,
    COST_TOTAL,

    // Relating to format_t
    VALUE_EXPR,
    TOTAL_EXPR,

    // Functions
    F_ARITH_MEAN,
    F_VALUE,
    F_NEG,
    F_ABS,
    F_STRIP,
    F_PAYEE_MASK,
    F_ACCOUNT_MASK,
    F_SHORT_ACCOUNT_MASK,
    F_INTERP_FUNC,

    // Binary operators
    O_ADD,
    O_SUB,
    O_MUL,
    O_DIV,
    O_EQ,
    O_LT,
    O_LTE,
    O_GT,
    O_GTE,
    O_NOT,
    O_AND,
    O_OR,
    O_QUES,
    O_COL,
    O_ARG,

    LAST
  };

  kind_t	 kind;
  value_expr_t * left;
  value_expr_t * right;

  union {
    std::time_t	 constant_t;
    unsigned int constant_i;
  };
  std::string    constant_s;
  amount_t       constant_a;
  mask_t *	 mask;

  value_expr_t(const kind_t _kind)
    : kind(_kind), left(NULL), right(NULL), mask(NULL) {
    DEBUG_PRINT("ledger.memory.ctors", "ctor value_expr_t");
  }

  ~value_expr_t() {
    DEBUG_PRINT("ledger.memory.dtors", "dtor value_expr_t");
    if (mask)  delete mask;
    if (left)  delete left;
    if (right) delete right;
  }

  void compute(value_t& result, const details_t& details) const;
};

value_expr_t * parse_value_expr(std::istream& in,
				const bool partial = false);

inline value_expr_t * parse_value_expr(const char * p,
				       const bool partial = false) {
  std::istringstream stream(p);
  return parse_value_expr(stream, partial);
}

inline value_expr_t * parse_value_expr(const std::string& str,
				       const bool partial = false) {
  return parse_value_expr(str.c_str());
}

#ifdef DEBUG_ENABLED
void dump_value_expr(std::ostream& out, const value_expr_t * node);
#endif

//////////////////////////////////////////////////////////////////////

template <typename T>
class item_predicate
{
  const value_expr_t * predicate;

 public:
  item_predicate(const std::string& _predicate) : predicate(NULL) {
    DEBUG_PRINT("ledger.memory.ctors", "ctor item_predicate<T>");
    if (! _predicate.empty()) {
      try {
	predicate = parse_value_expr(_predicate);
      }
      catch (value_expr_error& err) {
	throw value_expr_error(std::string("In predicate '") +
			       _predicate + "': " + err.what());
      }
    }
  }
  item_predicate(const value_expr_t * _predicate)
    : predicate(_predicate) {
    DEBUG_PRINT("ledger.memory.ctors", "ctor item_predicate<T>");
  }

  ~item_predicate() {
    DEBUG_PRINT("ledger.memory.dtors", "dtor item_predicate<T>");
    if (predicate)
      delete predicate;
  }

  bool operator()(const T& item) const {
    if (predicate) {
      value_t result;
      predicate->compute(result, details_t(item));
      return result;
    } else {
      return true;
    }
  }
};

} // namespace report

#endif // _REPORT_H
