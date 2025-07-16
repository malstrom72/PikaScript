#include <math.h>
#include <algorithm>
#include <time.h>
#include "PikaScript.h"

namespace Pika {

/* --- Utility routines --- */

static const Char ESCAPE_CHARS[] = { '\\', '\"', '\'',  'a',  'b',  'f',  'n',  'r',  't',  'v' };
static const Char ESCAPE_CODES[] = { '\\', '\"', '\'', '\a', '\b', '\f', '\n', '\r', '\t', '\v' };

unsigned long hex2long(StringIt& p, const StringIt& e) {
	unsigned long l = 0;
	for (; p < e && (*p >= '0' && *p <= '9' || *p >= 'A' && *p <= 'F' || *p >= 'a' && *p <= 'f'); ++p) {
		l = (l << 4) + (*p <= '9' ? *p - '0' : (*p & ~0x20) - ('A' - 10));
	}
	return l;
}

long string2long(StringIt& p, const StringIt& e) {
	bool negative = (p < e && p + 1 < e && *p == '-' ? ++p, true : false);
	long l = 0;
	for (; p < e && *p >= '0' && *p <= '9'; ++p) l = l * 10 + (*p - '0');
	return negative ? -l : l;
}

double string2double(StringIt& p, const StringIt& e) {
	double d = 0, sign = (p < e && p + 1 < e && *p == '-' ? ++p, -1.0 : 1.0);
	if (p < e && *p >= '0' && *p <= '9') {
		if (*p == '0') ++p; else { do d = d * 10.0 + (*p - '0'); while (++p < e && *p >= '0' && *p <= '9'); }
		if (p < e && p + 1 < e && *p == '.' && p[1] >= '0' && p[1] <= '9') {
			++p;
			double f = 1.0;
			do d += (*p - '0') * (f *= 0.1); while (++p < e && *p >= '0' && *p <= '9');
		}
		if (p < e && p + 1 < e && (*p == 'E' || *p == 'e')) {
			d *= pow(10, static_cast<double>(string2long(p += (p + 2 < e && p[1] == '+' ? 2 : 1), e)));
		}
	}
	return d * sign;
}

bool string2double(const String& s, double& d) {
	StringIt b = s.begin(), e = s.end(), p = b;
	d = string2double(p, e);
	return (p != b && p >= e);
}

String double2string(double d, int precision, bool trim) {
	assert(1 <= precision && precision <= 24);

	if (trim && static_cast<long>(d) == d) return int2string<long>(static_cast<long>(d));

	const double epsilon = 1.0E-300, small = 1.0E-6, large = 1.0E+12;	
	Char buffer[32], * bp = buffer + 2, * dp = bp, * pp = dp + 1, * ep = pp + precision;

	double x = fabs(d), y = x;
	for (; x >= 10.0 && pp < ep; x *= 0.1) { ++pp; }							// Normalize values > 10 and move period position.
	if (pp >= ep || y < small || y > large) {									// Exponential treatment of very small or large values.
		if (y < epsilon) return STR("0");
		double e = floor(log10(y) + 1.0E-10);
		String exps(e >= 0 ? String(STR("e+")) : String(STR("e")));
		exps += int2string<long>(static_cast<long>(e));
		int maxp = 15;															// Limit precision because of rounding errors in log10 etc
		for (double f = abs(e); f >= 8; f /= 10) { --maxp; }
		return (double2string(d * pow(0.1, e), std::min(maxp, precision), trim) += exps);
	}

	for (; x < 1.0 && dp < buffer + 32; ++ep, x *= 10.0) {						// For values < 0, spit out leading 0's and increase precision.
		*dp++ = '0';
		if (dp == pp) *dp++ = '9';												// Hop over period position (set to 9 to avoid when eliminating 9's).
	}
	for (; dp < ep; ) {															// Exhaust all remaining digits of mantissa into buffer.
		unsigned int ix = static_cast<unsigned int>(x);
		*dp++ = ix + '0';
		if (dp == pp) *dp++ = '9';												// Hop over period position (set to 9 to avoid when eliminating 9's).
		x = (x - ix) * 10.0;
	}
	if (x >= 5) {																// If remainder is >= 5, increment trailing 9's...
		while (dp[-1] == '9') { *--dp = '0'; }
		if (dp == bp) *--bp = '1';												// If we are at spare position, set to '1' and include.
		else dp[-1]++;															// ... otherwise, increment last non-9.
		if (dp[-1] == '1') --ep;												// We incremented a 0, cut precision (afterwards, a bit Q&D).
	}
	*pp = '.';
	if (trim && ep > pp) { while (ep[-1] == '0') { --ep; } }
	if (ep - 1 == pp) --ep;
	if (d < 0) *--bp = '-';	
	return String(bp, ep - bp);
}

String unescape(StringIt& p, const StringIt& e)
{
	assert(p < e && (*p == '\'' || *p == '"'));
	String d;
	StringIt l = ++p;
	if (p[-1] == '\'') {
		while (true) {
			if ((p = std::find(p, e, '\'')) + 1 < e && p[1] == '\'') {
				d += String(l, ++p);
				l = ++p;
			} else break;
		}
	} else {
		while (p < e && *p != '\"') {
			if (*p != '\\') ++p;
			else {
				d += String(l, p);
				const Char* findEnd = ESCAPE_CHARS + sizeof (ESCAPE_CHARS) / sizeof (*ESCAPE_CHARS);
				const Char* f = std::find(ESCAPE_CHARS, findEnd, *++p);
				if (f != findEnd) d += (++p, ESCAPE_CODES[f - ESCAPE_CHARS]);
				else if (*p == 'x') { ++p; d += static_cast<Char>(hex2long(p, std::min(p + 2, e))); }
				else if (*p == 'u') { ++p; d += static_cast<Char>(hex2long(p, std::min(p + 4, e))); }
				else d += static_cast<Char>(string2long(p, e));
				l = p;
			}
		}
	}
	if (p >= e) throw Exception(STR("Unterminated string")); 
	return (d += String(l, p++));
}

String escape(const String& s)
{
	String d;
	StringIt b = s.begin(), e = s.end();
	bool needToBackup = false;
	while (b < e && *b >= 32 && *b <= 126 && *b != '\'') {
		needToBackup = needToBackup || (*b == '\\' || *b == '\"');
		++b;
	}
	if (b >= e) return ((String(STR("'")) += s) += '\'');
	
	if (needToBackup) b = s.begin();
	StringIt l = s.begin();
	d = STR("\"");
	while (true) {
		while (b < e && *b >= 32 && *b <= 126 && *b != '\\' && *b != '\"') ++b;
		d += String(l, b);
		if (b >= e) break;
		const Char* findEnd = ESCAPE_CODES + sizeof (ESCAPE_CODES) / sizeof (*ESCAPE_CODES);
		const Char* f = std::find(ESCAPE_CODES, ESCAPE_CODES + sizeof (ESCAPE_CODES) / sizeof (*ESCAPE_CODES), *b);
		if (f != findEnd) (d += '\\') += ESCAPE_CHARS[f - ESCAPE_CODES];
		else if (static_cast<unsigned char>(*b) == charcode(*b)) (d += STR("\\x")) += int2string<unsigned long>(charcode(*b), 16, 2);
		else (d += STR("\\u")) += int2string<unsigned long>(charcode(*b), 16, 4);
		l = ++b;
	}
	return (d += '\"');
}

static bool isSymbolChar(Char c) { return c == '_' || c >= '0' && c <= '9' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'; }
static unsigned long shiftRight(unsigned long l, int r) { return l >> r; }
static unsigned long shiftLeft(unsigned long l, int r) { return l << r; }
static unsigned long bitwiseAnd(unsigned long l, unsigned long r) { return l & r; }
static unsigned long bitwiseOr(unsigned long l, unsigned long r) { return l | r; }
static unsigned long bitwiseXor(unsigned long l, unsigned long r) { return l ^ r; }

/* --- Exception --- */

Exception::Exception(const String& error) : error(error) { }
const char* Exception::what() const throw() { cvt = std::string(error.c_str(), error.c_str() + error.size()); return static_cast<const char*>(cvt.c_str()); }
Exception::~Exception() throw() { } // GCC requires explicit destructor with one that has throw().

/* --- Value --- */

Value::operator bool() const {
	if (*this == STR("false")) return false;
	else if (*this == STR("true")) return true;
	else throw Exception(String(STR("Invalid boolean: ")) += escape(*this));
}

Value::operator long() const {
	StringIt p = begin();
	long y = string2long(p, end());
	if (p == begin() || p < end()) throw Exception(String(STR("Invalid integer: ")) += escape(*this));
	return y;
}

Value::operator double() const {
	double d;
	if (!string2double(*this, d)) throw Exception(String(STR("Invalid number: ")) += escape(*this));
	return d;
}

bool operator<(const Value& l, const Value& r) {
	double lv, rv;
	bool lnum = string2double(l, lv), rnum = string2double(r, rv);
	return (lnum == rnum ? (lnum ? lv < rv : (const String&)(l) < (const String&)(r)) : lnum);
}

bool operator==(const Value& l, const Value& r) {
	double lv, rv;
	bool lnum = string2double(l, lv), rnum = string2double(r, rv);
	return (lnum == rnum && (lnum ? lv == rv : (const String&)(l) == (const String&)(r)));
}

Value Value::operator[](const Value& i) const {
	return (empty() || !isSymbolChar((*this)[size() - 1]) ? String(*this) : (String(*this) + STR('.'))) += i;
}

Context::Context() : maxCallDepth(200), autoLabelStart(autoLabel + 30), traceLevel(Frame::NO_TRACE_LEVEL)
		, isInsideTracer(false) {
	std::fill(autoLabel, autoLabel + 32, ':');
}

String Context::generateLabel() {
	Char* b = autoLabelStart, * p = autoLabel + 30;
	while (*--p == 'z') { };
	switch (*p) {
		case ':': *p = '1'; *--b = ':'; autoLabelStart = b; break;
		case '9': *p = 'A'; break;
		case 'Z': *p = 'a'; break;
		default: (*p)++; break;
	}
	for (++p; *p != ':'; ++p) *p = '0';
	return String(const_cast<const Char*>(b), autoLabel + 31 - b);
}

/* --- Frame --- */

Frame::Frame(Context& context) : context(context), previous(0), closure(this), global(this)
		, label(STR("::")), callDepth(0) {
}

Frame::Frame(Frame* previous)
		: context(previous->context), previous(previous), closure(this), global(previous->global)
		, label(context.generateLabel()), callDepth(previous->callDepth + 1) {
	if (callDepth >= context.maxCallDepth) throw Exception(STR("Call depth limit exceeded"));
}

Frame* Frame::resolveFrame(StringIt& b, const StringIt& e) {
	// FIX : necessary to check?
	if (b >= e) return closure;
	switch (*b) {
		default:	return closure;
		case '^':	if (previous == 0) throw Exception(STR("Frame does not exist"));
					return previous->resolveFrame(++b, e);
		case '$':	return this;
		case ':':	if (b + 1 < e && b[1] == ':') {
						b += 2;
						return global->resolveFrame(b, e);
					} else {
						StringIt p = std::find(b + 1, e, ':');
						if (p >= e) throw Exception(String(STR("Invalid identifier: ")) += escape(String(b, e)));
						String refLabel(b, p + 1);
						b = p + 1;
						for (Frame* f = this; f != 0; f = f->previous) if (f->label == refLabel) return f->resolveFrame(b, e);
						throw Exception(String(STR("Frame does not exist: ")) += escape(refLabel));
					}
	}
}

std::pair<Frame*, String> Frame::resolveFrame(const String& identifier) {
	StringIt b = const_cast<const String&>(identifier).begin(), e = const_cast<const String&>(identifier).end();
	Frame* frame = resolveFrame(b, e);
	return std::pair<Frame*, String>(frame, String(b, e));
}

Value Frame::get(const String& identifier, bool fallback) {
	Value temp;
	bool canFall = (fallback && !identifier.empty() && isSymbolChar(identifier[0]));
	std::pair<Frame*, String> fs = resolveFrame(identifier);
	if (fs.first->lookup(fs.second, temp)) return temp;
	else if (canFall && fs.first != global && global->lookup(fs.second, temp)) return temp;
	else throw Exception((fallback ? String(STR("Undefined: ")) : String(STR("Undefined / inappropriate: "))) += escape(identifier));
}

Value Frame::getOptional(const String& identifier, const Value& defaultValue) {
	std::pair<Frame*, String> fs = resolveFrame(identifier);
	Value temp;
	return fs.first->lookup(fs.second, temp) ? temp : defaultValue;
}

const Value& Frame::set(const String& identifier, const Value& v) {
	std::pair<Frame*, String> fs = resolveFrame(identifier);
	if (!fs.first->assign(fs.second, v)) {
		throw Exception(String(STR("Cannot modify: ")) += escape(identifier));
	}
	return v;
}

Value Frame::indirect(const String& identifier) {
	std::pair<Frame*, String> fs = resolveFrame(identifier);
	return fs.first->label + fs.second;
}

Value Frame::rvalue(const XValue& v, bool fallback) { return (!v.first) ? v.second : get(v.second, fallback); }
const Value& Frame::lvalue(const XValue& v) { if (!v.first) throw Exception(STR("Invalid lvalue")); return v.second; }

void Frame::caught(const StringIt& p, const String& what) {
	if (context.traceLevel >= ERROR_LEVEL) {
		trace(*context.currentSource, p - context.currentSource->begin(), XValue(false, what), false, ERROR_LEVEL, previous == 0);
	}
}

void Frame::tick(const StringIt& p, const XValue& v, bool dry, Precedence thres, bool exit) {
	trace(*context.currentSource, p - context.currentSource->begin(), v, dry, thres, exit);
}
	
void Frame::white(StringIt& p, const StringIt& e) {
	while (p < e) {
		switch (*p) {
			case '\r':	if (p + 1 < e && p[1] == '\n') ++p;
			case ' ': case '\t': case '\v': case '\n': ++p; break;
			case '/':	if (p + 1 < e && p[1] == '/') {
							static const Char END_CHARS[] = { '\r', '\n' };
							p = std::find_first_of(p += 2, e, END_CHARS, END_CHARS + 2);
							break;
						} else if (p + 1 < e && p[1] == '*') {
							static const Char END_CHARS[] = { '*', '/' };
							p = std::search(p += 2, e, END_CHARS, END_CHARS + 2);
							if (p >= e) throw Exception(STR("Missing '*/'"));
							p += 2;
							break;
						}
			default:	return;
		}
	}
}

bool Frame::token(StringIt& p, const StringIt& e, const Char* token) {
	StringIt t = p;
	while (*token != 0 && (t < e && *t == *token)) { ++t; ++token; }
	if (*token != 0 || (t < e && isSymbolChar(*t))) return false;
	else {
		white(p = t, e);
		return true;
	}
}

template<class F> bool Frame::binaryOp(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres, int hop
		, Precedence prec, F op) {
	if (thres >= prec) return false;
	XValue r; expr(p += hop, e, r, false, dry, prec);
	if (!dry) v = XValue(false, Value(op(rvalue(v), rvalue(r))));
	return true;
}

template<class F> bool Frame::assignableOp(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres, int hop
		, Precedence prec, F op) {
	if (p + hop >= e || p[hop] != '=') return binaryOp(p, e, v, dry, thres, hop, prec, op);
	if (thres > ASSIGN_LEVEL) return false;
	XValue r; expr(p += hop + 1, e, r, false, dry, ASSIGN_LEVEL);
	if (!dry) v = XValue(false, set(lvalue(v), Value(op(rvalue(v, false), rvalue(r)))));
	return true;
}

template<class F> bool Frame::addSubOp(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres, const F& f) {
	if (p + 1 >= e || p[1] != *p) return assignableOp(p, e, v, dry, thres, 1, ADD_SUB_LEVEL, f);
	else if (thres >= POSTFIX_LEVEL) return false;
	else {
		if (!dry) {		// --- post inc/dec
			Value r = rvalue(v, false);
			set(lvalue(v), Value(f(static_cast<long>(r), 1)));
			v = XValue(false, r);
		}
		p += 2;
		return true;
	}
}

template<class E, class I, class S> bool Frame::lessGreaterOp(StringIt& p, const StringIt& e, XValue& v, bool dry
		, Precedence thres, const E& excl, const I& incl, const S& shift) {
	if (p + 1 < e && p[1] == *p) return assignableOp(p, e, v, dry, thres, 2, SHIFT_LEVEL, shift);						// --- shift
	else if (p + 1 < e && p[1] == '=') return binaryOp(p, e, v, dry, thres, 2, COMPARE_LEVEL, incl);		// --- less/greater or equal
	else return binaryOp(p, e, v, dry, thres, 1, COMPARE_LEVEL, excl);								// --- less/greater
}

bool Frame::pre(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres) {
	white(p, e);
	if (p >= e) return false;
	StringIt b = p;
	if (context.traceLevel >= thres) tick(p, v, dry, thres, false);

	switch (*p) {
		case '!':	expr(++p, e, v, false, dry, PREFIX_LEVEL); if (!dry) v = XValue(false, Value(!rvalue(v))); return true;							// --- logical not
		case '~':	expr(++p, e, v, false, dry, PREFIX_LEVEL); if (!dry) v = XValue(false, Value(~static_cast<unsigned long>(rvalue(v)))); return true;			// --- bitwise not
		case '(':	termExpr(++p, e, v, false, dry, PARENTHESIS_LEVEL, ')'); return true;													// --- parenthesis
		case 't':	if (token(p, e, STR("true"))) { if (!dry) v = XValue(false, String(STR("true"))); return true; } break;					// --- true literal
		case 'v':	if (token(p, e, STR("void"))) { if (!dry) v = XValue(false, String()); return true; } break;					// --- true literal
		case '<':	p = std::find(p, e, '>'); if (p < e) ++p; if (!dry) v = XValue(false, String(b, p)); return true;
		case ':':	if (p + 1 < e && p[1] == ':') p += 2; break;	// --- global
		case '^':	while (p + 1 < e && *(++p) == '^') { }; if (p >= e || *p != '$') break;
		case '$':	++p; break;
		case '>':	white(++p, e); b = p; expr(p, e, v, false, true, STATEMENT_LEVEL); if (!dry) v = XValue(false, (String(STR(">")) += closure->label) += String(b, p)); return true; // --- indirect
		case '@':	expr(++p, e, v, false, dry, PREFIX_LEVEL); if (!dry) v = XValue(false, indirect(lvalue(v))); return true; // --- reference

		case '[':	termExpr(++p, e, v, false, dry, PARENTHESIS_LEVEL, ']');						// --- indirection
					if (!dry) v = XValue(true, rvalue(v));
					return true;
					
		case '+': case '-':
					if (p + 1 >= e || p[1] != *b) {
						expr(++p, e, v, false, dry, PREFIX_LEVEL);															// --- add/sub
						if (!dry) v = XValue(false, Value(*b == '+' ? static_cast<double>(rvalue(v)) : -static_cast<double>(rvalue(v))));
					} else {
						expr(p += 2, e, v, false, dry, PREFIX_LEVEL);															// --- pre inc/dec
						if (!dry) v = XValue(false, set(lvalue(v), Value(static_cast<long>(rvalue(v, false)) + (*b == '+' ? 1 : -1))));
					}
					return true;
					
		case '\'': case '"': { Value s = unescape(p, e); if (!dry) v = XValue(false, s); } return true;																			// --- string literal
		
		case '{':	do { expr(++p, e, v, true, dry, STATEMENT_LEVEL); } while (p < e && *p == ';');					// --- compound
					if (p >= e || *p != '}') throw Exception(p >= e ? String(STR("Missing '}'")) : String(STR("Missing ';'?")));
					++p;
					return true;
		
		case '0':	if (p + 1 < e && p[1] == 'x') {
						long l = hex2long(p += 2, e);
						if (p == b + 2) throw Exception(STR("Invalid hexadecimal number"));
						if (!dry) v = XValue(false, Value(l));
						return true;
					} /* else continue */
		
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
						double d = string2double(p, e);																		// --- numeric literal
						if (p == b) throw Exception(STR("Invalid number"));
						if (!dry) v = XValue(false, Value(d));
					}
					return true;
					
		case 'e':	if (token(p, e, STR("else"))) throw Exception(STR("Unexpected 'else' (preceded by ';'?)"));
					break;
					
		case 'f': 	if (token(p, e, STR("false"))) {
						if (!dry) v = XValue(false, String(STR("false")));			// --- false literal
						return true;
					} else if (token(p, e, STR("for"))) {
						if (p >= e || *p != '(') throw Exception(STR("Expected '('"));	
						XValue c; termExpr(++p, e, v, true, dry, PARENTHESIS_LEVEL, ';');
						StringIt cp = p; termExpr(p, e, c, true, dry, STATEMENT_LEVEL, ';');
						StringIt ip = p; termExpr(p, e, v, true, true, STATEMENT_LEVEL, ')');
						StringIt bp = p;
						bool cb = !dry && rvalue(c);
						do {
							expr(p = bp, e, v, true, !cb, BODY_LEVEL);
							if (cb) {
								StringIt ep = p;
								expr(p = ip, e, v, true, false, STATEMENT_LEVEL);
								expr(p = cp, e, c, true, false, STATEMENT_LEVEL);
								p = ep;
								cb = !dry && rvalue(c);
							}
						} while (cb);
						return true;
					} else if (token(p, e, STR("function"))) {
						if (p >= e || *p != '{') throw Exception(STR("Expected '{'"));
						b = p;
						expr(p, e, v, false, true, DEFINITION_LEVEL);						// --- function
						if (!dry) v = XValue(false, String(b, p));
						return true;
					}
					break;

		case 'i':	if (token(p, e, STR("if"))) {
						if (p >= e || *p != '(') throw Exception(STR("Expected '('"));														// --- if
						XValue c;
						termExpr(++p, e, c, false, dry, PARENTHESIS_LEVEL, ')');
						bool b = dry || rvalue(c);
						expr(p, e, v, true, dry || !b, BODY_LEVEL);
						if (token(p, e, STR("else"))) expr(p, e, v, true, dry || b, BODY_LEVEL);	// --- else
						return true;
					}
					break;
	}
	while (p < e && isSymbolChar(*p)) ++p;
	if (b != p && !dry) v = XValue(true, String(b, p));
	return (b != p);
}

bool Frame::post(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres) {
	white(p, e);
	if (p >= e) return false;
	
	switch (*p) {
		case '+':	return addSubOp(p, e, v, dry, thres, std::plus<double>());											// --- add
		case '-':	return addSubOp(p, e, v, dry, thres, std::minus<double>());											// --- subtract
		case '#':	return assignableOp(p, e, v, dry, thres, 1, CONCAT_LEVEL, std::plus<String>());								// --- concat
		case '*':	return assignableOp(p, e, v, dry, thres, 1, MUL_DIV_LEVEL, std::multiplies<double>());							// --- multipy
		case '/':	return assignableOp(p, e, v, dry, thres, 1, MUL_DIV_LEVEL, std::divides<double>());							// --- divide
		case '%':	return assignableOp(p, e, v, dry, thres, 1, MUL_DIV_LEVEL, std::ptr_fun<double, double>(fmod));				// --- modulus
		case '^':	return assignableOp(p, e, v, dry, thres, 1, BIT_XOR_LEVEL, bitwiseXor);											// --- xor
		case '<':	return lessGreaterOp(p, e, v, dry, thres, std::less<Value>(), std::less_equal<Value>(), shiftLeft);
		case '>':	return lessGreaterOp(p, e, v, dry, thres, std::greater<Value>(), std::greater_equal<Value>(), shiftRight);
		
		// FIX : minimize
		case '!':	if (p + 2 < e && p[2] == '=' && p[1] == '=') return binaryOp(p, e, v, dry, thres, 3, EQUALITY_LEVEL, std::not_equal_to<String>());	// --- exact not equals
					else if (p + 1 < e && p[1] == '=') return binaryOp(p, e, v, dry, thres, 2, EQUALITY_LEVEL, std::not_equal_to<Value>());				// --- not equals
					break;

		case '=':	if (p + 2 < e && p[2] == '=' && p[1] == '=') return binaryOp(p, e, v, dry, thres, 3, EQUALITY_LEVEL, std::equal_to<String>());	// --- exact equals
					else if (p + 1 < e && p[1] == '=') return binaryOp(p, e, v, dry, thres, 2, EQUALITY_LEVEL, std::equal_to<Value>());				// --- equals
					else if (thres <= ASSIGN_LEVEL) {
						XValue r; expr(++p, e, r, false, dry, ASSIGN_LEVEL);														// --- assign
						if (!dry) v = XValue(false, set(lvalue(v), rvalue(r)));
						return true;
					}
					break;

		case '&':	if (p + 1 < e && p[1] != '&') return assignableOp(p, e, v, dry, thres, 1, BIT_AND_LEVEL, bitwiseAnd);			// --- bitwise and
					else if (thres < LOGICAL_AND_LEVEL) {
						bool l = !dry && rvalue(v);																		// --- logical and
						expr(p += 2, e, v, false, !l, LOGICAL_AND_LEVEL);
						if (!dry) v = XValue(false, Value(l && rvalue(v)));
						return true;
					}
					break;
		
		case '|': 	if (p + 1 < e && p[1] != '|') return assignableOp(p, e, v, dry, thres, 1, BIT_OR_LEVEL, bitwiseOr);			// --- bitwise or
					else if (thres < LOGICAL_OR_LEVEL) {
						bool l = dry || rvalue(v);																		// --- logical or
						expr(p += 2, e, v, false, l, LOGICAL_OR_LEVEL);
						if (!dry) v = XValue(false, Value(l || rvalue(v)));
						return true;
					}
					break;
		
		case '.': case '[':
					if (thres < POSTFIX_LEVEL) {																					// --- subscript
						XValue element;
						Char c = *p;
						if (c == '.') expr(++p, e, element, false, dry, POSTFIX_LEVEL);
						else termExpr(++p, e, element, false, dry, PARENTHESIS_LEVEL, ']');
						if (!dry) v = XValue(true, lvalue(v)[c == '.' ? lvalue(element) : rvalue(element)]);
						return true;
					}
					break;
					
		case '{':	if (thres < POSTFIX_LEVEL) {																					// --- substring
						XValue index;
						bool gotIndex = expr(++p, e, index, true, dry, PARENTHESIS_LEVEL);
						if (p >= e || (*p != ':' && *p != '}')) throw Exception(STR("Expected '}' or ':'"));
						if (*p++ == ':') {
							XValue count;
							bool gotCount = termExpr(p, e, count, true, dry, PARENTHESIS_LEVEL, '}');
							if (!dry) {
								String s = rvalue(v);
								long i = !gotIndex ? 0L : static_cast<long>(rvalue(index));
								long n = gotCount ? static_cast<long>(rvalue(count)) + std::min(i, 0L) : Value::npos;
								v = XValue(false, i <= static_cast<long>(s.size()) && (n == Value::npos || n >= 0L)
										? s.substr(std::max(i, 0L), n) : Value());
							}
						} else if (!gotIndex) throw Exception(STR("Syntax error"));
						else if (!dry) {
							String s = rvalue(v);
							long i = static_cast<long>(rvalue(index));
							v = XValue(false, i >= 0L && i <= static_cast<long>(s.size()) ? s.substr(i, 1) : Value());
						}
						return true;
					}
					break;

		case '(': 	if (thres < POSTFIX_LEVEL) { makePikaCall(p, e, v, dry, thres); return true; } break;							// --- call
	}
	return false;
}

bool Frame::expr(StringIt& p, const StringIt& e, XValue& v, bool emptyOk, bool dry, Precedence thres) {
	if (!pre(p, e, v, dry, thres)) {
		if (!emptyOk) throw Exception(STR("Syntax error"));
		return false;
	}
	while (post(p, e, v, dry, thres)) { };
	if (context.traceLevel >= thres) tick(p, v, dry, thres, true);
	return true;
}

bool Frame::termExpr(StringIt& p, const StringIt& e, XValue& v, bool emptyOk, bool dry, Precedence thres, Char term) {
	bool nonEmpty = expr(p, e, v, emptyOk, dry, thres);
	if (p >= e || *p != term) throw Exception((String(STR("Missing '")) += String(&term, 1)) += '\'');
	++p;
	return nonEmpty;
}

Value Frame::evaluateCompiled(const String code) {
	Value result;
	XValue v;
	StringIt p = code.begin(), e = code.end();
	while (p < e) {
		switch (*p++) {
			case 0: return v;
			case '#': assert(s < stack + 256); *s++ = v; v = constants[readInt(p)]; break;
			case '!': v = Var(!v); break;
			case '~': v = Var(~static_cast<unsigned long>(v)); break;
			case '+': v = *--s + v; break;
			case '-': v = Var((double)(*--s) - (double)(v)); break;
			case '*': v = Var((double)(*--s) * (double)(v)); break;
			case '/': v = Var((double)(*--s) / (double)(v)); break;
			case '%': v = Var(fmod(*--s, v)); break;
			case '&': v = Var((unsigned long)(*--s) & (unsigned long)(v)); break;
			case '|': v = Var((unsigned long)(*--s) | (unsigned long)(v)); break;
			case '^': v = Var((unsigned long)(*--s) ^ (unsigned long)(v)); break;
			case 'r': v = Var((unsigned long)(*--s) >> (unsigned long)(v)); break;
			case 'l': v = Var((unsigned long)(*--s) << (unsigned long)(v)); break;
			case '<': v = Var(*--s < v); break;
			case '=': v = Var(*--s == v); break;
			case 'R': v = realize(v); break;
			case 'W': assign(*--s, v); break;
			case '[': v = *level(v, thisFrameIx); break;
			case '@': v = Var::indirect(v); break;
//                      case '.': v = (*--s)[v]; break;
			case '?': assert(s > stack); --s; if (v) { int hop = readInt(p); p += hop; } else { p += 4; } break;
			case 'J': assert(s > stack); --s; { int hop = readInt(p); p += hop; } break;
			case '(': s -= *p; v = call(v, *p, s); ++p; break;
			case ';': assert(s > stack); --s; break;
			default: assert(0); break;
		}
	}
	result = rvalue(v);
	return result;
}

Value Frame::evaluate(const String source) {
	Value result;
	const String* oldSource = context.currentSource;
	context.currentSource = &source;
	try {
		XValue v;
		StringIt p = source.begin(), e = source.end();
		if (context.traceLevel >= CALL_LEVEL) tick(p, v, false, CALL_LEVEL, false);
		try {
			try {
				while (p < e) {
					expr(p, e, v, true, false, STATEMENT_LEVEL);
					if (p < e) {
						if (*p != ';') throw Exception(STR("Syntax error"));
						++p;
					}
				}
				result = rvalue(v);
			} catch (const Exception& x) { caught(p, x.error); throw; }
			catch (const std::exception& x) { const char* s = x.what(); caught(p, String(std::basic_string<Char>(s, s + strlen(s)))); throw; }
			catch (...) { caught(p, STR("Unknown exception")); throw; }
		} catch (...) { if (context.traceLevel >= CALL_LEVEL) tick(p, v, false, CALL_LEVEL, true); throw; }
		if (context.traceLevel >= CALL_LEVEL) { v = XValue(false, result); tick(p, v, false, CALL_LEVEL, true); }
	} catch (...) { context.currentSource = oldSource; throw; }
	context.currentSource = oldSource;
	return result;
}

void Frame::registerNative(const String& identifier, Native* native) {
	std::pair<Frame*, String> fs = resolveFrame(identifier);
	if (!fs.first->assignNative(fs.second, native)) {
		throw Exception(String(STR("Cannot register native: ")) += escape(identifier));
	}
	fs.first->set(fs.second, (String(STR("<")) += (fs.first == global ? fs.second : fs.first->label + fs.second)) += '>');
}

Context& Frame::getEnvironment() { return context; };

Value Frame::execute(Frame& inFrame, const String& body) {
	inFrame.assign(STR("$self"), body);
	switch (body[0]) {
		case '{':	return inFrame.evaluate(body);
		case '>':	{
						StringIt b = body.begin() + 1, e = body.end();
						inFrame.closure = resolveFrame(b, e);
						return inFrame.evaluate(String(b, e));
					}
		case '<':	{
						StringIt b = body.begin() + 1, e = body.end() - 1;
						if (b < e) {
							Frame* nativeFrame = (*b == ':' ? resolveFrame(b, e) : global);
							Native* native = nativeFrame->lookupNative(String(b, e));
							if (native != 0) return native->pikaCall(inFrame);
						}
						throw Exception(String(STR("Unknown native function: ")) += escape(body));
					}
		default:	throw Exception(String(STR("Illegal call on: ")) + escape(body));
	}
}

Value Frame::completeCppCall(Frame& inFrame, const String& func, long argc, const Value* argv, const Value& funcName
		, const Value& thisValue)
{
	for (long i = 0; i < argc; ++i) inFrame.assign(String(STR("$")) + Value(i), argv[i]);
	inFrame.assign(STR("$n"), Value(argc));
	if (!funcName.empty()) inFrame.assign(STR("$function"), funcName);
	if (!thisValue.empty()) inFrame.assign(STR("$this"), thisValue);
	return execute(inFrame, func);
}

void Frame::completePikaCall(Frame& inFrame, StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres)
{
	unsigned long n = 0;
	do {
		white(++p, e);
		if (p < e && *p == ')' && n == 0) break;
		XValue arg; expr(p, e, arg, true, dry, ARGUMENT_LEVEL);
		if (!dry) inFrame.assign(String(STR("$")) += Value(n), rvalue(arg));
		++n;
	} while (p < e && *p == ',');
	if (p >= e || *p != ')') throw Exception(STR("Expected ',' or ')'"));
	++p;
	if (!dry) {
		inFrame.assign(STR("$n"), Value(n));
		if (v.first) {
			inFrame.assign(STR("$function"), v.second);
			StringIt it = std::find(const_cast<const Value&>(v.second).rbegin(), const_cast<const Value&>(v.second).rend(), '.').base();
			if (it > const_cast<const Value&>(v.second).begin()) inFrame.assign(STR("$this"), indirect(String(static_cast<const String&>(v.second).begin(), --it)));
		}
		v = XValue(false, execute(inFrame, rvalue(v)));
	}
}

void Frame::trace(const String& source, String::size_type offset, const XValue& v, bool dry, Precedence thres, bool exit) {
	if (!dry && !context.tracerFunction.empty() && !context.isInsideTracer) {
		try {
			context.isInsideTracer = true;
			Value argv[6] = { source, Value(offset), Value(v.first), v.second, Value(static_cast<int>(thres)), Value(exit) };
			call(context.tracerFunction, 6, argv);
			context.isInsideTracer = false;
		} catch (...) {
			context.traceLevel = NO_TRACE_LEVEL;	// Turn off tracing on uncaught exceptions.
			context.isInsideTracer = false;
			throw;
		}
	}
}

/* --- STLMapFrame --- */

STLMapFrame::STLMapFrame(Context& context) : Frame(context) { };
STLMapFrame::STLMapFrame(Frame* previous) : Frame(previous) { };

int STLMapFrame::hash(const String& s) {
	int l = s.size();
	return (s[0] * 1733 + s[l >> 2] * 2069 + s[l >> 1] * 2377 + s[l - 1] * 2851) % CACHE_SIZE;
}

bool STLMapFrame::assign(const String& identifier, const Value& value) {
	if (identifier.empty()) return false;
	int i = hash(identifier);
	if (cache[i].first == identifier) { cache[i] = CacheEntry(identifier, value); return true; }
	if (!cache[i].first.empty()) variables[cache[i].first] = cache[i].second;
	cache[i] = CacheEntry(identifier, value);
	return true;
}

void STLMapFrame::erase(const String& identifier) {
	variables.erase(identifier);
	int i = hash(identifier);
	if (cache[i].first == identifier) cache[i] = std::pair<const String, Value>();
}

bool STLMapFrame::lookup(const String& identifier, Value& result) const {
	if (identifier.empty()) return false;
	int i = hash(identifier);
	if (cache[i].first == identifier) { result = cache[i].second; return true; }
	VariableMap::const_iterator it = variables.find(identifier);
	if (it == variables.end()) return false;
	if (!cache[i].first.empty()) variables[cache[i].first] = cache[i].second;
	cache[i] = CacheEntry(identifier, it->second);
	result = it->second;
	return true;
}

void STLMapFrame::list(const String& key, VarList& list) const {
	for (int i = 0; i < CACHE_SIZE; ++i) if (!cache[i].first.empty()) variables[cache[i].first] = cache[i].second;
	for (VariableMap::const_iterator it = variables.lower_bound(key)
			; it != variables.end() && it->first.substr(0, key.size()) == key; ++it) {
		list.push_back(*it);
	}
}

Native* STLMapFrame::lookupNative(const String& identifier) {
	NativeMap::iterator it = natives.find(identifier);
	return (it == natives.end() ? 0 : it->second);
}

bool STLMapFrame::assignNative(const String& name, Native* native) {	
	NativeMap::iterator it = natives.insert(NativeMap::value_type(name, 0)).first;
	if (it->second != native) delete it->second;
	it->second = native;
	return true;
}

Value STLMapFrame::call(const String& func, long argc, const Value* argv, const Value& funcName, const Value& thisValue) {
	STLMapFrame calleeFrame(this);
	return completeCppCall(calleeFrame, func, argc, argv, funcName, thisValue);
}

void STLMapFrame::makePikaCall(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres) {
	STLMapFrame calleeFrame(this);
	completePikaCall(calleeFrame, p, e, v, dry, thres);
}

STLMapFrame::~STLMapFrame() throw() {
	for (NativeMap::iterator it = natives.begin(); it != natives.end(); ++it) delete it->second;
}

Native::~Native() { }

/* --- Standard library --- */

void print(const String& s) { xputs(s.c_str()); xputs("\n"); }
String upper(String s) { for (String::size_type i = 0; i < s.size(); ++i) s.at(i) = toupper(s[i]); return s; }
String lower(String s) { for (String::size_type i = 0; i < s.size(); ++i) s.at(i) = tolower(s[i]); return s; }
double log2(double d) { return log(d) * 1.44269504088896340736; }
double round(double r) { return floor(r + 0.5); }
Value time(Frame&) { return Value(static_cast<double>(::time(0))); }
double random(double m) { return Value(m * ::rand() / static_cast<double>(RAND_MAX)); }
void thrower(const String& s) { throw Exception(s); }
String::size_type find(const String& a, const String& b) { return std::find_first_of(a.begin(), a.end(), b.begin(), b.end()) - a.begin(); }
String::size_type search(const String& a, const String& b) { return std::search(a.begin(), a.end(), b.begin(), b.end()) - a.begin(); }
Value evaluate(Frame& frame) { String s(frame.get("$0")); std::pair<Frame*, String> fs = frame.resolveFrame(s); return fs.first->evaluate(fs.second); }
Value exists(Frame& frame) { Value x = frame.get(STR("$0")); Value result; std::pair<Frame*, String> fs = frame.resolveFrame(x); return Value(fs.first->lookup(fs.second, result)); }
Value deleter(Frame& frame) { Value x = frame.get(STR("$0")); std::pair<Frame*, String> fs = frame.resolveFrame(x); fs.first->erase(fs.second); return Value(); }

String character(double d) {
	if (charcode(static_cast<Char>(d)) != d) throw Exception(String(STR("Illegal char code: ")) += Value(d));
	return String(1, static_cast<Char>(d));
}

unsigned int charcode(const String& s) {
	if (s.size() != 1) throw Exception(String(STR("Value is not single character: ")) += s);
	return charcode(s[0]);
}

String input(const String& prompt) {
	Char buffer[1024];
	xputs(prompt.c_str());
	if (xfgets(buffer, 1023, stdin) == 0) return String();
	size_t l = xstrlen(buffer);
	while (l > 0 && (buffer[l - 1] == '\n' || buffer[l - 1] == '\r')) buffer[l - 1] = 0;
	return String(buffer);
}

Value invoke(Frame& frame) {
	Value source = frame.get(STR("$1")), arg3 = frame.getOptional(STR("$3"));
	long offset = static_cast<long>(frame.getOptional(STR("$2"), Value(0)));
	std::vector<Value> a(arg3.empty() ? static_cast<long>(frame.get(source[String(STR("n"))])) - offset : static_cast<long>(arg3));
	for (long i = 0; i < static_cast<long>(a.size()); ++i) a[i] = frame.get(source[Value(i + offset)]);
	return frame.call(frame.get(STR("$0")), static_cast<long>(a.size()), a.empty() ? 0 : &a[0], frame.getOptional(STR("$4")), frame.getOptional(STR("$5")));
}

Value string(Frame& frame) {
	Value arg0 = frame.get(STR("$0")), arg1 = frame.getOptional(STR("$1"));
	return arg1.empty() ? arg0 : double2string(static_cast<double>(arg0)
			, std::min(std::max(static_cast<long>(arg1), 1L), 24L), frame.getOptional(STR("$2")) == Value(true));
}

Value trunc(Frame& frame) {
	double v = static_cast<double>(frame.get(STR("$0")));
	Value arg1 = frame.getOptional(STR("$1"));
	double m = !arg1.empty() ? pow(10.0, std::min(std::max(static_cast<long>(arg1), 0L), 23L)) : 1.0;
	return Value((v < 0 ? ceil(v * m) : floor(v * m)) / m);
}

Value tryer(Frame& frame) {
	try { frame.call(frame.get(STR("$0"))); }
	catch (const Exception& x) { return x.error; }
	catch (const std::exception& x) { const char* s = x.what(); return String(std::basic_string<Char>(s, s + strlen(s))); }
	catch (...) { return String(STR("Unknown exception")); }
	return Value();
}

Value foreach(Frame& frame) {
	Value arg1 = frame.get(STR("$1"));
	std::pair<Frame*, String> fs = frame.resolveFrame(frame.get(STR("$0"))[String()]); 
	Frame::VarList list;
	fs.first->list(fs.second, list);
	for (Frame::VarList::const_iterator it = list.begin(); it != list.end(); ++it) {
		Value argv[3] = { fs.first->indirect(it->first), it->first.substr(fs.second.size()), it->second };
		frame.call(arg1, 3, argv);
	}
	return Value();
}

String classify(const Value& v) {
	static const String STRING(STR("string"));
	switch (v[0]) {
		case 0: return (v.empty() ? String(STR("void")) : STRING);
		case 'f': return (v == STR("false") ? String(STR("boolean")) : STRING);
		case 't': return (v == STR("true") ? String(STR("boolean")) : STRING);
		case '{': if (v[v.size() - 1] != '}') return STRING; /* else continue */
		case '>': return STR("function");
		case ':': return (std::find(v.begin() + 1, v.end(), ':') != v.end()) ? String(STR("reference")) : STRING;
		case '<': return (v[v.size() - 1] == '>') ? String(STR("native")) : STRING;
		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9': case '-': {
			double d;
			if (string2double(v, d)) return STR("number");
		} /* else continue */
		default: return STRING;
	}
}

