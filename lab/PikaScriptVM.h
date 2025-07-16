/**
	\file PikaScript.h
	                                                                           
	(C)2008 NuEdge Development.
	All rights reserved.
	This copyright notice may not be removed etc etc.
*/
#ifndef PikaScriptVM_h
#define PikaScriptVM_h

// FIX : templatize entire Pika instead?
// FIX : wouldn't it be cool if frames could have explicit labels, couldn't that somehow fix some of the limitations of not having good OO support, for example, the window which executes a script can setup its own frame called :window: (all anonymous frames need some new prefix) and still have globals in ::.

#if !defined(PIKA_UNICODE)
	#define PIKA_UNICODE 0
#endif

#if !defined(PIKA_STRING_CLASS)
	#define PIKA_STRING_CLASS std::basic_string<Char>
#endif

#if !defined(PIKA_STRING_INCLUDE)
	#define PIKA_STRING_INCLUDE <string>
#endif

#include <assert.h>
#include <vector>
#include <map>
#include PIKA_STRING_INCLUDE

namespace Pika {

#if (PIKA_UNICODE)
	typedef wchar_t Char;
	inline unsigned int charcode(wchar_t c) { return c; }
	#define STR(s) L##s
	#define PRINTFS "ls"
	inline FILE* xfopen(const wchar_t* f, const wchar_t* m) { return _wfopen(f, m); }
	inline wchar_t* xfgets(wchar_t* s, int n, FILE* f) { return fgetws(s, n, f); }
	inline size_t xstrlen(const wchar_t* s) { return wcslen(s); }
	inline size_t xputs(const wchar_t* s) { return fputws(s, stdout); }
#else
	typedef char Char;
	inline unsigned int charcode(char c) { return static_cast<unsigned char>(c); }
	#define STR(x) x
	#define PRINTFS "s"
	inline FILE* xfopen(const char* f, const char* m) { return fopen(f, m); }
	inline char* xfgets(char* s, int n, FILE* f) { return fgets(s, n, f); }
	inline size_t xstrlen(const char* s) { return strlen(s); }
	inline size_t xputs(const char* s) { return fputs(s, stdout); }
#endif

typedef PIKA_STRING_CLASS String;
typedef String::const_iterator StringIt;

/*
	Conversion routines for string <-> other types. For some of these we could use stdlib implementations yes, but:
	
	1) Some of them (e.g. atof, strtod) behaves differently depending on global "locale" setting. We can't have that.
	2) The stdlib implementations can be slow (e.g. my double->string conversion is about 3 times faster than MSVC CRT).
	3) Pika requires high-precision string representation and proper handling of trailing 9's etc.
*/

unsigned long hex2long(StringIt& p, const StringIt& e);
long string2long(StringIt& p, const StringIt& e);
double string2double(StringIt& p, const StringIt& e);
bool string2double(const String& s, double& d);
String double2string(double d, int precision = 14, bool trim = true);
String unescape(StringIt& p, const StringIt& e);
String escape(const String& s);

template<typename T> String int2string(T i, int radix = 10, int minLength = 1) {
	assert(1 <= radix && radix <= 16);
	assert(0 <= minLength);
	Char buffer[32], * p = buffer + 32, * e = p - minLength;
	T x = i;
	while (p > e || x != 0) {
		assert(p >= buffer + 1);
		*--p = STR("fedcba9876543210123456789abcdef")[15 + x % radix];	// Mirrored hex string to handle negative x
		x /= radix;
	}
	if (i < 0) *--p = '-';
	return String(p, buffer + 32 - p);
}

class Exception : public std::exception {
	public:		Exception(const String& error);
	public:		virtual const char *what() const throw();
	public:		virtual ~Exception() throw(); // GCC requires explicit destructor with one that has throw().
	public:		String error;
	protected:	mutable std::string cvt;
};

// FIX : add Value(const char*) etc, dangerous otherwise because of stupid c++ automatic ptr->bool conversion
class Value : public String {
	public:		Value() { }
	public:		Value(const String& copy) : String(copy) { }
	public:		explicit Value(double d) : String(double2string(d)) { }
	public:		explicit Value(float f) : String(double2string(f)) { }
	public:		explicit Value(long i) : String(int2string<long>(i)) { }
	public:		explicit Value(unsigned long i) : String(int2string<unsigned long>(i)) { }
	public:		explicit Value(int i) : String(int2string<long>(i)) { }
	public:		explicit Value(unsigned int i) : String(int2string<unsigned long>(i)) { }
	public:		explicit Value(bool b) : String(b ? String(STR("true")) : String(STR("false"))) { }
	public:		operator bool() const;
	public:		operator long() const;
	public:		operator double() const;
	public:		operator float() const { return static_cast<float>(double(*this)); }
	public:		operator unsigned long() const { return static_cast<unsigned long>(long(*this)); }
	public:		operator int() const { return static_cast<int>(long(*this)); }
	public:		operator unsigned int() const { return static_cast<unsigned int>(int(*this)); }
	public:		friend bool operator<(const Value& l, const Value& r);
	public:		friend bool operator==(const Value& l, const Value& r);
	public:		friend bool operator!=(const Value& l, const Value& r) { return !(l == r); }
	public:		friend bool operator>(const Value& l, const Value& r) { return r < l; }
	public:		friend bool operator<=(const Value& l, const Value& r) { return !(l > r); }
	public:		friend bool operator>=(const Value& l, const Value& r) { return !(l < r); }
	public:		Char operator[](String::size_type i) const { return String::operator[](i); }
	public:		Value operator[](const Value& i) const;
};

class Frame;
class Native {
	public:		virtual Value pikaCall(Frame& frame) { throw Exception("Not callable"); }
	public:		virtual ~Native();
};

class Context;

class Frame {
	public:		enum Precedence {
					NO_TRACE_LEVEL = 0
					, ERROR_LEVEL = 1			// used only for tracing with tick()
					, CALL_LEVEL = 2			// used only for tracing with tick()
					, STATEMENT_LEVEL = 3		// x; y;
					, BODY_LEVEL = 4			// if () x, for () x
					, ARGUMENT_LEVEL = 5		// (x, y)
					, PARENTHESIS_LEVEL = 6		// (x) [x]
					, ASSIGN_LEVEL = 7			// x=y x*=y x/=y x+=y x-=y x<<=y x>>=y x#=y x&=y x^=y x|=y
					, LOGICAL_OR_LEVEL = 8		// x||y
					, LOGICAL_AND_LEVEL = 9		// x&&y
					, BIT_OR_LEVEL = 10			// x|y
					, BIT_XOR_LEVEL = 11		// x^y
					, BIT_AND_LEVEL = 12		// x&y
					, EQUALITY_LEVEL = 13		// x===y x==y x!==y x!=y
					, COMPARE_LEVEL = 14		// x<y x<=y x>y x>=y
					, CONCAT_LEVEL = 15			// x<<y x>>y
					, SHIFT_LEVEL = 16			// x<<y x>>y
					, ADD_SUB_LEVEL = 17		// x+y x-y
					, MUL_DIV_LEVEL = 18		// x*y x/y
					, PREFIX_LEVEL = 19 		// @x !x ~x +x -x ++x --x
					, POSTFIX_LEVEL = 20		// x() x.y x[y] x{y} x++ x--
					, DEFINITION_LEVEL = 21		// function { }
				};
	public:		typedef std::pair<bool, Value> XValue;			///< first = lvalue or not
				
