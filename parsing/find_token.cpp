#include <vector>

#include "token.hpp"
#include "tokenizer.hpp"

enum class e_state {
  error,
  start,
  whitespaces,
  operators,
  parenthesis,
  zero,
  nonzero_digits,
  floating,
  word,
};

enum class e_char {
  invalid,
  whitespace,
  parenthesis,
  operators,
  zero,
  nonzero_digit,
  dot,
  character,
};

#include "char_table.hpp"

static e_state
get_next_state(e_state state, e_char char_type)
{
  switch (state) {
    case e_state::error:
      return state;

    case e_state::start:
      switch (char_type) {
        case e_char::whitespace:
          return e_state::whitespaces;
        case e_char::operators:
          return e_state::operators;
        case e_char::parenthesis:
          return e_state::parenthesis;
        case e_char::zero:
          return e_state::zero;
        case e_char::nonzero_digit:
          return e_state::nonzero_digits;
        case e_char::character:
          return e_state::word;
        default:
          return e_state::error;
      };

    case e_state::whitespaces:
      if (char_type == e_char::whitespace) {
        return state;
      }
      return e_state::error;

    case e_state::operators:
      return e_state::error;

    case e_state::parenthesis:
      return e_state::error;

    case e_state::zero:
      if (char_type == e_char::dot) {
        return e_state::floating;
      }
      return e_state::error;

    case e_state::nonzero_digits:
      switch (char_type) {
        case e_char::dot:
          return e_state::floating;
        case e_char::zero:
        case e_char::nonzero_digit:
          return e_state::nonzero_digits;
        default:
          return e_state::error;
      };

    case e_state::floating:
      if (char_type == e_char::nonzero_digit ||
          char_type == e_char::zero) {
        return e_state::floating;
      }
      return e_state::error;

    case e_state::word:
      if (char_type == e_char::character) {
        return e_state::word;
      }
      return e_state::error;
  };
}

static bool
is_accepted(e_state state)
{
  switch (state) {
    case e_state::whitespaces:
    case e_state::operators:
    case e_state::parenthesis:
    case e_state::zero:
    case e_state::nonzero_digits:
    case e_state::floating:
    case e_state::word:
      return true;
    default:
      return false;
  }
}

static e_state
fsm(std::string_view::iterator& itr)
{
  // TODO: maximal munch scanner
  // TODO: stores position of failure and use it to handle failure efficiently
  std::vector<e_state> stack{e_state::error};
  e_state state = e_state::start;
  e_char  char_type;

  while (state != e_state::error) {
    char_type = char_table[*itr++];
    state = get_next_state(state, char_type);
    if (is_accepted(state)) {
      stack.clear();
    }
    stack.push_back(state);
  }
  while (!is_accepted(state) && state != e_state::error) {
    state = stack.back();
    stack.pop_back();
    --itr;
  }
  if (is_accepted(state)) {
    return state;
  } else {
    return e_state::error;
  }
}

parsing::token
parsing::tokenizer::find_token(std::string_view input) const
{
  std::string_view::iterator  begin;
  std::string_view::iterator  end;
  token::type token_type;
  e_state     cur_state;

  while (true) {
    begin = input.begin();
    end = begin;
    cur_state = fsm(end);
    switch (cur_state) {
      // NOTE: handle whitespaces in different loop?
      case e_state::whitespaces:
        input = input.substr(end - begin, input.npos);
        continue;
      case e_state::operators:
        token_type = token::type::op;
        break;
      case e_state::parenthesis:
        token_type = token::type::parenthesis;
        break;
      case e_state::zero:
        token_type = token::type::zero;
        break;
      case e_state::nonzero_digits:
        token_type = token::type::nonzero_digit;
        break;
      case e_state::floating:
        token_type = token::type::floating;
        break;
      case e_state::word:
        token_type = token::type::word;
        break;
      case e_state::start: /* fall-through */
      case e_state::error:
        return token{};
    }
    return parsing::token{std::string_view(input.data(), end - begin), token_type};
  }
  return token{};
}