Value load(const String& file) {
	FILE* f = xfopen(file.c_str(), STR("rt"));
	if (f == 0) throw Exception(String(STR("Cannot open file for reading: ")) += file);
	Value chars;
	Char buffer[1024];
	while (xfgets(buffer, 1024, f) != 0) chars += Value(String(buffer));
	bool error = (ferror(f) != 0);
	fclose(f);
	if (error) throw Exception(String(STR("Error reading from file: ")) += file);
	return chars;
}

void save(const String& file, const String& chars) {
	FILE* f = xfopen(file.c_str(), STR("wt"));
	if (f == 0) throw Exception(String(STR("Cannot open file for writing: ")) += file);
	fwrite(chars.c_str(), 1, chars.size(), f);
	bool error = (ferror(f) != 0);
	fclose(f);
	if (error) throw Exception(String(STR("Error writing to file: ")) += file);
}

String::size_type span(const String& a, const String& b) {
	StringIt it;
	for (it = a.begin(); it != a.end() && std::find(b.begin(), b.end(), *it) != b.end(); ++it) { };
	return it - a.begin();
}

String reverse(String s) {
	String::size_type n = s.size();
 // FIX : implement normal iterators in qstring and / or rbegin, rend etc and use either std::reverse or String(s.rbegin(), s.rend())
 	for (String::size_type i = 0; i < n / 2; ++i) std::swap(s.at(i), s.at(n - i - 1));
	return s;
}