	public:		typedef std::vector< std::pair<String, Value> > VarList;
	public:		Frame(Context& context);
	public:		Frame(Frame* previous);

	public:		virtual bool lookup(const String& symbol, Value& result) const = 0;
	public:		virtual bool assign(const String& symbol, const Value& value) = 0;
	public:		virtual void erase(const String& symbol) = 0;
	public:		virtual void list(const String& key, VarList& list) const = 0;
	public:		virtual Native* lookupNative(const String& name) = 0;
	public:		virtual bool assignNative(const String& name, Native* native) = 0;
	public:		virtual Value call(const String& func, long argc = 0, const Value* argv = 0
						, const Value& funcName = Value(), const Value& thisValue = Value()) = 0;
	
	public:		std::pair<Frame*, String> resolveFrame(const String& identifier);
	public:		Value get(const String& identifier, bool fallback = true);
	public:		Value getOptional(const String& identifier, const Value& defaultValue = Value());
	public:		const Value& set(const String& identifier, const Value& v);
	public:		Value evaluate(const String source); // Note: not using reference to string here so that we are safe in case the Pika-code manipulates the very string it is running on.
	public:		Value indirect(const String& identifier);
	public:		void registerNative(const String& name, Native* native);
	public:		Context& getEnvironment();
	public:		Value evaluateCompiled(const String code); // Note: not using reference to string here so that we are safe in case the Pika-code manipulates the very string it is running on.
	
