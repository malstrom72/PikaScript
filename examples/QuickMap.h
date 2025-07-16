class STLMapFrame : public Frame {
	protected:	enum { CACHE_SIZE = 11 };
	public:		typedef std::map<const String, Value> VariableMap;
	public:		typedef std::map<const String, Native*> NativeMap;
	public:		typedef std::pair<String, Value> CacheEntry;
	public:		STLMapFrame(Context& context);
	public:		STLMapFrame(Frame* previous);
	public:		virtual bool lookup(const String& symbol, Value& result);
	public:		virtual bool assign(const String& symbol, const Value& value);
	public:		virtual void erase(const String& symbol);
	public:		virtual void list(const String& key, VarList& list);
	public:		virtual Native* lookupNative(const String& name);
	public:		virtual bool assignNative(const String& name, Native* native);
	public:		virtual Value call(const Value& func, long argc, const Value* argv
						, const Value& funcName = Value(), const Value& thisValue = Value());
	protected:	virtual void makePikaCall(StringIt& p, const StringIt& e, XValue& v, bool dry, Precedence thres);
	public:		virtual ~STLMapFrame() throw();
	protected:	static int hash(const String& s);
	protected:	CacheEntry cache[CACHE_SIZE];
	protected:	VariableMap variables;
	protected:	NativeMap natives;
};


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

bool STLMapFrame::lookup(const String& identifier, Value& result) {
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

void STLMapFrame::list(const String& key, VarList& list) {
	for (int i = 0; i < CACHE_SIZE; ++i) if (!cache[i].first.empty()) variables[cache[i].first] = cache[i].second;
	for (VariableMap::const_iterator it = variables.lower_bound(key)
			; it != variables.end() && it->first.substr(0, key.size()) == key; ++it) {
		list.push_back(*it);
	}
}

Frame::Native* STLMapFrame::lookupNative(const String& identifier) {
	NativeMap::iterator it = natives.find(identifier);
	return (it == natives.end() ? 0 : it->second);
}

bool STLMapFrame::assignNative(const String& name, Native* native) {	
	NativeMap::iterator it = natives.insert(NativeMap::value_type(name, 0)).first;
	if (it->second != native) delete it->second;
	it->second = native;
	return true;
}

Value STLMapFrame::call(const Value& func, long argc, const Value* argv, const Value& funcName, const Value& thisValue) {
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

