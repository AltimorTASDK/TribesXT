#pragma once

#define LPAREN (
#define RPAREN )
#define PARENS ()

#define EVAL1(x) x
#define EVAL4(x) EVAL1(EVAL1(EVAL1(EVAL1(x))))
#define EVAL16(x) EVAL4(EVAL4(EVAL4(EVAL4(x))))
#define EVAL64(x) EVAL16(EVAL16(EVAL16(EVAL16(x))))
#define EVAL(x) EVAL64(EVAL64(EVAL64(EVAL64(x))))

#define CONCAT1_(a, ...) a##__VA_ARGS__
#define CONCAT1(a, ...) CONCAT1_(a, __VA_ARGS__)
#define IF_EMPTY_(a, b) a
#define IF_EMPTY_NOT_EMPTY_(a, b) b
#define IF_EMPTY(...) CONCAT1(IF_EMPTY_, __VA_OPT__(NOT_EMPTY_))
#define FALLBACK(a, b) IF_EMPTY(a)(b, a)

#define CONCAT__() CONCAT_
#define CONCAT_(a, b, ...) __VA_OPT__(CONCAT__ PARENS LPAREN) a##b __VA_OPT__(, __VA_ARGS__ RPAREN)
#define CONCAT(a, ...) EVAL16(FALLBACK(__VA_OPT__(CONCAT_(a, __VA_ARGS__)), a))

#define FORCE_SEMICOLON static_assert(true)