	public:		virtual ~Frame() throw() { }

	protected:	void caught(const StringIt& p, const String& what);
	protected:	void tick(const StringIt& p, const XValue& v, bool dry, Precedence thres, bool exit);
	protected:	Value rvalue(const XValue& v, bool fallback = true);
	protected:	const Value& lvalue(const XValue& v);
	protected:	void fullWhite(StringIt& p, const StringIt& e);
	protected:	void white(StringIt& p, const StringIt& e);
	protected:	bool token(StringIt& p, const StringIt& e, const Char* token);
	protected:	Frame* resolveFrame(StringIt& b, const StringIt& e);
	protected:	template<class F> bool binaryOp(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres
						, int hop, Precedence prec, F op);
	protected:	template<class F> bool assignableOp(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres
						, int hop, Precedence prec, F op);
	protected:	template<class F> bool addSubOp(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres
						, const F& f);
	protected:	template<class E, class I, class S> bool lessGreaterOp(StringIt& p, const StringIt& e, XValue& v
						, bool dry, Precedence thres, const E& excl, const I& incl, const S& shift);
	protected:	bool pre(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres);
	protected:	bool post(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres);
	protected:	bool expr(StringIt& p, const StringIt& e, XValue& v, bool emptyOk, bool dry, Precedence thres);
	protected:	bool termExpr(StringIt& p, const StringIt& e, XValue& v, bool emptyOk, bool dry, Precedence thres
						, Char term);
	protected:	Value execute(Frame& inFrame, const String& body);
	protected:	Value completeCppCall(Frame& inFrame, const String& func, long argc, const Value* argv
						, const Value& funcName, const Value& thisValue);
	protected:	void completePikaCall(Frame& inFrame, StringIt& p, const StringIt& e, XValue& v, bool dry
						, Precedence thres);
	protected:	virtual void makePikaCall(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres) = 0;
	protected:	virtual void trace(const String& source, String::size_type offset, const XValue& v, bool dry, Precedence thres, bool exit);
	
	protected:	Context& context;
	protected:	Frame* const previous;
	protected:	Frame* closure;
	protected:	Frame* const global; // FIX : move to context
	protected:	const String label;
	protected:	const int callDepth; // FIX : move to context
	