String unescape(const String& s) {
	StringIt p = s.begin(), e = s.end();
	if (p >= e || (*p != '"' && *p != '\'')) throw Exception(String(STR("Invalid string literal: ")) += escape(s));
	String r = unescape(p, e);
	if (p < e) throw Exception(String(STR("Invalid string literal: ")) += escape(s));
	return r;
}

Value radix(Frame& frame) {
	int radix = frame.get(STR("$1"));
	if (radix < 2 || radix > 16) throw Exception(String(STR("Radix out of range: ")) += Value(radix));
	int minLength = frame.getOptional(STR("$2"), Value(1));
	if (minLength < 0 || minLength > 32) throw Exception(String(STR("Minimum length out of range: ")) += Value(minLength));
	return int2string<unsigned long>(frame.get(STR("$0")), frame.get(STR("$1")), minLength);
}

Value trace(Frame& frame) {
	Context& env = frame.getEnvironment();
	env.tracerFunction = frame.getOptional(STR("$0"));
	env.traceLevel = static_cast<Frame::Precedence>(static_cast<int>(frame.getOptional(STR("$1")
			, Value(static_cast<int>(Frame::CALL_LEVEL)))));
	return Value();
}

String::size_type mismatch(const String& a, const String& b) {
	if (a.size() < b.size()) return std::mismatch(b.begin(), b.end(), a.begin()).first - b.begin();
	else return std::mismatch(a.begin(), a.end(), b.begin()).first - a.begin();
}