	private:	Frame(const Frame& copy); // N/A
	private:	Frame& operator=(const Frame& copy); // N/A
};

class Context {
	public:		Context();
	public:		String generateLabel();
	public:		int maxCallDepth;
	public:		Frame::Precedence traceLevel;
	public:		Value tracerFunction;			///< Pika-script tracer function (used by the default Frame::trace() method implementation).
	public:		bool isInsideTracer;			///< Set to prevent recursive calling of tracer (used by the default Frame::trace() method implementation).
	public:		const String* currentSource;
	protected:	Char autoLabel[32];
	protected:	Char* autoLabelStart;
};

class STLMapFrame : public Frame {
	protected:	enum { CACHE_SIZE = 11 };
	public:		typedef std::map<const String, Value> VariableMap;
	public:		typedef std::map<const String, Native*> NativeMap;
	public:		typedef std::pair<String, Value> CacheEntry;
	public:		STLMapFrame(Context& context);
	public:		STLMapFrame(Frame* previous);
	public:		virtual bool lookup(const String& symbol, Value& result) const;
	public:		virtual bool assign(const String& symbol, const Value& value);
	public:		virtual void erase(const String& symbol);
	public:		virtual void list(const String& key, VarList& list) const;
	public:		virtual Native* lookupNative(const String& name);
	public:		virtual bool assignNative(const String& name, Native* native);
	public:		virtual Value call(const String& func, long argc, const Value* argv, const Value& thisValue
						, const Value& funcName);
	protected:	virtual void makePikaCall(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres);
	public:		virtual ~STLMapFrame() throw();
	protected:	static int hash(const String& s);
	protected:	mutable CacheEntry cache[CACHE_SIZE];
	protected:	mutable VariableMap variables;
	protected:	NativeMap natives;
};

template<class C> class CppObject : public Native {
	public:		CppObject(C* o) : o(o) { }
	public:		virtual ~CppObject() { delete o; }
	protected:	C* o;
};

typedef Value (*FunctionPointer)(Frame&);
template<FunctionPointer F> class CppFunction : public Native {
	public:		virtual Value pikaCall(Frame& frame) { return F(frame); }
};

template<class C, Value (C::*M)(Frame& frame)> class CppMethod : public Native {
	public:		CppMethod(C* o) : o(o) { }
	public:		virtual Value pikaCall(Frame& frame) { return (o->*M)(frame); }
	protected:	C* o;
};

template<class F, typename A0 = typename F::argument_type, typename R = typename F::result_type> class UnaryFunctor
		: public Native {
	public:		UnaryFunctor(const F& f) : f(f) { }
	protected:	template<class T> Value callT(const Value& a0, T*) const { return Value(f(static_cast<A0>(a0))); }
	protected:	Value callT(const Value& a0, void*) const { f(static_cast<A0>(a0)); return Value(); }
	public:		virtual Value pikaCall(Frame& frame) { return callT(frame.get(STR("$0")), (R*)(0)); }
	protected:	F f;
};

template<typename F, typename A0 = typename F::first_argument_type, typename A1 = typename F::second_argument_type
		, typename R = typename F::result_type> class BinaryFunctor : public Native {
	public:		BinaryFunctor(const F& f) : f(f) { }
	protected:	template<class T> Value callT(const Value& a0, const Value& a1, T*) const {
					return Value(f(static_cast<A0>(a0), static_cast<A1>(a1)));
				}
	protected:	Value callT(const Value& a0, const Value& a1, void*) const {
					f(static_cast<A0>(a0), static_cast<A1>(a1)); return Value();
				}
	public:		virtual Value pikaCall(Frame& frame) {
					return callT(frame.get(STR("$0")), frame.get(STR("$1")), (R*)(0));
				}
	protected:	F f;
};

template<typename F> UnaryFunctor<F>* newUnaryFunctor(const F& f) { return new UnaryFunctor<F>(f); }
template<typename F> BinaryFunctor<F>* newBinaryFunctor(const F& f) { return new BinaryFunctor<F>(f); }

/* --- Standard library --- */

void print(const String& s);
String upper(String s);
String lower(String s);
double log2(double d);
String::size_type mismatch(const String& a, const String& b);
void thrower(const String& s);
String::size_type find(const String& a, const String& b);
String::size_type search(const String& a, const String& b);
Value evaluate(Frame& frame);
Value exists(Frame& frame);
Value deleter(Frame& frame);
String character(double d);
unsigned int charcode(const String& s);
String input(const String& prompt);
Value invoke(Frame& frame);
Value string(Frame& frame);
Value trunc(Frame& frame);
Value substr(Frame& frame);
Value tryer(Frame& frame);
Value foreach(Frame& frame);
String classify(const Value& v);
Value load(const String& file);
Value radix(Frame& frame);
double random(double m);
String reverse(String s);
double round(double r);
void save(const String& file, const String& chars);
String::size_type span(const String& a, const String& b);
Value time(Frame&);
String unescape(const String& s);
void addStandardLib(Frame& frame);

} // namespace Pika

#endif