void addStandardLib(Frame& frame)
{
	frame.registerNative(STR("abs"), newUnaryFunctor(std::ptr_fun<double, double>(fabs)));
	frame.registerNative(STR("acos"), newUnaryFunctor(std::ptr_fun<double, double>(acos)));
	frame.registerNative(STR("asin"), newUnaryFunctor(std::ptr_fun<double, double>(asin)));
	frame.registerNative(STR("atan"), newUnaryFunctor(std::ptr_fun<double, double>(atan)));
	frame.registerNative(STR("atan2"), newBinaryFunctor(std::ptr_fun<double, double>(atan2)));
	frame.registerNative(STR("ceil"), newUnaryFunctor(std::ptr_fun<double, double>(ceil)));
	frame.registerNative(STR("char"), newUnaryFunctor(std::ptr_fun(character)));
	frame.registerNative(STR("charcode"), newUnaryFunctor(std::ptr_fun<const String&, unsigned int>(charcode)));
	frame.registerNative(STR("classify"), newUnaryFunctor(std::ptr_fun(classify)));
	frame.registerNative(STR("cos"), newUnaryFunctor(std::ptr_fun<double, double>(cos)));
	frame.registerNative(STR("cosh"), newUnaryFunctor(std::ptr_fun<double, double>(cosh)));
	frame.registerNative(STR("delete"), new CppFunction<deleter>());
	frame.registerNative(STR("escape"), newUnaryFunctor(std::ptr_fun(escape)));
	frame.registerNative(STR("exists"), new CppFunction<exists>());
	frame.registerNative(STR("evaluate"), new CppFunction<Pika::evaluate>());
	frame.registerNative(STR("exp"), newUnaryFunctor(std::ptr_fun<double, double>(exp)));
	frame.registerNative(STR("foreach"), new CppFunction<foreach>());
	frame.registerNative(STR("find"), newBinaryFunctor(std::ptr_fun(Pika::find)));
	frame.registerNative(STR("floor"), newUnaryFunctor(std::ptr_fun<double, double>(floor)));
	frame.registerNative(STR("input"), newUnaryFunctor(std::ptr_fun(input)));
	frame.registerNative(STR("invoke"), new CppFunction<invoke>());
	frame.registerNative(STR("length"), newUnaryFunctor(std::mem_fun_ref(&Value::size)));
	frame.registerNative(STR("ln"), newUnaryFunctor(std::ptr_fun<double, double>(log)));
	frame.registerNative(STR("load"), newUnaryFunctor(std::ptr_fun(load)));
	frame.registerNative(STR("log2"), newUnaryFunctor(std::ptr_fun(Pika::log2)));
	frame.registerNative(STR("log10"), newUnaryFunctor(std::ptr_fun<double, double>(log10)));
	frame.registerNative(STR("lower"), newUnaryFunctor(std::ptr_fun(lower)));
	frame.registerNative(STR("mismatch"), newBinaryFunctor(std::ptr_fun(mismatch)));
	frame.registerNative(STR("pow"), newBinaryFunctor(std::ptr_fun<double, double>(pow)));
	frame.registerNative(STR("print"), newUnaryFunctor(std::ptr_fun(print)));
	frame.registerNative(STR("radix"), new CppFunction<radix>());
	frame.registerNative(STR("random"), newUnaryFunctor(std::ptr_fun(random)));
	frame.registerNative(STR("reverse"), newUnaryFunctor(std::ptr_fun(reverse)));
	frame.registerNative(STR("sin"), newUnaryFunctor(std::ptr_fun<double, double>(sin)));
	frame.registerNative(STR("sinh"), newUnaryFunctor(std::ptr_fun<double, double>(sinh)));
	frame.registerNative(STR("save"), newBinaryFunctor(std::ptr_fun(save)));
	frame.registerNative(STR("search"), newBinaryFunctor(std::ptr_fun(search)));
	frame.registerNative(STR("span"), newBinaryFunctor(std::ptr_fun(span)));
	frame.registerNative(STR("sqrt"), newUnaryFunctor(std::ptr_fun<double, double>(sqrt)));
	frame.registerNative(STR("string"), new CppFunction<string>());
	frame.registerNative(STR("tan"), newUnaryFunctor(std::ptr_fun<double, double>(tan)));
	frame.registerNative(STR("tanh"), newUnaryFunctor(std::ptr_fun<double, double>(tanh)));
	frame.registerNative(STR("time"), new CppFunction<Pika::time>());
	frame.registerNative(STR("throw"), newUnaryFunctor(std::ptr_fun(thrower)));
	frame.registerNative(STR("trace"), new CppFunction<trace>());
	frame.registerNative(STR("trunc"), new CppFunction<Pika::trunc>());
	frame.registerNative(STR("try"), new CppFunction<tryer>());
	frame.registerNative(STR("unescape"), newUnaryFunctor(std::ptr_fun<const String&, String>(unescape)));
	frame.registerNative(STR("upper"), newUnaryFunctor(std::ptr_fun(upper)));
}

} // namespace Pika
